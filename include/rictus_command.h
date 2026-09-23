#ifndef RICTUS_COMMAND_H
#define RICTUS_COMMAND_H

#include <stddef.h>

#include "rictus_dispatch.h"

#define RICTUS_COMMAND_NAME_MAX 64
#define RICTUS_COMMAND_ARGS_MAX RICTUS_IRC_PARAM_MAX

typedef enum rictus_command_type {
    RICTUS_COMMAND_NONE = 0,
    RICTUS_COMMAND_STATUS,
    RICTUS_COMMAND_HELP,
    RICTUS_COMMAND_UNKNOWN
} rictus_command_type;

typedef struct rictus_command {
    rictus_command_type type;
    char name[RICTUS_COMMAND_NAME_MAX];
    char arguments[RICTUS_COMMAND_ARGS_MAX];
} rictus_command;

int rictus_command_parse(const rictus_dispatch_result *dispatch,
                         rictus_command *command);

int rictus_command_response(const rictus_command *command,
                            char *response,
                            size_t response_size);

#endif
