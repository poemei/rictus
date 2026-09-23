#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "rictus_dispatch.h"

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

static const char *skip_space(const char *text)
{
    while (text != NULL && *text != '\0' &&
           isspace((unsigned char)*text)) {
        ++text;
    }
    return text;
}

int rictus_dispatch_event(const rictus_event *event,
                          rictus_dispatch_result *result)
{
    const char *text;

    if (event == NULL || result == NULL) {
        return 0;
    }

    memset(result, 0, sizeof(*result));

    if (event->type != RICTUS_EVENT_CHANNEL_MESSAGE &&
        event->type != RICTUS_EVENT_PRIVATE_MESSAGE) {
        return 0;
    }

    copy_string(result->source, sizeof(result->source), event->source);
    copy_string(result->target, sizeof(result->target), event->target);

    text = skip_space(event->text);
    if (text != NULL && *text == '!') {
        ++text;
        text = skip_space(text);
        if (*text == '\0') {
            return 0;
        }

        result->action = RICTUS_DISPATCH_COMMAND;
        copy_string(result->text, sizeof(result->text), text);
        return 1;
    }

    result->action = RICTUS_DISPATCH_OBSERVATION;
    copy_string(result->text, sizeof(result->text), event->text);
    return 1;
}

void rictus_dispatch_observe(const rictus_dispatch_result *result)
{
    const char *name;

    if (result == NULL || result->action == RICTUS_DISPATCH_NONE) {
        return;
    }

    name = result->action == RICTUS_DISPATCH_COMMAND
        ? "COMMAND"
        : "OBSERVATION";

    printf("[DISPATCH] %s source=%s target=%s text=%s\n",
           name,
           result->source,
           result->target,
           result->text);
}
