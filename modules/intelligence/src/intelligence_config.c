#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "intelligence_internal.h"

static int copy_value(char *destination, size_t size, const char *value)
{
    size_t length;
    if (!destination || !size || !value) return 0;
    length = strlen(value);
    if (!length || length >= size) return 0;
    memcpy(destination, value, length + 1U);
    return 1;
}

static int valid_url(const char *value)
{
    return value &&
        (!strncmp(value, "https://", 8U) || !strncmp(value, "http://", 7U)) &&
        !strchr(value, '\r') && !strchr(value, '\n');
}

static int parse_interval(const char *value, unsigned int *interval)
{
    char *end = NULL;
    unsigned long parsed;

    if (!value || !*value || !interval) return 0;
    parsed = strtoul(value, &end, 10);
    if (!end || *end || parsed < 15UL || parsed > 3600UL) return 0;
    *interval = (unsigned int)parsed;
    return 1;
}

int rictus_intelligence_config_load(const char *path,
                                    rictus_intelligence_config_t *config)
{
    FILE *file;
    char line[1024];
    int seen_url = 0;
    int seen_interval = 0;

    if (!path || !config) return 0;
    memset(config, 0, sizeof *config);
    config->interval_seconds = 60U;

    file = fopen(path, "r");
    if (!file) return 0;

    while (fgets(line, sizeof line, file)) {
        char *equals;
        char *value;
        size_t length = strlen(line);

        if (length && line[length - 1] != '\n' && !feof(file)) goto invalid;
        while (length && (line[length - 1] == '\n' || line[length - 1] == '\r'))
            line[--length] = '\0';
        if (!length || line[0] == '#') continue;

        equals = strchr(line, '=');
        if (!equals || equals == line) goto invalid;
        *equals = '\0';
        value = equals + 1;
        if (!*value) goto invalid;

        if (!strcmp(line, "RICTUS_INTELLIGENCE_URL")) {
            if (seen_url || !valid_url(value) ||
                !copy_value(config->url, sizeof config->url, value))
                goto invalid;
            seen_url = 1;
        } else if (!strcmp(line, "RICTUS_INTELLIGENCE_INTERVAL")) {
            if (seen_interval || !parse_interval(value, &config->interval_seconds))
                goto invalid;
            seen_interval = 1;
        } else {
            goto invalid;
        }
    }

    if (ferror(file) || !seen_url) goto invalid;
    fclose(file);
    return 1;

invalid:
    fclose(file);
    memset(config, 0, sizeof *config);
    return 0;
}
