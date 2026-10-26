#ifndef RICTUS_INTELLIGENCE_INTERNAL_H
#define RICTUS_INTELLIGENCE_INTERNAL_H

#include <stddef.h>

#define RICTUS_INTELLIGENCE_ENV_PATH "/opt/rictus/auth/intelligence/intelligence.env"
#define RICTUS_INTELLIGENCE_LOG_PATH "/var/log/rictus/rictus.log"
#define RICTUS_INTELLIGENCE_STATE_PATH "/var/lib/rictus/intelligence.cursor"

#define RICTUS_INTELLIGENCE_URL_MAX 512
#define RICTUS_INTELLIGENCE_ID_MAX 96
#define RICTUS_INTELLIGENCE_TYPE_MAX 96
#define RICTUS_INTELLIGENCE_LEVEL_MAX 16
#define RICTUS_INTELLIGENCE_TIME_MAX 64

typedef struct {
    char url[RICTUS_INTELLIGENCE_URL_MAX];
    unsigned int interval_seconds;
} rictus_intelligence_config_t;

typedef struct {
    char id[RICTUS_INTELLIGENCE_ID_MAX];
    char type[RICTUS_INTELLIGENCE_TYPE_MAX];
    char severity[RICTUS_INTELLIGENCE_LEVEL_MAX];
    char relevance[RICTUS_INTELLIGENCE_LEVEL_MAX];
    char created_at[RICTUS_INTELLIGENCE_TIME_MAX];
} rictus_intelligence_record_t;

int rictus_intelligence_config_load(const char *path,
                                    rictus_intelligence_config_t *config);
int rictus_intelligence_relevant(const char *relevance);
int rictus_intelligence_extract_records(
    const char *json,
    rictus_intelligence_record_t *records,
    size_t capacity,
    size_t *count);
int rictus_intelligence_run_selftests(unsigned int *executed,
                                      unsigned int *passed,
                                      int *negative_executed,
                                      int *negative_passed);

#endif
