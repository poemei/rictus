#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rictus_net.h"

typedef struct rictus_windows_socket {
    SOCKET socket;
    int winsock_started;
} rictus_windows_socket;

static void set_error(char *error, size_t error_size, const char *message)
{
    if (error != NULL && error_size > 0U) {
        (void)snprintf(error, error_size, "%s", message);
    }
}

static void set_wsa_error(char *error, size_t error_size, const char *operation, int code)
{
    if (error != NULL && error_size > 0U) {
        (void)snprintf(error, error_size, "%s failed with Winsock error %d", operation, code);
    }
}

int rictus_net_connect(rictus_net_connection *connection,
                       const char *host,
                       unsigned short port,
                       char *error,
                       size_t error_size)
{
    WSADATA wsa;
    struct addrinfo hints;
    struct addrinfo *addresses = NULL;
    struct addrinfo *address;
    rictus_windows_socket *native;
    char service[6];
    int status;

    if (connection == NULL || host == NULL || host[0] == '\0' || port == 0U) {
        set_error(error, error_size, "invalid network connection request");
        return 0;
    }

    connection->native = NULL;

    status = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (status != 0) {
        set_wsa_error(error, error_size, "WSAStartup", status);
        return 0;
    }

    (void)snprintf(service, sizeof(service), "%u", (unsigned int)port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    status = getaddrinfo(host, service, &hints, &addresses);
    if (status != 0) {
        set_wsa_error(error, error_size, "getaddrinfo", status);
        WSACleanup();
        return 0;
    }

    native = (rictus_windows_socket *)malloc(sizeof(*native));
    if (native == NULL) {
        freeaddrinfo(addresses);
        WSACleanup();
        set_error(error, error_size, "unable to allocate network connection");
        return 0;
    }

    native->socket = INVALID_SOCKET;
    native->winsock_started = 1;

    for (address = addresses; address != NULL; address = address->ai_next) {
        native->socket = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
        if (native->socket == INVALID_SOCKET) {
            continue;
        }

        if (connect(native->socket, address->ai_addr, (int)address->ai_addrlen) == 0) {
            break;
        }

        closesocket(native->socket);
        native->socket = INVALID_SOCKET;
    }

    freeaddrinfo(addresses);

    if (native->socket == INVALID_SOCKET) {
        int code = WSAGetLastError();
        free(native);
        WSACleanup();
        set_wsa_error(error, error_size, "connect", code);
        return 0;
    }

    connection->native = native;
    return 1;
}

int rictus_net_native_handle(const rictus_net_connection *connection,
                             void **handle)
{
    rictus_windows_socket *native;

    if (connection == NULL || connection->native == NULL || handle == NULL) {
        return 0;
    }

    native = (rictus_windows_socket *)connection->native;
    *handle = (void *)(uintptr_t)native->socket;
    return 1;
}

void rictus_net_close(rictus_net_connection *connection)
{
    rictus_windows_socket *native;

    if (connection == NULL || connection->native == NULL) {
        return;
    }

    native = (rictus_windows_socket *)connection->native;

    if (native->socket != INVALID_SOCKET) {
        closesocket(native->socket);
    }
    if (native->winsock_started) {
        WSACleanup();
    }

    free(native);
    connection->native = NULL;
}
