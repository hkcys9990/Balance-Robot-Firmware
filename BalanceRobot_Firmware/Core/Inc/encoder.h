#ifndef __ENCODER_H__
#define __ENCODER_H__

#include "main.h"

void Encoder_Config(void);
void Encoder_Update(void);
float Encoder_GetRightRpm(void);
float Encoder_GetLeftRpm(void);
float Encoder_GetRightAngle(void);
float Encoder_GetLeftAngle(void);

#endif