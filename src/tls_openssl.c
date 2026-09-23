#include <stdio.h>
#include <stdlib.h>

#include <openssl/err.h>
#include <openssl/ssl.h>

#include "rictus_tls.h"

typedef struct rictus_openssl_connection {
    SSL_CTX *context;
    SSL *ssl;
} rictus_openssl_connection;

static void set_error(char *error, size_t error_size, const char *message)
{
    if (error != NULL && error_size > 0U) {
        (void)snprintf(error, error_size, "%s", message);
    }
}

static void set_openssl_error(char *error, size_t error_size, const char *operation)
{
    unsigned long code = ERR_get_error();
    char detail[160];

    if (code == 0UL) {
        set_error(error, error_size, operation);
        return;
    }

    ERR_error_string_n(code, detail, sizeof(detail));
    if (error != NULL && error_size > 0U) {
        (void)snprintf(error, error_size, "%s: %s", operation, detail);
    }
}

int rictus_tls_connect(rictus_tls_connection *tls,
                       rictus_net_connection *transport,
                       const char *host,
                       char *error,
                       size_t error_size)
{
    rictus_openssl_connection *native;
    void *handle = NULL;

    if (tls == NULL || transport == NULL || host == NULL || host[0] == '\0') {
        set_error(error, error_size, "invalid TLS connection request");
        return 0;
    }

    tls->native = NULL;

    if (!rictus_net_native_handle(transport, &handle) || handle == NULL) {
        set_error(error, error_size, "TLS transport is unavailable");
        return 0;
    }

    native = (rictus_openssl_connection *)calloc(1U, sizeof(*native));
    if (native == NULL) {
        set_error(error, error_size, "unable to allocate TLS connection");
        return 0;
    }

    native->context = SSL_CTX_new(TLS_client_method());
    if (native->context == NULL) {
        set_openssl_error(error, error_size, "unable to create TLS context");
        free(native);
        return 0;
    }

    SSL_CTX_set_verify(native->context, SSL_VERIFY_PEER, NULL);
    if (SSL_CTX_set_default_verify_paths(native->context) != 1) {
        set_openssl_error(error, error_size, "unable to load system trust store");
        SSL_CTX_free(native->context);
        free(native);
        return 0;
    }

    native->ssl = SSL_new(native->context);
    if (native->ssl == NULL) {
        set_openssl_error(error, error_size, "unable to create TLS session");
        SSL_CTX_free(native->context);
        free(native);
        return 0;
    }

    if (SSL_set_tlsext_host_name(native->ssl, host) != 1) {
        set_openssl_error(error, error_size, "unable to set TLS SNI hostname");
        SSL_free(native->ssl);
        SSL_CTX_free(native->context);
        free(native);
        return 0;
    }

    if (SSL_set1_host(native->ssl, host) != 1) {
        set_openssl_error(error, error_size, "unable to set TLS verification hostname");
        SSL_free(native->ssl);
        SSL_CTX_free(native->context);
        free(native);
        return 0;
    }

#ifdef _WIN32
    if (SSL_set_fd(native->ssl, (int)(intptr_t)handle) != 1) {
#else
    if (SSL_set_fd(native->ssl, *(int *)handle) != 1) {
#endif
        set_openssl_error(error, error_size, "unable to attach TLS transport");
        SSL_free(native->ssl);
        SSL_CTX_free(native->context);
        free(native);
        return 0;
    }

    if (SSL_connect(native->ssl) != 1) {
        set_openssl_error(error, error_size, "TLS handshake failed");
        SSL_free(native->ssl);
        SSL_CTX_free(native->context);
        free(native);
        return 0;
    }

    if (SSL_get_verify_result(native->ssl) != X509_V_OK) {
        set_error(error, error_size, "TLS certificate verification failed");
        SSL_shutdown(native->ssl);
        SSL_free(native->ssl);
        SSL_CTX_free(native->context);
        free(native);
        return 0;
    }

    tls->native = native;
    return 1;
}

void rictus_tls_close(rictus_tls_connection *tls)
{
    rictus_openssl_connection *native;

    if (tls == NULL || tls->native == NULL) {
        return;
    }

    native = (rictus_openssl_connection *)tls->native;
    if (native->ssl != NULL) {
        (void)SSL_shutdown(native->ssl);
        SSL_free(native->ssl);
    }
    if (native->context != NULL) {
        SSL_CTX_free(native->context);
    }

    free(native);
    tls->native = NULL;
}
