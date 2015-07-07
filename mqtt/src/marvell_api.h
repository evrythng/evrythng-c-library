#if !defined(MARVELL_API_H)
#define MARVELL_API_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>

#if !defined(FREERTOS_SIMULATOR)
#include <lwip/lwipopts.h>
#include <lwip/opt.h>
#include <lwip/sockets.h>
#include <lwip/tcpip.h>
#include <lwip/ip.h>
#include <lwip/netdb.h>
#include <lwip/err.h>
#include <lwip/inet.h>

#define time_t uint32_t

#define exit(...)
extern time_t rtc_time_get(void);
void time(time_t *c_time);

struct iovec {
	void *iov_base;   /* Starting address */
	size_t iov_len;   /* Number of bytes */
};

#define realloc pvPortReAlloc

#else

#include <semphr.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>

#define rtc_time_get() time(NULL)

#endif

#define malloc pvPortMalloc
#define free(ptr) vPortFree(ptr)

#define difftime(t1, t0) (double)(t1 - t0)

void Log_setLevel(int level);
#if defined(EVRYTHNG_DEBUG)
void Log(int, int, char *, ...);
#else
#define Log(...)
#endif


// OpenSSL defines that are missing in CyaSSL

#define SSLEAY_CFLAGS 2
#define SSLEAY_BUILT_ON 3
#define SSLEAY_PLATFORM 4
#define SSLEAY_DIR 5

#define X509_V_ERR_UNABLE_TO_GET_CRL                    3
#define X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE     4
#define X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE      5
#define X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY   6
#define X509_V_ERR_CERT_SIGNATURE_FAILURE               7
#define X509_V_ERR_CRL_NOT_YET_VALID                    11
#define X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD       15
#define X509_V_ERR_OUT_OF_MEM                           17
#define X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT          18
#define X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN            19
#define X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY    20
#define X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE      21
#define X509_V_ERR_INVALID_CA                           24
#define X509_V_ERR_PATH_LENGTH_EXCEEDED                 25
#define X509_V_ERR_INVALID_PURPOSE                      26
#define X509_V_ERR_CERT_UNTRUSTED                       27
#define X509_V_ERR_CERT_REJECTED                        28
#define X509_V_ERR_SUBJECT_ISSUER_MISMATCH              29
#define X509_V_ERR_AKID_SKID_MISMATCH                   30
#define X509_V_ERR_AKID_ISSUER_SERIAL_MISMATCH          31
#define X509_V_ERR_KEYUSAGE_NO_CERTSIGN                 32
#define X509_V_ERR_UNABLE_TO_GET_CRL_ISSUER             33
#define X509_V_ERR_UNHANDLED_CRITICAL_EXTENSION         34
#define X509_V_ERR_KEYUSAGE_NO_CRL_SIGN                 35
#define X509_V_ERR_UNHANDLED_CRITICAL_CRL_EXTENSION     36
#define X509_V_ERR_INVALID_NON_CA                       37
#define X509_V_ERR_PROXY_PATH_LENGTH_EXCEEDED           38
#define X509_V_ERR_KEYUSAGE_NO_DIGITAL_SIGNATURE        39
#define X509_V_ERR_PROXY_CERTIFICATES_NOT_ALLOWED       40
#define X509_V_ERR_INVALID_EXTENSION                    41
#define X509_V_ERR_INVALID_POLICY_EXTENSION             42
#define X509_V_ERR_NO_EXPLICIT_POLICY                   43
#define X509_V_ERR_UNNESTED_RESOURCE                    46

# define SSL_CB_EXIT                     0x02
# define SSL_CB_HANDSHAKE_START          0x10
# define SSL2_VERSION                    0x0002
# define SSL3_VERSION                    0x0300
# define TLS1_VERSION                    0x0301

#ifdef NO_FILESYSTEM
    #define SSL_CTX_use_PrivateKey_buffer CyaSSL_CTX_use_PrivateKey_buffer
    #define SSL_CTX_load_verify_buffer CyaSSL_CTX_load_verify_buffer
    #define SSL_CTX_use_certificate_chain_buffer CyaSSL_CTX_use_certificate_chain_buffer

    #define SSL_CTX_use_certificate_buffer CyaSSL_CTX_use_certificate_buffer
    #define SSL_use_certificate_buffer CyaSSL_use_certificate_buffer
    #define SSL_use_PrivateKey_buffer CyaSSL_use_PrivateKey_buffer
    #define SSL_use_certificate_chain_buffer CyaSSL_use_certificate_chain_buffer
#endif

#define SSL_get_ciphers CyaSSL_get_ciphers

#endif
