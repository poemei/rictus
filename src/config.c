#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rictus_config.h"

#define RICTUS_CONFIG_FILE_MAX 16384

static void set_error(char *error, size_t error_size, const char *message)
{
    if (error != NULL && error_size > 0U) {
        (void)snprintf(error, error_size, "%s", message);
    }
}

static const char *skip_ws(const char *p)
{
    while (*p != '\0' && isspace((unsigned char)*p)) {
        ++p;
    }
    return p;
}

static const char *find_value(const char *json, const char *key)
{
    char needle[96];
    const char *p;
    int written;

    written = snprintf(needle, sizeof(needle), "\"%s\"", key);
    if (written < 0 || (size_t)written >= sizeof(needle)) {
        return NULL;
    }

    p = strstr(json, needle);
    if (p == NULL) {
        return NULL;
    }

    p += strlen(needle);
    p = skip_ws(p);
    if (*p != ':') {
        return NULL;
    }

    return skip_ws(p + 1);
}

static int parse_string(const char *json, const char *key, char *out, size_t out_size)
{
    const char *p;
    size_t used = 0U;

    p = find_value(json, key);
    if (p == NULL || *p != '\"' || out_size == 0U) {
        return 0;
    }
    ++p;

    while (*p != '\0' && *p != '\"') {
        char value = *p++;

        if (value == '\\') {
            value = *p++;
            if (value == '\0') {
                return 0;
            }
            switch (value) {
                case '\"': value = '\"'; break;
                case '\\': value = '\\'; break;
                case '/': value = '/'; break;
                case 'b': value = '\b'; break;
                case 'f': value = '\f'; break;
                case 'n': value = '\n'; break;
                case 'r': value = '\r'; break;
                case 't': value = '\t'; break;
                default: return 0;
            }
        }

        if (used + 1U >= out_size) {
            return 0;
        }
        out[used++] = value;
    }

    if (*p != '\"') {
        return 0;
    }

    out[used] = '\0';
    return 1;
}

static int parse_port(const char *json, unsigned short *port)
{
    const char *p;
    char *end;
    unsigned long value;

    p = find_value(json, "port");
    if (p == NULL) {
        return 0;
    }

    errno = 0;
    value = strtoul(p, &end, 10);
    if (errno != 0 || end == p || value == 0UL || value > 65535UL) {
        return 0;
    }

    end = (char *)skip_ws(end);
    if (*end != ',' && *end != '}') {
        return 0;
    }

    *port = (unsigned short)value;
    return 1;
}

static int parse_bool(const char *json, const char *key, int *value)
{
    const char *p = find_value(json, key);

    if (p == NULL) {
        return 0;
    }
    if (strncmp(p, "true", 4U) == 0) {
        *value = 1;
        return 1;
    }
    if (strncmp(p, "false", 5U) == 0) {
        *value = 0;
        return 1;
    }
    return 0;
}

int rictus_config_load(const char *path, rictus_config *config, char *error, size_t error_size)
{
    FILE *file;
    long length;
    size_t read_count;
    char *json;

    if (path == NULL || config == NULL) {
        set_error(error, error_size, "invalid configuration request");
        return 0;
    }

    file = fopen(path, "rb");
    if (file == NULL) {
        set_error(error, error_size, "unable to open rictus.json");
        return 0;
    }

    if (fseek(file, 0L, SEEK_END) != 0) {
        fclose(file);
        set_error(error, error_size, "unable to read rictus.json");
        return 0;
    }

    length = ftell(file);
    if (length < 0L || length > RICTUS_CONFIG_FILE_MAX) {
        fclose(file);
        set_error(error, error_size, "rictus.json has an invalid size");
        return 0;
    }

    if (fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        set_error(error, error_size, "unable to read rictus.json");
        return 0;
    }

    json = (char *)malloc((size_t)length + 1U);
    if (json == NULL) {
        fclose(file);
        set_error(error, error_size, "unable to allocate configuration buffer");
        return 0;
    }

    read_count = fread(json, 1U, (size_t)length, file);
    fclose(file);
    if (read_count != (size_t)length) {
        free(json);
        set_error(error, error_size, "unable to read rictus.json");
        return 0;
    }
    json[read_count] = '\0';

    memset(config, 0, sizeof(*config));

    if (strstr(json, "\"irc\"") == NULL ||
        !parse_string(json, "server", config->irc.server, sizeof(config->irc.server)) ||
        !parse_port(json, &config->irc.port) ||
        !parse_bool(json, "tls", &config->irc.tls) ||
        !parse_string(json, "username", config->irc.username, sizeof(config->irc.username)) ||
        !parse_string(json, "password", config->irc.password, sizeof(config->irc.password)) ||
        !parse_string(json, "channel", config->irc.channel, sizeof(config->irc.channel))) {
        free(json);
        set_error(error, error_size, "rictus.json is missing or contains an invalid IRC setting");
        return 0;
    }

    free(json);

    if (config->irc.server[0] == '\0' ||
        config->irc.username[0] == '\0' ||
        config->irc.password[0] == '\0' ||
        config->irc.channel[0] != '#') {
        set_error(error, error_size, "rictus.json contains an invalid IRC configuration");
        return 0;
    }

    return 1;
}
