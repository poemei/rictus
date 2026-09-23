#ifndef RICTUS_TLS_H
#define RICTUS_TLS_H

#include <stddef.h>

#include "rictus_net.h"

typedef struct rictus_tls_connection {
    void *native;
} rictus_tls_connection;

int rictus_tls_connect(rictus_tls_connection *tls,
                       rictus_net_connection *transport,
                       const char *host,
                       char *error,
                       size_t error_size);

void rictus_tls_close(rictus_tls_connection *tls);

#endif
