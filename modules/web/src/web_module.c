#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include "stnlabz_abi/module.h"
#include "rictus_route.h"
#include "web_protocol.h"

#define WEB_PORT 18761
#define WEB_TLS_CERT "/opt/rictus/auth/web/cert.pem"
#define WEB_TLS_KEY "/opt/rictus/auth/web/key.pem"
#define WEB_TOKEN_FILE "/opt/rictus/auth/web/token"
#define WEB_VERSION_MAJOR 1U
#define WEB_VERSION_MINOR 0U
#define WEB_VERSION_PATCH 1U

static const stnlabz_module_host_t *host;
static pthread_t worker;
static atomic_int stopping;
static int worker_started;
static int listen_fd = -1;
static SSL_CTX *web_tls_ctx;
static char secret[RICTUS_WEB_TOKEN_MAX + 1U];

/* Credentials are read once at activation, not supplied through the web API. */
static int load_secret(void)
{
    int fd;
    struct stat st;
    ssize_t n;
    char raw[RICTUS_WEB_TOKEN_MAX + 2U];
    size_t i;
    fd = open(WEB_TOKEN_FILE, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (fd < 0) return 0;
    if (fstat(fd, &st) != 0 || !S_ISREG(st.st_mode) ||
        (st.st_mode & 0077) != 0 || st.st_uid != geteuid()) {
        close(fd);
        return 0;
    }
    n = read(fd, raw, sizeof raw);
    close(fd);
    if (n < 32 || n > (ssize_t)(RICTUS_WEB_TOKEN_MAX + 1U))
        return 0;
    if (raw[n - 1] == '\n') --n;
    if (n < 32 || n > (ssize_t)RICTUS_WEB_TOKEN_MAX) return 0;
    for (i = 0; i < (size_t)n; ++i) {
        if (!((raw[i] >= 'a' && raw[i] <= 'f') ||
              (raw[i] >= '0' && raw[i] <= '9'))) return 0;
    }
    memcpy(secret, raw, (size_t)n);
    secret[n] = 0;
    memset(raw, 0, sizeof raw);
    return 1;
}

static void reply_http(SSL *tls, int status, const char *reason, const char *text)
{
    char header[512];
    size_t len = strlen(text);
    int n = snprintf(header, sizeof header,
       "HTTP/1.1 %d %s\r\nContent-Type: text/plain; charset=utf-8\r\n"
       "Cache-Control: no-store\r\nX-Content-Type-Options: nosniff\r\n"
       "Connection: close\r\nContent-Length: %zu\r\n\r\n",
       status, reason, len);
    if (n > 0 && (size_t)n < sizeof header) {
        (void)SSL_write(tls, header, n);
        (void)SSL_write(tls, text, (int)len);
    }
}

static void serve_one(int fd, SSL *tls)
{
    char raw[RICTUS_WEB_MAX_REQUEST + 1U];
    size_t used = 0U, offset = 0U, length = 0U, i;
    int parsed = 0, finished = 0;
    struct timeval timeout = {3, 0};
    rictus_route_request_t request;
    rictus_route_reply_t response;
    size_t response_used = 0U;
    stnlabz_module_result_t result;

    (void)setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);
    (void)setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof timeout);
    while (used < RICTUS_WEB_MAX_REQUEST && !atomic_load(&stopping)) {
        int n = SSL_read(tls, raw + used, (int)(RICTUS_WEB_MAX_REQUEST - used));
        if (n <= 0) break;
        used += (size_t)n;
        for (i = 0; i + 3U < used; ++i) {
            if (memcmp(raw + i, "\r\n\r\n", 4U) == 0) {
                finished = 1;
                break;
            }
        }
        if (finished) break;
    }
    if (!finished) {
        reply_http(tls, 400, "Bad Request", "Invalid HTTP request");
        return;
    }
    parsed = rictus_web_parse_headers(raw, used, secret, &offset, &length);
    if (parsed == -2) {
        reply_http(tls, 401, "Unauthorized", "Unauthorized");
        return;
    }
    if (parsed == -1) {
        reply_http(tls, 413, "Payload Too Large", "Request too large");
        return;
    }
    if (parsed != 1 || offset > used || used - offset > length) {
        reply_http(tls, 400, "Bad Request", "Invalid request");
        return;
    }
    while (used - offset < length && used < RICTUS_WEB_MAX_REQUEST
           && !atomic_load(&stopping)) {
        int n = SSL_read(tls, raw + used, (int)(length - (used - offset)));
        if (n <= 0) break;
        used += (size_t)n;
    }
    if (used - offset != length) {
        reply_http(tls, 400, "Bad Request", "Incomplete body");
        return;
    }
    for (i = 0; i < length; ++i) {
        unsigned char c = (unsigned char)raw[offset + i];
        if (c == 0U || c == '\r' || c == '\n') {
            reply_http(tls, 400, "Bad Request", "Invalid text");
            return;
        }
    }
    memset(&request, 0, sizeof request);
    memset(&response, 0, sizeof response);
    memcpy(request.sender, "web", sizeof "web");
    memcpy(request.account, "web", sizeof "web");
    memcpy(request.text, raw + offset, length);
    request.text[length] = 0;
    if (!host || !host->invoke_service) {
        reply_http(tls, 503, "Service Unavailable", "Router unavailable");
        return;
    }
    result = host->invoke_service(RICTUS_ROUTER_DISPATCH_SERVICE,
                                  &request, sizeof request,
                                  &response, sizeof response, &response_used);
    if (result != STNLABZ_MODULE_OK || response_used != sizeof response ||
        !response.replied || !memchr(response.text, 0, sizeof response.text)) {
        reply_http(tls, 503, "Service Unavailable", "No verified router reply");
        return;
    }
    for (i = 0; response.text[i]; ++i) {
        if (response.text[i] == '\r' || response.text[i] == '\n')
            response.text[i] = ' ';
    }
    reply_http(tls, 200, "OK", response.text);
}

