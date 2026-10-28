#ifndef RICTUS_MODULE_HOST_H
#define RICTUS_MODULE_HOST_H

#include "stnlabz_abi/module.h"

const stnlabz_module_host_t *rictus_module_host(void);
void rictus_module_host_begin_registration(const char *module_id);
void rictus_module_host_end_registration(void);

#endif
