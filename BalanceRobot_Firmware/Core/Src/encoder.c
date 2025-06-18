#include "encoder.h"
#include "iir.h"

#define GM25_370_ENC_CNT_PRE_REV 748
#define ENC_UPDATE_FREQ_HZ 1000 


typedef struct{
	
	uint16_t u16_count_pre_rev;
	float f_encoder_sum_freq;
	int32_t i32_left_count_accumulation;
	int32_t i32_right_count_accumulation;
	float f_left_rpm;
	float f_left_angle;
	float f_right_rpm;
	float f_right_angle;
	
}MotorEncoder_t;

MotorEncoder_t MotorEncoder;
void tim3_init(void);
void tim4_init(void);
IIR_t iir_Rpm_L;
IIR_t iir_Rpm_R;

void Encoder_Config(void)
{
	tim3_init();
	tim4_init();
	MotorEncoder.u16_count_pre_rev = GM25_370_ENC_CNT_PRE_REV;
	MotorEncoder.f_encoder_sum_freq = ENC_UPDATE_FREQ_HZ;// / ENC_UPDATE_DOWNSCALE;
	MotorEncoder.f_left_rpm = 0;
	MotorEncoder.f_right_rpm = 0;
	IIR_Config(&iir_Rpm_L, 0.0002196f, 0.0006588f , 0.0006588f, 0.0002196f, -2.7488f, 2.5282f, -0.7776f);//20hz lpf
	IIR_Config(&iir_Rpm_R, 0.0002196f, 0.0006588f , 0.0006588f, 0.0002196f, -2.7488f, 2.5282f, -0.7776f);//20hz lpf

}
void Encoder_Update(void)
{
	int32_t r = TIM3->CNT - 30000;
	int32_t l = -(TIM4->CNT - 30000);
	TIM3->CNT  = 30000;
	TIM4->CNT  = 30000;	
		
	MotorEncoder.i32_left_count_accumulation +=l;
	MotorEncoder.i32_right_count_accumulation +=r;
	MotorEncoder.f_left_angle = (float)MotorEncoder.i32_left_count_accumulation / MotorEncoder.u16_count_pre_rev *360;
	MotorEncoder.f_right_angle = (float)MotorEncoder.i32_right_count_accumulation / MotorEncoder.u16_count_pre_rev *360;	
	MotorEncoder.f_left_rpm = IIR_Process(&iir_Rpm_L, (float)l / (float)MotorEncoder.u16_count_pre_rev *(float)MotorEncoder.f_encoder_sum_freq);
	MotorEncoder.f_right_rpm = IIR_Process(&iir_Rpm_L,(float)r / (float)MotorEncoder.u16_count_pre_rev *(float)MotorEncoder.f_encoder_sum_freq);

}
float Encoder_GetRightRpm(void)
{
	return MotorEncoder.f_right_rpm;
}
float Encoder_GetLeftRpm(void)
{
	return MotorEncoder.f_left_rpm;
}
float Encoder_GetRightAngle(void)
{
	return MotorEncoder.f_right_angle;
}
float Encoder_GetLeftAngle(void)
{
	return MotorEncoder.f_left_angle;
}

void tim3_init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  /**TIM3 GPIO Configuration
  PB4   ------> TIM3_CH1
  PB5   ------> TIM3_CH2
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_4|LL_GPIO_PIN_5;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  LL_TIM_SetEncoderMode(TIM3, LL_TIM_ENCODERMODE_X4_TI12);
  LL_TIM_IC_SetActiveInput(TIM3, LL_TIM_CHANNEL_CH1, LL_TIM_ACTIVEINPUT_DIRECTTI);
  LL_TIM_IC_SetPrescaler(TIM3, LL_TIM_CHANNEL_CH1, LL_TIM_ICPSC_DIV1);
  LL_TIM_IC_SetFilter(TIM3, LL_TIM_CHANNEL_CH1, LL_TIM_IC_FILTER_FDIV1);
  LL_TIM_IC_SetPolarity(TIM3, LL_TIM_CHANNEL_CH1, LL_TIM_IC_POLARITY_RISING);
  LL_TIM_IC_SetActiveInput(TIM3, LL_TIM_CHANNEL_CH2, LL_TIM_ACTIVEINPUT_DIRECTTI);
  LL_TIM_IC_SetPrescaler(TIM3, LL_TIM_CHANNEL_CH2, LL_TIM_ICPSC_DIV1);
  LL_TIM_IC_SetFilter(TIM3, LL_TIM_CHANNEL_CH2, LL_TIM_IC_FILTER_FDIV1);
  LL_TIM_IC_SetPolarity(TIM3, LL_TIM_CHANNEL_CH2, LL_TIM_IC_POLARITY_RISING);
  TIM_InitStruct.Prescaler = 1;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 65535;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM3, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM3);
  LL_TIM_SetTriggerOutput(TIM3, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM3);
  /* USER CODE BEGIN TIM3_Init 2 */
	TIM3->CNT = 30000;
	LL_TIM_EnableCounter(TIM3);
  /* USER CODE END TIM3_Init 2 */

}
void tim4_init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM4);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  /**TIM4 GPIO Configuration
  PB6   ------> TIM4_CH1
  PB7   ------> TIM4_CH2
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_6|LL_GPIO_PIN_7;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  LL_TIM_SetEncoderMode(TIM4, LL_TIM_ENCODERMODE_X4_TI12);
  LL_TIM_IC_SetActiveInput(TIM4, LL_TIM_CHANNEL_CH1, LL_TIM_ACTIVEINPUT_DIRECTTI);
  LL_TIM_IC_SetPrescaler(TIM4, LL_TIM_CHANNEL_CH1, LL_TIM_ICPSC_DIV1);
  LL_TIM_IC_SetFilter(TIM4, LL_TIM_CHANNEL_CH1, LL_TIM_IC_FILTER_FDIV1);
  LL_TIM_IC_SetPolarity(TIM4, LL_TIM_CHANNEL_CH1, LL_TIM_IC_POLARITY_RISING);
  LL_TIM_IC_SetActiveInput(TIM4, LL_TIM_CHANNEL_CH2, LL_TIM_ACTIVEINPUT_DIRECTTI);
  LL_TIM_IC_SetPrescaler(TIM4, LL_TIM_CHANNEL_CH2, LL_TIM_ICPSC_DIV1);
  LL_TIM_IC_SetFilter(TIM4, LL_TIM_CHANNEL_CH2, LL_TIM_IC_FILTER_FDIV1);
  LL_TIM_IC_SetPolarity(TIM4, LL_TIM_CHANNEL_CH2, LL_TIM_IC_POLARITY_RISING);
  TIM_InitStruct.Prescaler = 1;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 65535;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM4, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM4);
  LL_TIM_SetTriggerOutput(TIM4, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM4);
  /* USER CODE BEGIN TIM4_Init 2 */
	TIM4->CNT = 30000;
	LL_TIM_EnableCounter(TIM4);
	
  /* USER CODE END TIM4_Init 2 */

}
