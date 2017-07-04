// Waterloo Hybrid 2016

#ifndef DEBUG_H
#define DEBUG_H

#ifdef DEBUG_MODE
#define formulaDBG(...) printf(__VA_ARGS__)
#else
#define formulaDBG(...) (void)0
#endif

void debug_init(void);
#endif
