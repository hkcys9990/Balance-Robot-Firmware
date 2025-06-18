#include "motion.h"
#include "math.h"
#include "datalog.h"

//#define POSITION_LOOP_DEBUG 1
//#define ANGLE_LOOP_DEBUG 1

#ifndef POSITION_LOOP_DEBUG
	#ifndef ANGLE_LOOP_DEBUG

	#define PID_VELOCITY_KP 10
	#define PID_VELOCITY_KI PID_VELOCITY_KP/20
	#define PID_VELOCITY_KD 0

	#define PID_ANGLE_KP 7//7//7
	#define PID_ANGLE_KI 0.03//0.03//0.035//0.0175//0.035
	#define PID_ANGLE_KD 0
	
	#define PID_POSITION_KP 600
	#define PID_POSITION_KI 0
	#define PID_POSITION_KD 0

	#define PID_TURN_KP 10000
	#define PID_TURN_KI PID_TURN_KP/1000
	#define PID_TURN_KD 0

	#else

	#define PID_VELOCITY_KP 0
	#define PID_VELOCITY_KI 0
	#define PID_VELOCITY_KD 0

	#define PID_ANGLE_KP 7
	#define PID_ANGLE_KI 0.01
	#define PID_ANGLE_KD 0
	
	#define PID_POSITION_KP 600
	#define PID_POSITION_KI 10
	#define PID_POSITION_KD 0

	#define PID_TURN_KP 10000
	#define PID_TURN_KI PID_TURN_KP/1000
	#define PID_TURN_KD 0


	#endif
#else
	#define PID_VELOCITY_KP 0
	#define PID_VELOCITY_KI 0
	#define PID_VELOCITY_KD 0

	#define PID_ANGLE_KP 7
	#define PID_ANGLE_KI 0.01
	#define PID_ANGLE_KD 0
	
	#define PID_POSITION_KP 600
	#define PID_POSITION_KI 0
	#define PID_POSITION_KD 0

	#define PID_TURN_KP 10000
	#define PID_TURN_KI PID_TURN_KP/1000
	#define PID_TURN_KD 0


#endif
typedef enum {
	MOTION_BALANCING,
	MOTION_STOP,
	MOTION_STANDBY
}e_MOTION_STATUS;
typedef struct{
	e_MOTION_STATUS e_motion_status;
	uint32_t u32_FallingCounter;
	uint32_t u32_StopStateCounter;
	float f_TargetTurnVelocity;
	float f_TargetVelocityTurnOffset;
	float f_TargetForwardVelocity;
	float f_TargetLeftVelocity;
	float f_TargetRightVelocity;
	float f_TargetLeftAngle;
	float f_TargetRightAngle;
	float f_CtrlLeftAngle;
	float f_CtrlRightAngle;
}MOTION_t;

MOTION_t Motion;
PID_t PID_LeftAngle;
PID_t PID_RightAngle;
PID_t PID_LeftPosition;
PID_t PID_RightPosition;
PID_t PID_LeftVelocity;
PID_t PID_RightVelocity;
PID_t PID_Turn;

void tim9_init(void);

void Motion_Config(void)
{	
	//Datalog Configuration
	uint32_t datalog_items[16];
	datalog_items[0] = (uint32_t)&PID_LeftPosition.f_setpoint;
	datalog_items[1] = (uint32_t)&PID_LeftPosition.f_feedback;
	datalog_items[2] = (uint32_t)&PID_LeftPosition.f_output_normal;
	datalog_items[3] = (uint32_t)&PID_LeftAngle.f_setpoint;
	datalog_items[4] = (uint32_t)&PID_LeftAngle.f_feedback;
	datalog_items[5] = (uint32_t)&PID_LeftAngle.f_output_normal;
	datalog_items[6] = (uint32_t)&PID_LeftVelocity.f_setpoint;
	datalog_items[7] = (uint32_t)&PID_LeftVelocity.f_feedback;
	datalog_items[8] = (uint32_t)&PID_LeftVelocity.f_output_normal;
	Datalog_SetLogData(9,datalog_items);
	
	//PID Configuration
	PID_Config((PID_t*)&PID_Turn,PID_TURN_KP,PID_TURN_KI,PID_TURN_KD, 4095, 4095, -4095,400);
	PID_Config((PID_t*)&PID_LeftVelocity,PID_VELOCITY_KP ,PID_VELOCITY_KI,PID_VELOCITY_KD, 4095, 4095, -4095,45);
	PID_Config((PID_t*)&PID_RightVelocity,PID_VELOCITY_KP ,PID_VELOCITY_KI,PID_VELOCITY_KD, 4095, 4095, -4095,45);
	PID_Config((PID_t*)&PID_LeftAngle,PID_ANGLE_KP ,PID_ANGLE_KI,PID_ANGLE_KD, 4095, 4095, -4095,90);
	PID_Config((PID_t*)&PID_RightAngle,PID_ANGLE_KP,PID_ANGLE_KI,PID_ANGLE_KD, 4095, 4095, -4095,90);
	PID_Config((PID_t*)&PID_LeftPosition,PID_POSITION_KP,PID_POSITION_KI,PID_POSITION_KD, 4095, 4095, -4095, 4095);
	PID_Config((PID_t*)&PID_RightPosition,PID_POSITION_KP,PID_POSITION_KI,PID_POSITION_KD, 4095 , 4095, -4095, 4095);
	Motion.f_TargetLeftAngle = 0;
	Motion.f_TargetRightAngle = 0;
	Motion.f_TargetLeftVelocity = 0;
	Motion.f_TargetRightVelocity = 0;
	Motion.e_motion_status = MOTION_BALANCING;
	//Control Timer Configuration
	tim9_init();
}

