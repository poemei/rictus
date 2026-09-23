#ifndef RICTUS_CONFIG_H
#define RICTUS_CONFIG_H

#include <stddef.h>

#define RICTUS_CONFIG_DEFAULT_PATH "rictus.json"
#define RICTUS_CONFIG_SERVER_MAX 256
#define RICTUS_CONFIG_USERNAME_MAX 64
#define RICTUS_CONFIG_PASSWORD_MAX 256
#define RICTUS_CONFIG_CHANNEL_MAX 128

typedef struct rictus_irc_config {
    char server[RICTUS_CONFIG_SERVER_MAX];
    unsigned short port;
    int tls;
    char username[RICTUS_CONFIG_USERNAME_MAX];
    char password[RICTUS_CONFIG_PASSWORD_MAX];
    char channel[RICTUS_CONFIG_CHANNEL_MAX];
} rictus_irc_config;

typedef struct rictus_config {
    rictus_irc_config irc;
} rictus_config;

int rictus_config_load(const char *path, rictus_config *config, char *error, size_t error_size);

#endif
