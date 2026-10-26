#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/x509_vfy.h>
#include "stnlabz_abi/module.h"
#include "irc_internal.h"

#define IRC_VERSION_MAJOR 1U
#define IRC_VERSION_MINOR 0U
#define IRC_VERSION_PATCH 0U

static pthread_t worker;
static atomic_int stop_requested;
static atomic_int active_socket;
static int worker_started;
static rictus_irc_config_t config;

static void status(const char *message)
{
    if (message && *message) {
        fprintf(stderr, "[IRC] %s\n", message);
        fflush(stderr);
    }
}

static int sleep_interruptible(unsigned int seconds)
{
    struct timespec delay = { 1, 0 };
    unsigned int i;
    for (i = 0; i < seconds; ++i) {
        struct timespec remaining = delay;
        if (atomic_load(&stop_requested)) return 0;
        while (nanosleep(&remaining, &remaining) != 0) {
            if (errno == EINTR) {
                if (atomic_load(&stop_requested)) return 0;
                continue;
            }
            return 0;
        }
    }
    return !atomic_load(&stop_requested);
}

static int tcp_connect(const char *server, unsigned short port)
{
    struct addrinfo hints;
    struct addrinfo *results = NULL;
    struct addrinfo *item;
    char service[16];
    int fd = -1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if (snprintf(service, sizeof service, "%u", (unsigned)port) < 0) return -1;
    if (getaddrinfo(server, service, &hints, &results) != 0) return -1;
    for (item = results; item; item = item->ai_next) {
        fd = socket(item->ai_family, item->ai_socktype, item->ai_protocol);
        if (fd < 0) continue;
        if (connect(fd, item->ai_addr, item->ai_addrlen) == 0) break;
        close(fd);
        fd = -1;
    }
    freeaddrinfo(results);
    return fd;
}

static int ssl_write_all(SSL *ssl, const char *data, size_t length)
{
    size_t offset = 0U;
    while (offset < length) {
        int written = SSL_write(ssl, data + offset, (int)(length - offset));
        if (written <= 0) return 0;
        offset += (size_t)written;
    }
    return 1;
}

static int send_line(SSL *ssl, const char *line)
{
    char buffer[RICTUS_IRC_LINE_MAX + 3U];
    int written;
    if (!ssl || !line) return 0;
    written = snprintf(buffer, sizeof buffer, "%s\r\n", line);
    if (written < 0 || (size_t)written >= sizeof buffer) return 0;
    return ssl_write_all(ssl, buffer, (size_t)written);
}

static int send_authenticate_plain(SSL *ssl)
{
    char encoded[1024];
    size_t length;
    size_t offset = 0U;

    if (!rictus_irc_build_plain(config.account, config.password,
                                encoded, sizeof encoded)) return 0;
    length = strlen(encoded);
    while (offset < length) {
        char line[420];
        size_t chunk = length - offset;
        int written;
        if (chunk > 400U) chunk = 400U;
        written = snprintf(line, sizeof line, "AUTHENTICATE %.*s",
                           (int)chunk, encoded + offset);
        if (written < 0 || (size_t)written >= sizeof line || !send_line(ssl, line))
            return 0;
        offset += chunk;
    }
    if (length % 400U == 0U && !send_line(ssl, "AUTHENTICATE +")) return 0;
    OPENSSL_cleanse(encoded, sizeof encoded);
    return 1;
}

static int cap_is_continuation(const char *params)
{
    return params && strstr(params, " LS * ") != NULL;
}

