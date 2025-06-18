#ifndef __APP_H__
#define __APP_H__

#include "main.h"
#include "encoder.h"
#include "motordrive.h"
#include "dmp.h"
#include "pid.h"


void Motion_Config(void);
void Motion_setSpeedRpm(float rpm);
void Motion_setCommnad(float x, float y);
void Motion_Mainloop(void);
void Motion_ModeSetBalance();
void Motion_ModeSetStop();

#endif