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
 *******************************************************************************/
#if !defined(TLSSOCKET_H)
#define TLSSOCKET_H
#include <wm-tls.h>
#include "SocketBuffer.h"
#include "Clients.h"

#define URI_SSL "ssl://"
#define DEFAULT_RECEIVE_TIMEOUT 5000

int TLSSocket_initialize();
int TLSSocket_connect(tls_handle_t *h, int sockfd, const tls_init_config_t *cfg);
void TLSSocket_terminate();
int TLSSocket_close(tls_handle_t *h);

char *TLSSocket_getdata(tls_handle_t h, int socket, int bytes, int* actual_len);
int TLSSocket_getch(tls_handle_t h, int socket, char* c);
int TLSSocket_getPendingRead();

int TLSSocket_putdatas(tls_handle_t h, int socket, char* buf0, size_t buf0len, int count, char** buffers, size_t* buflens, int* frees);
int TLSSocket_continueWrite(pending_writes* pw);

#endif
