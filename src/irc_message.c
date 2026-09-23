#include <ctype.h>
#include <string.h>

#include "rictus_irc_message.h"

static int copy_token(char *destination,
                      size_t destination_size,
                      const char *start,
                      size_t length)
{
    if (destination == NULL || destination_size == 0U ||
        start == NULL || length >= destination_size) {
        return 0;
    }

    memcpy(destination, start, length);
    destination[length] = '\0';
    return 1;
}

int rictus_irc_message_parse(const char *line, rictus_irc_message *message)
{
    const char *cursor;
    const char *start;

    if (line == NULL || message == NULL || line[0] == '\0') {
        return 0;
    }

    memset(message, 0, sizeof(*message));
    cursor = line;

    if (*cursor == ':') {
        ++cursor;
        start = cursor;
        while (*cursor != '\0' && *cursor != ' ') {
            ++cursor;
        }
        if (*cursor != ' ' ||
            !copy_token(message->prefix,
                        sizeof(message->prefix),
                        start,
                        (size_t)(cursor - start))) {
            return 0;
        }
        while (*cursor == ' ') {
            ++cursor;
        }
    }

    start = cursor;
    while (*cursor != '\0' && *cursor != ' ') {
        ++cursor;
    }
    if (cursor == start ||
        !copy_token(message->command,
                    sizeof(message->command),
                    start,
                    (size_t)(cursor - start))) {
        return 0;
    }

    {
        size_t i;
        for (i = 0U; message->command[i] != '\0'; ++i) {
            message->command[i] = (char)toupper((unsigned char)message->command[i]);
        }
    }

    while (*cursor != '\0') {
        while (*cursor == ' ') {
            ++cursor;
        }
        if (*cursor == '\0') {
            break;
        }
        if (message->param_count >= RICTUS_IRC_PARAMS_MAX) {
            return 0;
        }

        if (*cursor == ':') {
            ++cursor;
            if (!copy_token(message->params[message->param_count],
                            sizeof(message->params[message->param_count]),
                            cursor,
                            strlen(cursor))) {
                return 0;
            }
            ++message->param_count;
            return 1;
        }

        start = cursor;
        while (*cursor != '\0' && *cursor != ' ') {
            ++cursor;
        }
        if (!copy_token(message->params[message->param_count],
                        sizeof(message->params[message->param_count]),
                        start,
                        (size_t)(cursor - start))) {
            return 0;
        }
        ++message->param_count;
    }

    return 1;
}
