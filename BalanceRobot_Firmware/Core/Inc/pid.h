#ifndef __PID_H__
#define __PID_H__

#include "main.h"

typedef struct{
	float f_setpoint;
	float f_feedback;
	float f_err;
	
	float f_kp;
	float f_kp_err;
	
	float f_ki;
	float f_ki_err;
	
	float f_kd;
	float f_kd_err;
	float f_kd_pre_err;
	
	float f_output_limit_up;
	float f_output_limit_low;	
	float f_output;
	float f_output_full_range;
	float f_output_normal;
	float f_output_gain;
}PID_t;

void  PID_Config(PID_t* pid, float kp, float ki, float kd, float control_range, float output_limit_up, float output_limit_low, float output_gain);
float PID_Update(PID_t* pid, float setpoint, float feedback);
float PID_Update_withKd(PID_t* pid, float setpoint, float feedback, float feedback_kd);
float PID_Reset(PID_t* pid);
float PID_SetKd(PID_t* pid, float Kd);
float PID_GetFeedback(PID_t* pid);
float PID_GetSetPoint(PID_t* pid);
float PID_GetOutput(PID_t* pid);
void  PID_ResetAccumulator(PID_t* pid);


#endif