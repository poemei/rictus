#ifndef RICTUS_MODULE_WATCH_H
#define RICTUS_MODULE_WATCH_H

#include <pthread.h>

#include "rictus_module_discovery.h"

typedef struct rictus_module_watch {
    pthread_t thread;
    char modules_path[RICTUS_MODULE_DISCOVERY_PATH_MAX];
    rictus_module_candidate_t known[RICTUS_MODULE_LOADER_MAX];
    size_t known_count;
    int running;
} rictus_module_watch_t;

int rictus_module_watch_start(
    rictus_module_watch_t *watch,
    const char *modules_path,
    const rictus_module_candidate_t *initial,
    size_t initial_count);

#endif
