#include "motordrive.h"
void motor_gpio_init(void);
void tim2_init(void);
void motor1_ShortBrake(void);
void motor1_CCW(void);
void motor1_CW(void);
void motor12_DisableStandBy(void);
void motor12_EnableStandBy(void);
void motor2_ShortBrake(void);
void motor2_CCW(void);
void motor2_CW(void);

void MotorDrive_Config(void)
{
	motor_gpio_init();
	tim2_init();
	motor12_DisableStandBy();
}
void MotorDrive_Drive(float Lduty, float Rduty)
{
	if(	Lduty>=0)
	{
		motor1_CW();
	}else
	{
		Lduty = -Lduty;
		motor1_CCW();
	}
	if(	Rduty>=0)
	{
		motor2_CW();
	}else
	{
		Rduty = -Rduty;
		motor2_CCW();
	}
	TIM2->CCR1 = (uint16_t)((float)Lduty );
	TIM2->CCR2 = (uint16_t)((float)Rduty );
}

void motor_gpio_init(void)
{
	
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

	
  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_8 | LL_GPIO_PIN_11|LL_GPIO_PIN_12);
  LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_14|LL_GPIO_PIN_15);

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_11|LL_GPIO_PIN_12 | LL_GPIO_PIN_8);
  LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_14|LL_GPIO_PIN_15);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_11|LL_GPIO_PIN_12 | LL_GPIO_PIN_8;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_14|LL_GPIO_PIN_15;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

void tim2_init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};
  LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);

  /* USER CODE BEGIN TIM2_Init 1 */
  //NVIC_SetPriority(TIM2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),1, 0));
  //NVIC_EnableIRQ(TIM2_IRQn);

  /* USER CODE END TIM2_Init 1 */
	
  TIM_InitStruct.Prescaler = 0;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 0xfff;//20.5khz
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
	
	/*
	TIM_InitStruct.Prescaler = 8400-1;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 10000-1;//20.5khz
	TIM_InitStruct.RepetitionCounter = 100;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV4;
	*/
  LL_TIM_Init(TIM2, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM2);
  LL_TIM_OC_EnablePreload(TIM2, LL_TIM_CHANNEL_CH1);
  TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
  TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.CompareValue = 0;
  TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
  LL_TIM_OC_Init(TIM2, LL_TIM_CHANNEL_CH1, &TIM_OC_InitStruct);
  LL_TIM_OC_DisableFast(TIM2, LL_TIM_CHANNEL_CH1);
  LL_TIM_OC_EnablePreload(TIM2, LL_TIM_CHANNEL_CH2);
  TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
  LL_TIM_OC_Init(TIM2, LL_TIM_CHANNEL_CH2, &TIM_OC_InitStruct);
  LL_TIM_OC_DisableFast(TIM2, LL_TIM_CHANNEL_CH2);
  LL_TIM_SetTriggerOutput(TIM2, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM2);
  /* USER CODE BEGIN TIM2_Init 2 */
	LL_TIM_EnableAutomaticOutput(TIM2);
	LL_TIM_CC_EnableChannel(TIM2,LL_TIM_CHANNEL_CH2);
	LL_TIM_CC_EnableChannel(TIM2,LL_TIM_CHANNEL_CH1);
	
	
  /* USER CODE END TIM2_Init 2 */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  /**TIM2 GPIO Configuration
  PA15   ------> TIM2_CH1
  PB3   ------> TIM2_CH2
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_15;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_3;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	LL_TIM_ClearFlag_UPDATE(TIM2);
	//LL_TIM_EnableIT_UPDATE(TIM2);
	LL_TIM_EnableCounter(TIM2);

	
}

void motor1_ShortBrake(void)
{
	MOTOR1_GPIO->BSRR = MOTOR1_AIN1_PIN;
	MOTOR1_GPIO->BSRR = MOTOR1_AIN2_PIN;
}
void motor1_CCW(void)
{
	MOTOR1_GPIO->BSRR = MOTOR1_AIN1_PIN;
	MOTOR1_GPIO->BSRR = MOTOR1_AIN2_PIN<<16;	
}

void motor1_CW(void)
{
	MOTOR1_GPIO->BSRR = MOTOR1_AIN1_PIN<<16;
	MOTOR1_GPIO->BSRR = MOTOR1_AIN2_PIN;	
}

void motor12_DisableStandBy(void)
{
	MOTOR_STANDBY_GPIO->BSRR = MOTOR_STANDBY_PIN;
}
void motor12_EnableStandBy(void)
{
	MOTOR_STANDBY_GPIO->BSRR = MOTOR_STANDBY_PIN<<16;
}

void motor2_ShortBrake(void)
{
	MOTOR2_GPIO->BSRR = MOTOR2_AIN1_PIN;
	MOTOR2_GPIO->BSRR = MOTOR2_AIN2_PIN;
	
	
}
void motor2_CCW(void)
{
	MOTOR2_GPIO->BSRR = MOTOR2_AIN1_PIN<<16;
  MOTOR2_GPIO->BSRR = MOTOR2_AIN2_PIN;	

}
void motor2_CW(void)
{
	MOTOR2_GPIO->BSRR = MOTOR2_AIN1_PIN;
	MOTOR2_GPIO->BSRR = MOTOR2_AIN2_PIN<<16;		
}