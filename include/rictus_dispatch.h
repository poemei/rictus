#ifndef RICTUS_DISPATCH_H
#define RICTUS_DISPATCH_H

#include <stddef.h>

#include "rictus_event.h"

typedef enum rictus_dispatch_action {
    RICTUS_DISPATCH_NONE = 0,
    RICTUS_DISPATCH_COMMAND,
    RICTUS_DISPATCH_OBSERVATION
} rictus_dispatch_action;

typedef struct rictus_dispatch_result {
    rictus_dispatch_action action;
    char source[RICTUS_IRC_PREFIX_MAX];
    char target[RICTUS_IRC_PARAM_MAX];
    char text[RICTUS_IRC_PARAM_MAX];
} rictus_dispatch_result;

int rictus_dispatch_event(const rictus_event *event,
                          rictus_dispatch_result *result);

void rictus_dispatch_observe(const rictus_dispatch_result *result);

#endif
