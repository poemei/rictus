#include <stdio.h>
#include <string.h>

#include "rictus.h"

int main(int argc, char **argv)
{
    if (argc == 1) {
        return rictus_run();
    }

    if (argc == 4 && strcmp(argv[1], "module") == 0) {
        return rictus_module_control(argv[2], argv[3]);
    }

    fprintf(stderr,
            "Usage: %s [module <status|enable|disable> <module-id>]\n",
            argv[0]);
    return 2;
}