static int handle_line(SSL *ssl, const char *line, int *sasl_seen,
                       int *sasl_requested, int *authenticated,
                       int *registered, int *joined)
{
    char command[32];
    const char *params;

    if (!rictus_irc_line_command(line, command, sizeof command, &params)) return 1;

    if (!strcmp(command, "PING")) {
        char response[RICTUS_IRC_LINE_MAX];
        int written = snprintf(response, sizeof response, "PONG %s", params);
        return written >= 0 && (size_t)written < sizeof response && send_line(ssl, response);
    }

    if (!strcmp(command, "CAP")) {
        if (strstr(params, " LS ") || strstr(params, " LS :")) {
            if (rictus_irc_word_present(params, "sasl")) *sasl_seen = 1;
            if (!cap_is_continuation(params) && !*sasl_requested) {
                if (!*sasl_seen) return 0;
                if (!send_line(ssl, "CAP REQ :sasl")) return 0;
                *sasl_requested = 1;
            }
        } else if ((strstr(params, " ACK ") || strstr(params, " ACK :")) &&
                   rictus_irc_word_present(params, "sasl")) {
            if (!send_line(ssl, "AUTHENTICATE PLAIN")) return 0;
        } else if (strstr(params, " NAK ") || strstr(params, " NAK :")) {
            return 0;
        }
        return 1;
    }

    if (!strcmp(command, "AUTHENTICATE") && !strcmp(params, "+"))
        return send_authenticate_plain(ssl);

    if (!strcmp(command, "903")) {
        *authenticated = 1;
        return send_line(ssl, "CAP END");
    }
    if (!strcmp(command, "904") || !strcmp(command, "905") ||
        !strcmp(command, "906") || !strcmp(command, "907")) return 0;

    if (!strcmp(command, "001")) {
        *registered = 1;
        if (*authenticated && !*joined) {
            char join[RICTUS_IRC_CHANNEL_MAX + 8U];
            int written = snprintf(join, sizeof join, "JOIN %s", config.channel);
            if (written < 0 || (size_t)written >= sizeof join || !send_line(ssl, join)) return 0;
            *joined = 1;
            status("authenticated and join requested");
        }
        return 1;
    }

    if (!strcmp(command, "433")) return 0;

    if (!strcmp(command, "KICK") && strstr(params, config.channel) && strstr(params, config.nick)) {
        char join[RICTUS_IRC_CHANNEL_MAX + 8U];
        int written = snprintf(join, sizeof join, "JOIN %s", config.channel);
        if (written < 0 || (size_t)written >= sizeof join) return 0;
        *joined = 0;
        if (!send_line(ssl, join)) return 0;
        *joined = 1;
        status("rejoin requested after kick");
    }

    if (*registered && *authenticated && !*joined) {
        char join[RICTUS_IRC_CHANNEL_MAX + 8U];
        int written = snprintf(join, sizeof join, "JOIN %s", config.channel);
        if (written < 0 || (size_t)written >= sizeof join || !send_line(ssl, join)) return 0;
        *joined = 1;
        status("authenticated and join requested");
    }
    return 1;
}

static int run_session(void)
{
    SSL_CTX *ctx = NULL;
    SSL *ssl = NULL;
    int fd = -1;
    int ok = 0;
    int sasl_seen = 0;
    int sasl_requested = 0;
    int authenticated = 0;
    int registered = 0;
    int joined = 0;
    char receive_buffer[8192];
    size_t receive_used = 0U;

    fd = tcp_connect(config.server, config.port);
    if (fd < 0) goto done;
    atomic_store(&active_socket, fd);

    ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) goto done;
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    if (SSL_CTX_set_default_verify_paths(ctx) != 1) goto done;
    ssl = SSL_new(ctx);
    if (!ssl) goto done;
    if (SSL_set_tlsext_host_name(ssl, config.server) != 1) goto done;
    if (X509_VERIFY_PARAM_set1_host(SSL_get0_param(ssl), config.server, 0) != 1) goto done;
    if (SSL_set_fd(ssl, fd) != 1 || SSL_connect(ssl) != 1) goto done;
    if (SSL_get_verify_result(ssl) != X509_V_OK) goto done;

    if (!send_line(ssl, "CAP LS 302")) goto done;
    {
        char line[RICTUS_IRC_NICK_MAX + 8U];
        int written = snprintf(line, sizeof line, "NICK %s", config.nick);
        if (written < 0 || (size_t)written >= sizeof line || !send_line(ssl, line)) goto done;
    }
    {
        char line[RICTUS_IRC_NICK_MAX + 64U];
        int written = snprintf(line, sizeof line, "USER %s 0 * :STN-LABZ Rictus", config.nick);
        if (written < 0 || (size_t)written >= sizeof line || !send_line(ssl, line)) goto done;
    }
    status("TLS connected; negotiating CAP and SASL");

    while (!atomic_load(&stop_requested)) {
        int received;
        char *newline;
        if (receive_used >= sizeof receive_buffer - 1U) goto done;
        received = SSL_read(ssl, receive_buffer + receive_used,
                            (int)(sizeof receive_buffer - receive_used - 1U));
        if (received <= 0) goto done;
        receive_used += (size_t)received;
        receive_buffer[receive_used] = '\0';

        while ((newline = strstr(receive_buffer, "\r\n")) != NULL) {
            size_t line_length = (size_t)(newline - receive_buffer);
            char line[RICTUS_IRC_LINE_MAX];
            size_t consumed = line_length + 2U;
            if (line_length >= sizeof line) goto done;
            memcpy(line, receive_buffer, line_length);
            line[line_length] = '\0';
            memmove(receive_buffer, receive_buffer + consumed, receive_used - consumed);
            receive_used -= consumed;
            receive_buffer[receive_used] = '\0';
            if (!handle_line(ssl, line, &sasl_seen, &sasl_requested,
                             &authenticated, &registered, &joined)) goto done;
        }
    }
    ok = 1;

