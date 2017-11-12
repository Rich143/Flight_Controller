#ifndef __LOG_H
#define __LOG_H

#include "fc.h"

FC_Status logInit();
FC_Status logFinish();
FC_Status logWriteTest();

void vLogTask(void *pvParameters);

#endif /* defined(__LOG_H) */
