#ifndef RICTUS_IRC_MESSAGE_H
#define RICTUS_IRC_MESSAGE_H

#include <stddef.h>

#define RICTUS_IRC_PREFIX_MAX 256
#define RICTUS_IRC_COMMAND_MAX 32
#define RICTUS_IRC_PARAMS_MAX 15
#define RICTUS_IRC_PARAM_MAX 512

typedef struct rictus_irc_message {
    char prefix[RICTUS_IRC_PREFIX_MAX];
    char command[RICTUS_IRC_COMMAND_MAX];
    char params[RICTUS_IRC_PARAMS_MAX][RICTUS_IRC_PARAM_MAX];
    size_t param_count;
} rictus_irc_message;

int rictus_irc_message_parse(const char *line, rictus_irc_message *message);

#endif
