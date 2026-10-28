#ifndef RICTUS_CORE_DIAGNOSTICS_H
#define RICTUS_CORE_DIAGNOSTICS_H

#include <stddef.h>
#include "stnlabz_abi/module.h"

#define RICTUS_DIAG_TEXT_MAX 2048
#define RICTUS_DIAG_SERVICE_MAX 16

void rictus_diag_begin(const char *sender, const char *account, const char *text);
void rictus_diag_route(const char *command);
void rictus_diag_command(const char *command, const char *owner,
                         stnlabz_module_result_t result, int replied);
void rictus_diag_service(const char *service, const char *owner,
                         stnlabz_module_result_t result);
int rictus_diag_render(char *output, size_t output_size);

#endif
