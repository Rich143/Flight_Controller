#ifndef __PID_H
#define __PID_H

typedef struct ControlInfo {
    float integratedError;
    int saturated;
    int dt; // Time between control loop calls in ms
    int lastError;
} ControlInfo_t;

typedef struct PID_Gains {
    float K_P;
    float K_I;
    float K_D;
} PID_Gains_t;

typedef struct Limits {
    int max;
    int min;
} Limits_t;

int controlLoop(int error, ControlInfo_t *info, PID_Gains_t *gains,
                Limits_t* limits);

#endif

