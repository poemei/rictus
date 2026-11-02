#include <stdio.h>
#include "compliance_internal.h"

int main(void)
{
    unsigned executed = 0U, passed = 0U;
    int negative_executed = 0, negative_passed = 0;
    int ok = rictus_compliance_selftest(&executed, &passed,
        &negative_executed, &negative_passed);

    printf("Compliance qualification executed=%u passed=%u negative=%d/%d\n",
        executed, passed, negative_passed, negative_executed);
    puts(ok ? "Compliance parser PASS" : "Compliance parser FAIL");
    return ok ? 0 : 1;
}
