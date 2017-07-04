#include "stdint.h"
#include "stdio.h"

int32_t log_assert_violation(char *file, uint32_t line, char *condition)
{
    printf("ASSERT FAILURE: (%s): %s:%lu\n", condition, file, line);
    return 1;
}
