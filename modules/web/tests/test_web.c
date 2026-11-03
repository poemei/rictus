#include <stdio.h>
#include "web_protocol.h"

int main(void)
{
    unsigned int executed = 0U, passed = 0U;
    int negative_executed = 0, negative_passed = 0;
    int ok = rictus_web_run_selftests(&executed, &passed,
                                     &negative_executed, &negative_passed);
    printf("web protocol: %u/%u passed, negative=%d/%d\n",
           passed, executed, negative_passed, negative_executed);
    return ok ? 0 : 1;
}
