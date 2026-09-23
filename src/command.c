#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "rictus_command.h"

static int equal_ignore_case(const char *left, const char *right)
{
    while (*left != '\0' && *right != '\0') {
        if (tolower((unsigned char)*left) != tolower((unsigned char)*right)) {
            return 0;
        }
        ++left;
        ++right;
    }

    return *left == '\0' && *right == '\0';
}

int rictus_command_parse(const rictus_dispatch_result *dispatch,
                         rictus_command *command)
{
    const char *cursor;
    size_t length = 0U;
    size_t arguments_length;

    if (dispatch == NULL || command == NULL ||
        dispatch->action != RICTUS_DISPATCH_COMMAND) {
        return 0;
    }

    memset(command, 0, sizeof(*command));
    cursor = dispatch->text;

    while (*cursor != '\0' && !isspace((unsigned char)*cursor)) {
        if (length + 1U >= sizeof(command->name)) {
            return 0;
        }
        command->name[length++] = *cursor++;
    }
    command->name[length] = '\0';

    if (length == 0U) {
        return 0;
    }

    while (*cursor != '\0' && isspace((unsigned char)*cursor)) {
        ++cursor;
    }

    arguments_length = strlen(cursor);
    if (arguments_length >= sizeof(command->arguments)) {
        return 0;
    }
    memcpy(command->arguments, cursor, arguments_length + 1U);

    if (equal_ignore_case(command->name, "status")) {
        command->type = RICTUS_COMMAND_STATUS;
    } else if (equal_ignore_case(command->name, "help")) {
        command->type = RICTUS_COMMAND_HELP;
    } else {
        command->type = RICTUS_COMMAND_UNKNOWN;
    }

    return 1;
}

int rictus_command_response(const rictus_command *command,
                            char *response,
                            size_t response_size)
{
    int length;

    if (command == NULL || response == NULL || response_size == 0U) {
        return 0;
    }

    switch (command->type) {
        case RICTUS_COMMAND_STATUS:
            length = snprintf(response,
                              response_size,
                              "Rictus online. IRC session authenticated and active.");
            break;
        case RICTUS_COMMAND_HELP:
            length = snprintf(response,
                              response_size,
                              "Commands: !status, !help");
            break;
        case RICTUS_COMMAND_UNKNOWN:
            length = snprintf(response,
                              response_size,
                              "Unknown command: %s",
                              command->name);
            break;
        default:
            return 0;
    }

    return length >= 0 && (size_t)length < response_size;
}
