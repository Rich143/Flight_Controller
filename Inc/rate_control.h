#ifndef __RATE_CONTROL_H
#define __RATE_CONTROL_H

#define ROTATION_AXIS_OUTPUT_MAX 1024
#define ROTATION_AXIS_OUTPUT_MIN -1024

#define RATES_MAX 500 //! Max rotation rate in dps
#define RATES_MIN -500 //! Min rotation rate in dps

/**
 * @brief Rotation rates for roll pitch and yaw
 */
typedef struct Rates {
    int roll;
    int pitch;
    int yaw;
} Rates_t;

/**
 * @brief Intermediate representation of desired motor outputs.
 * Each int represents the desired power output for that roll axis. These
 * values range from ROTATION_AXIS_OUTPUT_MAX to ROTATION_AXIS_OUTPUT_MIN
 */
typedef struct RotationAxisOutputs {
    int roll;
    int pitch;
    int yaw;
} RotationAxisOutputs_t;

RotationAxisOutputs_t* controlRates(Rates_t* actualRates, Rates_t* desiredRates);
void resetRateInfo();
#endif
