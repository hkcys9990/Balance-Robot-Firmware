#include "pid.h"



void PID_Config(PID_t* pid, float kp, float ki,  float kd, float control_range, float output_limit_up, float output_limit_low, float output_gain)
{
	pid->f_kp = kp;
	pid->f_ki = ki;
	pid->f_kd = kd;
	pid->f_output_full_range = control_range;
	pid->f_output_limit_up = output_limit_up;
	pid->f_output_limit_low = output_limit_low;
	pid->f_output_gain = output_gain;
	pid->f_kd_pre_err = 0;
	pid->f_setpoint = 0;
	pid->f_feedback = 0;	
}
void PID_ResetAccumulator(PID_t* pid)
{
	pid->f_ki_err = 0;
}
float PID_GetFeedback(PID_t* pid)
{
	return pid->f_feedback;
}
float PID_GetSetPoint(PID_t* pid)
{
	return pid->f_setpoint;
}
float PID_GetOutput(PID_t* pid)
{
	return pid->f_output_normal * pid->f_output_gain;
}
float PID_Reset(PID_t* pid)
{
	pid->f_setpoint = 0;
	pid->f_feedback = 0;
	pid->f_ki_err = 0;
	pid->f_output = 0;
	pid->f_output_normal = 0;
}
float PID_Update(PID_t* pid, float setpoint, float feedback)
{
	pid->f_feedback = feedback;
	pid->f_setpoint = setpoint;
	pid->f_err = pid->f_setpoint - pid->f_feedback;
	pid->f_kp_err = pid->f_err * pid->f_kp;
  pid->f_ki_err += pid->f_err * pid->f_ki;

	pid->f_kd_err = pid->f_kd * (pid->f_err - pid->f_kd_pre_err);
	pid->f_kd_pre_err = pid->f_err;
	
	pid->f_output = pid->f_kd_err + pid->f_ki_err + pid->f_kp_err;
	float output_clamped = pid->f_output;
	
	if(output_clamped > pid->f_output_limit_up)
	{
		output_clamped = pid->f_output_limit_up;
	}
	else if(output_clamped < pid->f_output_limit_low)
	{
		output_clamped = pid->f_output_limit_low;
	}
	if(output_clamped != pid->f_output)
	{
		pid->f_ki_err  = (output_clamped - pid->f_output)*pid->f_ki;
	}
	pid->f_output = output_clamped;
	
	pid->f_output_normal = pid->f_output/ pid->f_output_full_range;	
	
	return pid->f_output_normal * pid->f_output_gain;

}
float PID_SetKd(PID_t* pid, float Kd)
{
	pid->f_kd_err = Kd;
}
float PID_Update_withKd(PID_t* pid, float setpoint, float feedback, float feedback_kd)
{
	pid->f_feedback = feedback;
	pid->f_setpoint = setpoint;
	pid->f_err = pid->f_setpoint - pid->f_feedback;
	pid->f_kp_err = pid->f_err * pid->f_kp;
  pid->f_ki_err += pid->f_err * pid->f_ki;
	pid->f_kd_err = pid->f_kd * feedback_kd;
	
	pid->f_output = pid->f_kd_err + pid->f_ki_err + pid->f_kp_err;
	float output_clamped = pid->f_output;
	
	if(output_clamped > pid->f_output_limit_up)
	{
		output_clamped = pid->f_output_limit_up;
	}
	else if(output_clamped < pid->f_output_limit_low)
	{
		output_clamped = pid->f_output_limit_low;
	}
	
	if(output_clamped != pid->f_output)
	{
		pid->f_ki_err  += (output_clamped - pid->f_output)*pid->f_ki;
	}
	
	pid->f_output = output_clamped;
	
	pid->f_output_normal = pid->f_output/ pid->f_output_full_range;	
	
	return pid->f_output_normal * pid->f_output_gain;

}