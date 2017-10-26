#ifndef __MOTORS_H
#define __MOTORS_H

// PWM4 -- PA8 -- CH1
// PWM3 -- PA9 -- CH2
// PWM2 -- PA10 -- CH3
// PWM1 -- PA11 -- CH4
typedef enum MotorNum
{
    MOTOR_FRONT_LEFT = TIM_CHANNEL_3,  // PWM4 -- PA8
    MOTOR_FRONT_RIGHT = TIM_CHANNEL_4, // PWM3 -- PA9
    MOTOR_BACK_LEFT = TIM_CHANNEL_2,   // PWM2 -- PA10
    MOTOR_BACK_RIGHT = TIM_CHANNEL_1,  // PWM1 -- PA11
} MotorNum;

FC_Status motorsStart();
FC_Status motorsStop();
FC_Status motorsDeinit();
FC_Status setMotor(MotorNum motor, uint32_t val);
void motorsInit(void);

void vMotorsTask(void *pvParameters);
#endif /* defined(__MOTORS_H) */
