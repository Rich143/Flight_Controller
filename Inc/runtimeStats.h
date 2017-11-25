#ifndef __RUNTIME_STATS_H
#define __RUNTIME_STATS_H

#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() statsTimerInit()
#define portGET_RUN_TIME_COUNTER_VALUE() runTimeStatsGetTimerVal()

void statsTimerInit();
uint32_t runTimeStatsGetTimerVal();
#endif /* defined(__RUNTIME_STATS_H) */
