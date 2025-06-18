#ifndef __MOTORDRIVE_H__
#define __MOTORDRIVE_H__

#include "main.h"


#define MOTOR1_GPIO GPIOB
#define MOTOR1_AIN1_PIN LL_GPIO_PIN_14
#define MOTOR1_AIN2_PIN LL_GPIO_PIN_15
#define MOTOR2_GPIO GPIOA
#define MOTOR2_AIN1_PIN LL_GPIO_PIN_11
#define MOTOR2_AIN2_PIN LL_GPIO_PIN_12

#define MOTOR_STANDBY_GPIO GPIOA
#define MOTOR_STANDBY_PIN LL_GPIO_PIN_8

void MotorDrive_Config(void);
void MotorDrive_Drive(float Lduty, float Rduty);


#endif