done:
    atomic_store(&active_socket, -1);
    if (ssl) {
        if (ok) (void)SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    if (ctx) SSL_CTX_free(ctx);
    if (fd >= 0) close(fd);
    return ok;
}

static void *irc_worker(void *unused)
{
    unsigned int backoff = 5U;
    (void)unused;
    while (!atomic_load(&stop_requested)) {
        if (run_session()) break;
        if (atomic_load(&stop_requested)) break;
        status("connection/session lost; retrying");
        if (!sleep_interruptible(backoff)) break;
        if (backoff < 60U) {
            backoff *= 2U;
            if (backoff > 60U) backoff = 60U;
        }
    }
    return NULL;
}

static stnlabz_module_result_t irc_qualify(stnlabz_module_qualification_result_t *result)
{
    unsigned int executed = 0U;
    unsigned int passed = 0U;
    int negative_executed = 0;
    int negative_passed = 0;
    int ok;

    if (!result) return STNLABZ_MODULE_ERR_INVALID_ARGUMENT;
    memset(result, 0, sizeof *result);
    ok = rictus_irc_run_selftests(&executed, &passed,
                                  &negative_executed, &negative_passed);
    result->tests_executed = executed;
    result->tests_passed = passed;
    result->tests_failed = executed - passed;
    result->negative_test_executed = negative_executed;
    result->negative_test_passed = negative_passed;
    return ok ? STNLABZ_MODULE_OK : STNLABZ_MODULE_ERR_QUALIFICATION;
}

static stnlabz_module_result_t irc_start(const stnlabz_module_host_t *host)
{
    (void)host;
    if (worker_started) return STNLABZ_MODULE_ERR_INVALID_STATE;
    if (!rictus_irc_config_load(RICTUS_IRC_ENV_PATH, &config)) {
        status("credential/config load failed");
        return STNLABZ_MODULE_ERR_START_FAILED;
    }
    if (OPENSSL_init_ssl(0, NULL) != 1) {
        rictus_irc_config_clear(&config);
        return STNLABZ_MODULE_ERR_START_FAILED;
    }
    atomic_store(&stop_requested, 0);
    atomic_store(&active_socket, -1);
    if (pthread_create(&worker, NULL, irc_worker, NULL) != 0) {
        rictus_irc_config_clear(&config);
        return STNLABZ_MODULE_ERR_START_FAILED;
    }
    worker_started = 1;
    status("transport worker started");
    return STNLABZ_MODULE_OK;
}

static stnlabz_module_result_t irc_stop(void)
{
    int fd;
    if (!worker_started) return STNLABZ_MODULE_ERR_INVALID_STATE;
    atomic_store(&stop_requested, 1);
    fd = atomic_load(&active_socket);
    if (fd >= 0) (void)shutdown(fd, SHUT_RDWR);
    if (pthread_join(worker, NULL) != 0) return STNLABZ_MODULE_ERR_STOP_FAILED;
    worker_started = 0;
    rictus_irc_config_clear(&config);
    status("transport worker stopped");
    return STNLABZ_MODULE_OK;
}

static const stnlabz_module_descriptor_t descriptor = {
    "irc",
    "Rictus IRC Transport",
    IRC_VERSION_MAJOR,
    IRC_VERSION_MINOR,
    IRC_VERSION_PATCH,
    STNLABZ_MODULE_API_MAJOR,
    STNLABZ_MODULE_API_MINOR,
    irc_qualify,
    irc_start,
    irc_stop
};

const stnlabz_module_descriptor_t *stnlabz_module_get_descriptor(void)
{
    return &descriptor;
}
