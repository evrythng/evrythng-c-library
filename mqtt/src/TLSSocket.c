/*******************************************************************************
 * Copyright (c) 2009, 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs, Allan Stockdill-Mander - initial implementation
 *    Ian Craggs - fix for bug #409702
 *    Ian Craggs - allow compilation for OpenSSL < 1.0
 *******************************************************************************/

/**
 * @file
 * \brief TLS  related functions
 *
 */

#if defined(TLSSOCKET)

#include "SocketBuffer.h"
#include "MQTTClient.h"
#include "TLSSocket.h"
#include "Log.h"
#include "StackTrace.h"
#include "Socket.h"

#include "Heap.h"

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/crypto.h>



void TLSSocket_addPendingRead(int sock);
static mutex_type tlsCoreMutex;
extern Sockets s;
int TLSInitSuccessFlag = 0;
/**
 * Gets the specific error corresponding to SOCKET_ERROR
 * @param aString the function that was being used when the error occurred
 * @param sock the socket on which the error occurred
 * @return the specific TCP error code
 */
static int TLSSocket_error(char* aString, tls_handle_t h, int sock, int rc)
{
	int error = 0;

	FUNC_ENTRY;
	if (0 != h)
		error = tls_get_error(h, rc);

	if (error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_WRITE)
	{
		Log(TRACE_MIN, -1, "TLSSocket error WANT_READ/WANT_WRITE");
	}
	else
	{
#if !defined(CONFIG_OS_FREERTOS) || defined(EVRYTHNG_DEBUG)
		static char buf[120];
#endif

		if (strcmp(aString, "shutdown") != 0)
			Log(TRACE_MIN, -1, "TLSSocket error %s(%d) in %s for socket %d rc %d errno %d %s\n", buf, error, aString, sock, rc, errno, strerror(errno));
#ifdef TODO_ITEMS
		ERR_print_errors_fp(stderr);
#endif
		if (error == SSL_ERROR_SSL || error == SSL_ERROR_SYSCALL)
			error = SSL_FATAL;
	}
	FUNC_EXIT_RC(error);
	return error;
}

int TLSSocket_initialize()
{
	int rc = SOCKET_ERROR;

	FUNC_ENTRY;

	rc = tls_lib_init();

	tlsCoreMutex = Thread_create_mutex();

	FUNC_EXIT_RC(rc);

	return rc;
}

int TLSSocket_connect(tls_handle_t *h, int sockfd, const tls_init_config_t *cfg)
{
	int rc = SOCKET_ERROR;

	FUNC_ENTRY;
	/*
	This workaround (temporary set socket to blocking) was added because
	MQTT uses nonblocking sockets but TLS prefer to use blocking. 
	That is why we turn on blocking just for TLS session init. For
	TLS read/write operations MQTT will take care.
	*/
	Socket_setblocking(sockfd);

	int timeout = DEFAULT_RECEIVE_TIMEOUT;
	rc = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(int));
	if (SOCKET_ERROR == rc)
	{
		Log(TRACE_MIN, -1, "TLSSocket_connect setsockopt error");
	}
	else
	{
		rc = tls_session_init(h, sockfd, cfg);
		if (SOCKET_ERROR == rc)
		{
			Log(TRACE_MIN, -1, "TLSSocket_connect tls_session_init error");
		}
		else
		{
			TLSInitSuccessFlag = 1;
		}
	}

	Socket_setnonblocking(sockfd);

	FUNC_EXIT_RC(rc);

	return rc;
}



/**
 *  Reads one byte from a socket
 *  @param socket the socket to read from
 *  @param c the character read, returned
 *  @return completion code
 */
int TLSSocket_getch(tls_handle_t h, int socket, char* c)
{
	int rc = SOCKET_ERROR;

	FUNC_ENTRY;
	if ((rc = SocketBuffer_getQueuedChar(socket, c)) != SOCKETBUFFER_INTERRUPTED)
		goto exit;

	if ((rc = tls_recv(h, c, (size_t)1)) < 0)
	{
		int err = TLSSocket_error("TLS_read - getch", h, socket, rc);
		if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
		{
			rc = TCPSOCKET_INTERRUPTED;
			SocketBuffer_interrupted(socket, 0);
		}
	}
	else if (rc == 0)
		rc = SOCKET_ERROR; 	/* The return value from recv is 0 when the peer has performed an orderly shutdown. */
	else if (rc == 1)
	{
		SocketBuffer_queueChar(socket, *c);
		rc = TCPSOCKET_COMPLETE;
	}
exit:
	FUNC_EXIT_RC(rc);
	return rc;
}



/**
 *  Attempts to read a number of bytes from a socket, non-blocking. If a previous read did not
 *  finish, then retrieve that data.
 *  @param socket the socket to read from
 *  @param bytes the number of bytes to read
 *  @param actual_len the actual number of bytes read
 *  @return completion code
 */
