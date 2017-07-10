#ifndef __PID_H
#define __PID_H

typedef struct ControlInfo {
    int maxValue;
    int minValue;

    float K_P;
    float K_I;
    float K_D;

    float integratedError;
    int saturated;
    int dt; // Time between control loop calls in ms

    int lastError;
} ControlInfo_t;

int controlLoop(int error, ControlInfo_t *info);

#endif

