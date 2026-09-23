#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "rictus_net.h"

typedef struct rictus_linux_socket {
    int fd;
} rictus_linux_socket;

static void set_error(char *error, size_t error_size, const char *message)
{
    if (error != NULL && error_size > 0U) {
        (void)snprintf(error, error_size, "%s", message);
    }
}

int rictus_net_connect(rictus_net_connection *connection,
                       const char *host,
                       unsigned short port,
                       char *error,
                       size_t error_size)
{
    struct addrinfo hints;
    struct addrinfo *addresses = NULL;
    struct addrinfo *address;
    rictus_linux_socket *native;
    char service[6];
    int status;

    if (connection == NULL || host == NULL || host[0] == '\0' || port == 0U) {
        set_error(error, error_size, "invalid network connection request");
        return 0;
    }

    connection->native = NULL;
    (void)snprintf(service, sizeof(service), "%u", (unsigned int)port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    status = getaddrinfo(host, service, &hints, &addresses);
    if (status != 0) {
        set_error(error, error_size, gai_strerror(status));
        return 0;
    }

    native = (rictus_linux_socket *)malloc(sizeof(*native));
    if (native == NULL) {
        freeaddrinfo(addresses);
        set_error(error, error_size, "unable to allocate network connection");
        return 0;
    }
    native->fd = -1;

    for (address = addresses; address != NULL; address = address->ai_next) {
        native->fd = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
        if (native->fd < 0) {
            continue;
        }

        if (connect(native->fd, address->ai_addr, address->ai_addrlen) == 0) {
            break;
        }

        (void)close(native->fd);
        native->fd = -1;
    }

    freeaddrinfo(addresses);

    if (native->fd < 0) {
        free(native);
        set_error(error, error_size, strerror(errno));
        return 0;
    }

    connection->native = native;
    return 1;
}

int rictus_net_native_handle(const rictus_net_connection *connection,
                             void **handle)
{
    rictus_linux_socket *native;

    if (connection == NULL || connection->native == NULL || handle == NULL) {
        return 0;
    }

    native = (rictus_linux_socket *)connection->native;
    *handle = &native->fd;
    return 1;
}

void rictus_net_close(rictus_net_connection *connection)
{
    rictus_linux_socket *native;

    if (connection == NULL || connection->native == NULL) {
        return;
    }

    native = (rictus_linux_socket *)connection->native;
    if (native->fd >= 0) {
        (void)close(native->fd);
    }

    free(native);
    connection->native = NULL;
}
