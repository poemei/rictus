#ifndef RICTUS_IRC_H
#define RICTUS_IRC_H

#include <stddef.h>

#include "rictus_config.h"
#include "rictus_module.h"
#include "rictus_tls.h"


int rictus_irc_run(rictus_tls_connection *tls,
                   const rictus_irc_config *config,
                   char *error,
                   size_t error_size);

void rictus_irc_host_init(rictus_module_host_t *host);

#endif