static void *serve_loop(void *unused)
{
    (void)unused;
    while (!atomic_load(&stopping)) {
        struct pollfd pfd = {listen_fd, POLLIN, 0};
        int ready = poll(&pfd, 1, 250);
        if (ready < 0 && errno == EINTR) continue;
        if (ready <= 0) continue;
        if (pfd.revents & POLLIN) {
            int fd = accept(listen_fd, NULL, NULL);
            if (fd >= 0) {
                SSL *tls = SSL_new(web_tls_ctx);
                if (tls && SSL_set_fd(tls, fd) == 1 && SSL_accept(tls) == 1)
                    serve_one(fd, tls);
                if (tls) { (void)SSL_shutdown(tls); SSL_free(tls); }
                close(fd);
            }
        }
    }
    return NULL;
}

static stnlabz_module_result_t qualify(stnlabz_module_qualification_result_t *q)
{
    unsigned int e = 0, p = 0;
    int ne = 0, np = 0, ok;
    if (!q) return STNLABZ_MODULE_ERR_INVALID_ARGUMENT;
    memset(q, 0, sizeof *q);
    ok = rictus_web_run_selftests(&e, &p, &ne, &np);
    q->tests_executed = e;
    q->tests_passed = p;
    q->tests_failed = e - p;
    q->negative_test_executed = ne;
    q->negative_test_passed = np;
    return ok ? STNLABZ_MODULE_OK : STNLABZ_MODULE_ERR_QUALIFICATION;
}

static stnlabz_module_result_t start(const stnlabz_module_host_t *h)
{
    if (!h || !h->invoke_service) return STNLABZ_MODULE_ERR_INVALID_ARGUMENT;
    if (!load_secret()) {
        fprintf(stderr, "[WEB] missing or unsafe token file\n");
        return STNLABZ_MODULE_ERR_START_FAILED;
    }
    {
        struct stat key_stat;
        if (lstat(WEB_TLS_KEY, &key_stat) != 0 ||
            !S_ISREG(key_stat.st_mode) || key_stat.st_nlink != 1 ||
            (key_stat.st_mode & 0077) != 0 ||
            (key_stat.st_uid != 0 && key_stat.st_uid != geteuid()))
            goto start_failed;
        web_tls_ctx = SSL_CTX_new(TLS_server_method());
        if (!web_tls_ctx ||
            SSL_CTX_set_min_proto_version(web_tls_ctx, TLS1_2_VERSION) != 1 ||
            SSL_CTX_use_certificate_chain_file(web_tls_ctx, WEB_TLS_CERT) != 1 ||
            SSL_CTX_use_PrivateKey_file(web_tls_ctx, WEB_TLS_KEY, SSL_FILETYPE_PEM) != 1 ||
            SSL_CTX_check_private_key(web_tls_ctx) != 1)
            goto start_failed;
    }
    {
        struct sockaddr_in address;
        int one = 1;
        listen_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (listen_fd < 0) goto start_failed;
        (void)setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
        memset(&address, 0, sizeof address);
        address.sin_family = AF_INET;
        address.sin_port = htons(WEB_PORT);
        address.sin_addr.s_addr = htonl(INADDR_ANY);
        if (bind(listen_fd, (struct sockaddr *)&address, sizeof address) != 0 ||
            listen(listen_fd, 8) != 0) goto start_failed;
    }
    host = h;
    atomic_store(&stopping, 0);
    if (pthread_create(&worker, NULL, serve_loop, NULL) != 0)
        goto start_failed;
    fprintf(stderr, "[WEB] HTTPS listener active on port %d\n", WEB_PORT);
    worker_started = 1;
    return STNLABZ_MODULE_OK;

start_failed:
    if (listen_fd >= 0) close(listen_fd);
    listen_fd = -1;
    host = NULL;
    SSL_CTX_free(web_tls_ctx);
    web_tls_ctx = NULL;
    memset(secret, 0, sizeof secret);
    fprintf(stderr, "[WEB] HTTPS start failed\n");
    return STNLABZ_MODULE_ERR_START_FAILED;
}

static stnlabz_module_result_t stop(void)
{
    atomic_store(&stopping, 1);
    if (worker_started) {
        (void)pthread_join(worker, NULL);
        worker_started = 0;
    }
    if (listen_fd >= 0) close(listen_fd);
    listen_fd = -1;
    host = NULL;
    SSL_CTX_free(web_tls_ctx);
    web_tls_ctx = NULL;
    memset(secret, 0, sizeof secret);
    return STNLABZ_MODULE_OK;
}

static const stnlabz_module_descriptor_t descriptor = {
    "web", "Rictus HTTPS Web Transport",
    WEB_VERSION_MAJOR, WEB_VERSION_MINOR, WEB_VERSION_PATCH,
    STNLABZ_MODULE_API_MAJOR, STNLABZ_MODULE_API_MINOR,
    qualify, start, stop
};
const stnlabz_module_descriptor_t *stnlabz_module_get_descriptor(void)
{
    return &descriptor;
}