void Motion_Mainloop(void)
{
	static uint32_t motion_task_last;
	uint32_t now = HAL_GetTick();
	if(now - motion_task_last > 10) // enter every 10ms 
	{
		motion_task_last = HAL_GetTick();

		switch(Motion.e_motion_status)
		{
			case MOTION_BALANCING:
				#ifndef ANGLE_LOOP_DEBUG
				#ifndef POSITION_LOOP_DEBUG
				if(fabs(DMP_GetRollAngle()) > 45 )
				{
					Motion.u32_FallingCounter++;
				}
				else if(Motion.u32_FallingCounter!=0)
				{
					Motion.u32_FallingCounter--;
				}		
				if(Motion.u32_FallingCounter >= 30)
				{
					Motion.e_motion_status = MOTION_STOP;
				}				
				#endif
				#endif
				break;
			case MOTION_STOP:
				if(Motion.u32_StopStateCounter >= 300)//300*10ms = 3000ms
				{
					Motion.e_motion_status = MOTION_STANDBY;
					Motion.u32_StopStateCounter = 0;
					Motion.u32_FallingCounter = 0;
					Motion.f_TargetTurnVelocity = 0.0f;
					Motion.f_TargetForwardVelocity = 0.0f;
				}else{
					Motion.u32_StopStateCounter++;					
				}
				break;
			case MOTION_STANDBY:
				if(fabs(Motion.f_TargetTurnVelocity) > 0.5f || fabs(Motion.f_TargetForwardVelocity)>0.5f)
				{
						Motion.e_motion_status = MOTION_BALANCING;
				}
				break;
		}
	}
}
void Motion_ModeSetBalance()
{
	Motion.e_motion_status = MOTION_BALANCING;
}
void Motion_ModeSetStop()
{
	Motion.e_motion_status = MOTION_STOP;
}

void Motion_setCommnad(float x, float y)
{
	Motion.f_TargetTurnVelocity = x * 0.125;
	Motion.f_TargetForwardVelocity =  y * 50;
}

void Motion_Handler(void)
{
	switch(Motion.e_motion_status)
	{
		case MOTION_BALANCING:
			#ifndef ANGLE_LOOP_DEBUG

			Motion.f_TargetVelocityTurnOffset = PID_Update(&PID_Turn,Motion.f_TargetTurnVelocity,(DMP_GetTurn()));
			Motion.f_TargetLeftVelocity = Motion.f_TargetForwardVelocity - Motion.f_TargetVelocityTurnOffset;
			Motion.f_TargetRightVelocity = Motion.f_TargetForwardVelocity + Motion.f_TargetVelocityTurnOffset;
			Motion.f_TargetLeftAngle = PID_Update(&PID_LeftVelocity,Motion.f_TargetLeftVelocity,(Encoder_GetLeftRpm()));
			Motion.f_TargetRightAngle = PID_Update(&PID_RightVelocity,Motion.f_TargetRightVelocity,(Encoder_GetRightRpm()));
			Motion.f_CtrlLeftAngle = PID_Update(&PID_LeftAngle,Motion.f_TargetLeftAngle,DMP_GetRollAngle());
			Motion.f_CtrlRightAngle = PID_Update(&PID_RightAngle,Motion.f_TargetRightAngle,DMP_GetRollAngle());
			MotorDrive_Drive( PID_Update(&PID_LeftPosition, Encoder_GetLeftAngle() + Motion.f_CtrlLeftAngle, Encoder_GetLeftAngle()),PID_Update(&PID_RightPosition, Encoder_GetRightAngle() + Motion.f_CtrlRightAngle, Encoder_GetRightAngle()) );		
			#else
			MotorDrive_Drive( PID_Update(&PID_LeftPosition,  Motion.f_CtrlLeftAngle, Encoder_GetLeftAngle()),PID_Update(&PID_RightPosition,  Motion.f_CtrlRightAngle, Encoder_GetRightAngle()) );
			#endif
			
			break;
		case MOTION_STOP:
			PID_Reset(&PID_Turn);
			PID_Reset(&PID_LeftVelocity);
			PID_Reset(&PID_RightVelocity);
			PID_Reset(&PID_LeftAngle);
			PID_Reset(&PID_RightAngle);
			PID_Reset(&PID_LeftPosition);
			PID_Reset(&PID_RightPosition);
			MotorDrive_Drive(0,0);
			break;
		case MOTION_STANDBY:
			break;

	}
}
int32_t t0, t1, t2, t3, t4;
int32_t T1, T2, T3, T4;
void TIM1_BRK_TIM9_IRQHandler(void)
{
	if(LL_TIM_IsActiveFlag_UPDATE(TIM9))
	{
		LL_TIM_ClearFlag_UPDATE(TIM9);
		Encoder_Update();
		DMP_Update();	
		Motion_Handler();
		Datalog_Handler();
	}
}
void tim9_init(void)
{
	  /* USER CODE BEGIN TIM9_Init 0 */

  /* USER CODE END TIM9_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM9);

  /* TIM9 interrupt Init */
  NVIC_SetPriority(TIM1_BRK_TIM9_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(TIM1_BRK_TIM9_IRQn);

  /* USER CODE BEGIN TIM9_Init 1 */

  /* USER CODE END TIM9_Init 1 */
  TIM_InitStruct.Prescaler = 8400 -1 ;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 10 -1;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM9, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM9);
  LL_TIM_SetClockSource(TIM9, LL_TIM_CLOCKSOURCE_INTERNAL);
  /* USER CODE BEGIN TIM9_Init 2 */
	LL_TIM_EnableCounter(TIM9);
	LL_TIM_EnableIT_UPDATE(TIM9);
  /* USER CODE END TIM9_Init 2 */

}