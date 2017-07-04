#ifndef SCHED_H
#define SCHED_H

#define SCH_MAX_TASKS (20)

/* Limitations: */
/* The ONLY INTERRUPT in the whole system shall be the systick (scheduling) interrupt */
/* ALL tasks have to finish in under the scheduler tick period time (e.g. 1ms usually) */
/* Task overlap should be avoided */

typedef struct
{
    /* Pointer to the task (must be a 'void (void)' function)*/
    void (*p_task)(void);
    
    /* Delay (ticks) until the function will (next) be run*/
    /* - see SCH_Add_Task() for further details*/
    uint32_t delay;
    
    /* Interval (ticks) between subsequent runs.*/
    /* - see SCH_Add_Task() for further details*/
    uint32_t period;
    
    /* Incremented (by scheduler) when task is due to execute*/
    uint32_t runme;
} task_info;

/* Add tasks to scheduler, initial delay and period are in TICKS */
uint32_t sch_add_task(void (*p_function)(), const uint32_t delay, const uint32_t period);

/* Add this to a while(1) loop in main code, tasks are running from here */
void sch_dispatch_tasks(void);

/* Add this to your SysTick_Handler(void) */
void sch_update(void);

#endif /* SCHED_H*/
