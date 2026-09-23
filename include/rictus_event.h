#ifndef RICTUS_EVENT_H
#define RICTUS_EVENT_H

#include <stddef.h>

#include "rictus_irc_message.h"

typedef enum rictus_event_type {
    RICTUS_EVENT_NONE = 0,
    RICTUS_EVENT_CHANNEL_MESSAGE,
    RICTUS_EVENT_PRIVATE_MESSAGE,
    RICTUS_EVENT_JOIN,
    RICTUS_EVENT_PART,
    RICTUS_EVENT_QUIT
} rictus_event_type;

typedef struct rictus_event {
    rictus_event_type type;
    char source[RICTUS_IRC_PREFIX_MAX];
    char target[RICTUS_IRC_PARAM_MAX];
    char text[RICTUS_IRC_PARAM_MAX];
} rictus_event;

int rictus_event_from_irc(const rictus_irc_message *message,
                          const char *channel,
                          rictus_event *event);

void rictus_event_observe(const rictus_event *event);

#endif
