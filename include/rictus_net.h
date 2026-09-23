#ifndef RICTUS_NET_H
#define RICTUS_NET_H

#include <stddef.h>

typedef struct rictus_net_connection {
    void *native;
} rictus_net_connection;

int rictus_net_connect(rictus_net_connection *connection,
                       const char *host,
                       unsigned short port,
                       char *error,
                       size_t error_size);

int rictus_net_native_handle(const rictus_net_connection *connection,
                             void **handle);

void rictus_net_close(rictus_net_connection *connection);

#endif
