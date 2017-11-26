#ifndef __PID_H
#define __PID_H

typedef struct ControlInfo {
    int dt; // Time between control loop calls in us
    float integratedError;
    int saturated;
    int lastError;
} ControlInfo_t;

typedef struct PID_Gains {
    float K_P;
    float K_I;
    float K_D;
} PID_Gains_t;

typedef struct Limits {
    int min;
    int max;
} Limits_t;

typedef struct PidValues_t {
   int p;
   int i;
   int d;
} PidValues_t;

int controlLoop(int error, ControlInfo_t *info, PID_Gains_t *gains,
                Limits_t* limits, PidValues_t *pid);

#endif

