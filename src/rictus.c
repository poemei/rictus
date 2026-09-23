#include <stdio.h>

#include "rictus.h"
#include "rictus_config.h"

int rictus_run(void)
{
    rictus_config config;
    char error[160];

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
    puts("Rictus initialized.");

    return 0;
}
