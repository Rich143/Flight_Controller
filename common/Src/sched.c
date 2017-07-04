#include "stdint.h"
#include "sched.h"
#include "assert.h"

static volatile uint32_t sched_cycles = 0;          /* Scheduler cycle count */
static volatile task_info sch_tasks[SCH_MAX_TASKS]; /* Task list, modified in ISR */
static volatile int32_t task_overrun = 0;           /* Task overrun flag, set in ISR */
static int32_t dispatch_active = 0;                 /* Flag set by dispatch functions, used to detect overruns in ISR */

/* Schedule task for execution */
uint32_t sch_add_task(void (*p_function)(), const uint32_t delay, const uint32_t period)
{
    uint32_t index = 0;

    /* Check parameters */
    if( c_assert(p_function) ||
        c_assert(period > 0))
    {
        return 1;
    }

    /* First find a gap in the array (if there is one)*/
    while ((sch_tasks[index].p_task != 0) && (index < SCH_MAX_TASKS))
    {
        index++;
    }

    /* Have we reached the end of the list?*/
    if (index == SCH_MAX_TASKS)
    {
        return 1;
    }

    /* If we're here, there is a space in the task array*/
    sch_tasks[index].p_task = p_function;
    sch_tasks[index].delay = delay;
    sch_tasks[index].period = period - 1; // -1 to account for the fact that 0 is actually every 1 timer tick
    sch_tasks[index].runme = 0;

    return 0;
}

/* Timer ISR (one ISR in whole application) invoked every scheduler tick */
void sch_update(void) 
{
    uint32_t index = 0;

    /* Check if task is overrun */
    if(dispatch_active)
    {
        task_overrun = 1;
    }

    sched_cycles++;

    /* NOTE: calculations are in *TICKS* (not milliseconds) */
    for (index = 0; index < SCH_MAX_TASKS; index++)
    {
        /* Check if there is a task at this location */
        if (sch_tasks[index].p_task)
        {
            if (sch_tasks[index].delay == 0)
            {
                /* The task is due to run */
                sch_tasks[index].runme += 1; /* Inc. the 'runme' flag */
                if (sch_tasks[index].period)
                {
                    /* Schedule periodic tasks to run again */
                    sch_tasks[index].delay = sch_tasks[index].period;
                }
            }
            else
            {
                /* Not yet ready to run: just decrement the delay */
                sch_tasks[index].delay -= 1;
            }
        }
    }
}

/* Scheduler update function, this is what calls the actual tasks */
void sch_dispatch_tasks(void)
{
    uint32_t index;
    
    /* Dispatches (runs) the next task (if one is ready) */
    for (index = 0; index < SCH_MAX_TASKS; index++)
    {
        if (sch_tasks[index].runme > 0)
        {
            dispatch_active = 1;
            (*sch_tasks[index].p_task)(); /* Run the task */
            sch_tasks[index].runme -= 1; /* Reset / reduce RunMe flag */
            dispatch_active = 0;
        }
    }

    /* Log and reset task overrun flag */
    if(c_assert(!task_overrun))
    {
        task_overrun = 0;
    }
}