char *TLSSocket_getdata(tls_handle_t h, int socket, int bytes, int* actual_len)
{
	int rc;
	char* buf;

	FUNC_ENTRY;
	if (bytes == 0)
	{
		buf = SocketBuffer_complete(socket);
		goto exit;
	}

	buf = SocketBuffer_getQueuedData(socket, bytes, actual_len);

	if ((rc = tls_recv(h, buf + (*actual_len), (size_t)(bytes - (*actual_len)))) < 0)
	{
		rc = TLSSocket_error("TLS_read - getdata", h, socket, rc);
		if (rc != SSL_ERROR_WANT_READ && rc != SSL_ERROR_WANT_WRITE)
		{
			buf = NULL;
			goto exit;
		}
	}
	else if (rc == 0) /* rc 0 means the other end closed the socket */
	{
		buf = NULL;
		goto exit;
	}
	else
		*actual_len += rc;

	if (*actual_len == bytes)
	{
		SocketBuffer_complete(socket);
		/* if we read the whole packet, there might still be data waiting in the TLS buffer, which
		isn't picked up by select.  So here we should check for any data remaining in the TLS buffer, and
		if so, add this socket to a new "pending TLS reads" list.
		*/
		if (tls_pending(h) > 0) /* return no of bytes pending */
			TLSSocket_addPendingRead(socket);
	}
	else /* we didn't read the whole packet */
	{
		SocketBuffer_interrupted(socket, *actual_len);
		Log(TRACE_MAX, -1, "TLS_read: %d bytes expected but %d bytes now received", bytes, *actual_len);
	}
exit:
	FUNC_EXIT;
	return buf;
}

int TLSSocket_close(tls_handle_t *h)
{
	FUNC_ENTRY;

	if ((NULL != h) && (0 != *h) && TLSInitSuccessFlag)
	{
		tls_close(h);
		TLSInitSuccessFlag = 0;
	}

	FUNC_EXIT_RC(rc);

	return 1;
}


int TLSSocket_putdatas(tls_handle_t h, int socket, char* buf0, size_t buf0len, int count, char** buffers, size_t* buflens, int* frees)
{
	int rc = 0;
	int i;
	char *ptr;
	iobuf iovec;
	int tlserror;

	FUNC_ENTRY;
	iovec.iov_len = buf0len;
	for (i = 0; i < count; i++)
		iovec.iov_len += buflens[i];

	ptr = iovec.iov_base = (char *)malloc(iovec.iov_len);
	memcpy(ptr, buf0, buf0len);
	ptr += buf0len;
	for (i = 0; i < count; i++)
	{
		memcpy(ptr, buffers[i], buflens[i]);
		ptr += buflens[i];
	}

	Thread_lock_mutex(tlsCoreMutex);

	if ((rc = tls_send(h, iovec.iov_base, iovec.iov_len)) == iovec.iov_len)
	{
		rc = TCPSOCKET_COMPLETE;
	}
	else
	{
		tlserror = TLSSocket_error("TLS_write", h, socket, rc);

		if (tlserror == SSL_ERROR_WANT_WRITE)
		{
			int* sockmem = (int*)malloc(sizeof(int));
			int free = 1;

			Log(TRACE_MIN, -1, "Partial write: incomplete write of %d bytes on TLS socket %d",
				iovec.iov_len, socket);
			SocketBuffer_pendingWrite(socket, h, 1, &iovec, &free, iovec.iov_len, 0);
			*sockmem = socket;
			ListAppend(s.write_pending, sockmem, sizeof(int));
			FD_SET(socket, &(s.pending_wset));
			rc = TCPSOCKET_INTERRUPTED;
		}
		else
			rc = SOCKET_ERROR;
	}
	Thread_unlock_mutex(tlsCoreMutex);

	if (rc != TCPSOCKET_INTERRUPTED)
		free(iovec.iov_base);
	else
	{
		int i;
		free(buf0);
		for (i = 0; i < count; ++i)
		{
			if (frees[i])
				free(buffers[i]);
		}
	}
	FUNC_EXIT_RC(rc);
	return rc;
}

static List pending_reads = {NULL, NULL, NULL, 0, 0};

void TLSSocket_addPendingRead(int sock)
{
	FUNC_ENTRY;
	if (ListFindItem(&pending_reads, &sock, intcompare) == NULL) /* make sure we don't add the same socket twice */
	{
		int* psock = (int*)malloc(sizeof(sock));
		*psock = sock;
		ListAppend(&pending_reads, psock, sizeof(sock));
	}
	else
		Log(TRACE_MIN, -1, "TLSSocket_addPendingRead: socket %d already in the list", sock);

	FUNC_EXIT;
}


int TLSSocket_getPendingRead()
{
	int sock = -1;

	if (pending_reads.count > 0)
	{
		sock = *(int*)(pending_reads.first->content);
		ListRemoveHead(&pending_reads);
	}
	return sock;
}


int TLSSocket_continueWrite(pending_writes* pw)
{
	int rc = 0;

	FUNC_ENTRY;
	if ((rc = tls_send(pw->h, pw->iovecs[0].iov_base, pw->iovecs[0].iov_len)) == pw->iovecs[0].iov_len)
	{
		/* topic and payload buffers are freed elsewhere, when all references to them have been removed */
		free(pw->iovecs[0].iov_base);
		Log(TRACE_MIN, -1, "TLS continueWrite: partial write now complete for socket %d", pw->socket);
		rc = 1;
	}
	else
	{
		int tlserror = TLSSocket_error("TLS_write", pw->h, pw->socket, rc);
		if (tlserror == SSL_ERROR_WANT_WRITE)
			rc = 0; /* indicate we haven't finished writing the payload yet */
	}
	FUNC_EXIT_RC(rc);
	return rc;
}
#endif
