#include <stdio.h>

#include "rictus.h"
#include "rictus_config.h"
#include "rictus_net.h"
#include "rictus_tls.h"

int rictus_run(void)
{
    rictus_config config;
    rictus_net_connection connection;
    rictus_tls_connection tls;
    char error[256];

    puts("STN-LABZ Rictus");

    if (!rictus_config_load(RICTUS_CONFIG_DEFAULT_PATH, &config, error, sizeof(error))) {
        fprintf(stderr, "[ERROR] Configuration: %s\n", error);
        return 1;
    }

    printf("[INFO] IRC configuration loaded: server=%s port=%u tls=%s user=%s channel=%s\n",
           config.irc.server,
           (unsigned int)config.irc.port,
           config.irc.tls ? "true" : "false",
           config.irc.username,
           config.irc.channel);

    printf("[INFO] Connecting to %s:%u\n",
           config.irc.server,
           (unsigned int)config.irc.port);

    if (!rictus_net_connect(&connection,
                            config.irc.server,
                            config.irc.port,
                            error,
                            sizeof(error))) {
        fprintf(stderr, "[ERROR] IRC transport: %s\n", error);
        return 1;
    }

    puts("[INFO] IRC transport connected.");

    if (!config.irc.tls) {
        fprintf(stderr, "[ERROR] IRC TLS is required for this Rictus connection.\n");
        rictus_net_close(&connection);
        return 1;
    }

    if (!rictus_tls_connect(&tls, &connection, config.irc.server, error, sizeof(error))) {
        fprintf(stderr, "[ERROR] IRC TLS: %s\n", error);
        rictus_net_close(&connection);
        return 1;
    }

    puts("[INFO] IRC TLS established and certificate verified.");
    rictus_tls_close(&tls);
    rictus_net_close(&connection);
    puts("Rictus initialized.");

    return 0;
}
