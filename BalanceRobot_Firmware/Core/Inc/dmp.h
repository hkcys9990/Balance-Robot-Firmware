#ifndef __DMP_H__
#define __DMP_H__

#include "main.h"


void DMP_Config(void);
void DMP_Update(void);
float DMP_GetRollAngle(void);
float DMP_GetGyro(void);
float DMP_GetTurn(void);
void DMP_Mainloop(void);

#endif