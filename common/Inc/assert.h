#ifndef ASSERT_H
#define ASSERT_H

// Static assert
#define STATIC_ASSERT(COND,MSG) typedef char static_assertion_##MSG[(!!(COND))*2-1]

// Runtime assert
#define c_assert(e) ((e) ? (0) : log_assert_violation(__FILE__,__LINE__,#e))
int32_t log_assert_violation(char *file, uint32_t line, char *condition);

#endif // ASSERT_H
