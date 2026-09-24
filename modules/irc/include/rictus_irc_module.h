#ifndef RICTUS_IRC_MODULE_H
#define RICTUS_IRC_MODULE_H

#include "rictus_module.h"

/*
 * First real Rictus module.
 *
 * This establishes IRC as a module capability without yet changing the
 * proven runtime path. Core remains the authority for qualification and
 * activation.
 */
const rictus_module_descriptor_t *rictus_irc_module_descriptor(void);

#endif
