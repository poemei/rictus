#include <stdio.h>
#include <string.h>

#include "rictus_event.h"

static void copy_string(char *destination, size_t size, const char *source)
{
    if (destination == NULL || size == 0U) {
        return;
    }

    if (source == NULL) {
        destination[0] = '\0';
        return;
    }

    (void)snprintf(destination, size, "%s", source);
}

int rictus_event_from_irc(const rictus_irc_message *message,
                          const char *channel,
                          rictus_event *event)
{
    if (message == NULL || channel == NULL || event == NULL) {
        return 0;
    }

    memset(event, 0, sizeof(*event));

    if (strcmp(message->command, "PRIVMSG") == 0 && message->param_count >= 2U) {
        event->type = strcmp(message->params[0], channel) == 0
            ? RICTUS_EVENT_CHANNEL_MESSAGE
            : RICTUS_EVENT_PRIVATE_MESSAGE;
        copy_string(event->source, sizeof(event->source), message->prefix);
        copy_string(event->target, sizeof(event->target), message->params[0]);
        copy_string(event->text, sizeof(event->text), message->params[1]);
        return 1;
    }

    if (strcmp(message->command, "JOIN") == 0 && message->param_count >= 1U) {
        event->type = RICTUS_EVENT_JOIN;
        copy_string(event->source, sizeof(event->source), message->prefix);
        copy_string(event->target, sizeof(event->target), message->params[0]);
        return 1;
    }

    if (strcmp(message->command, "PART") == 0 && message->param_count >= 1U) {
        event->type = RICTUS_EVENT_PART;
        copy_string(event->source, sizeof(event->source), message->prefix);
        copy_string(event->target, sizeof(event->target), message->params[0]);
        if (message->param_count >= 2U) {
            copy_string(event->text, sizeof(event->text), message->params[1]);
        }
        return 1;
    }

    if (strcmp(message->command, "QUIT") == 0) {
        event->type = RICTUS_EVENT_QUIT;
        copy_string(event->source, sizeof(event->source), message->prefix);
        if (message->param_count >= 1U) {
            copy_string(event->text, sizeof(event->text), message->params[0]);
        }
        return 1;
    }

    return 0;
}

void rictus_event_observe(const rictus_event *event)
{
    const char *name;

    if (event == NULL || event->type == RICTUS_EVENT_NONE) {
        return;
    }

    switch (event->type) {
        case RICTUS_EVENT_CHANNEL_MESSAGE:
            name = "CHANNEL_MESSAGE";
            break;
        case RICTUS_EVENT_PRIVATE_MESSAGE:
            name = "PRIVATE_MESSAGE";
            break;
        case RICTUS_EVENT_JOIN:
            name = "JOIN";
            break;
        case RICTUS_EVENT_PART:
            name = "PART";
            break;
        case RICTUS_EVENT_QUIT:
            name = "QUIT";
            break;
        default:
            return;
    }

    printf("[EVENT] %s source=%s", name, event->source);
    if (event->target[0] != '\0') {
        printf(" target=%s", event->target);
    }
    if (event->text[0] != '\0') {
        printf(" text=%s", event->text);
    }
    putchar('\n');
}
