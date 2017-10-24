#ifndef __RC_H
#define __RC_H

typedef enum tRCChannel {
    ROLL_CHANNEL         = 0,
    PITCH_CHANNEL        = 1,
    THROTTLE_CHANNEL     = 2,
    YAW_CHANNEL          = 3,
    FLIGHT_MODE_CHANNEL  = 4,
    ARMED_SWITCH_CHANNEL = 5,
    UNUSED_CHANNEL_1     = 6,
    UNUSED_CHANNEL_2     = 7,
} tRCChannel;

#define SWITCH_HIGH_THRESHOLD 1700
#define SWITCH_LOW_THRESHOLD 1300

#define MOTOR_LOW_VAL_US   1000
#define MOTOR_HIGH_VAL_US  2000

#define THROTTLE_LOW_THRESHOLD 1100 // TODO: test to find val where motors start spinning

#define ESC_STARTUP_TIME 7000 // ESCs expect 1000us pwm during their startup

#define RC_CHANNEL_IN_COUNT    8


#endif /*defined(__RC_H)*/
