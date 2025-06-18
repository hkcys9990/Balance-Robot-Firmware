#include "bms.h"

#define ADC2BAT_FACTOR 0.003460207 //ADC CNT/ 4095 *3.3 / 10 * 43
#define BAT_LPF_FACTOR_INDEX 0.5
#define BAT_LD_GPIO GPIOB
#define BAT_LD_A_GPIO_PIN LL_GPIO_PIN_0
#define BAT_LD_B_GPIO_PIN LL_GPIO_PIN_1
#define BAT_LD_C_GPIO_PIN LL_GPIO_PIN_2
#define BAT_LD_D_GPIO_PIN LL_GPIO_PIN_10

typedef enum {
	BMS_BAT_INIT,
	BMS_BAT_DISCHARING,
	BMS_BAT_CHARGING
}e_BMS_STATUS;

typedef enum {
	BAT_LV_VERY_HIGH,
  BAT_LV_HIGH,
	BAT_LV_MID,
	BAT_LV_LOW
}e_BMS_BAT_LEVEL;

typedef struct{
	float f_battery_voltage;
	e_BMS_STATUS bms_status;	
	e_BMS_BAT_LEVEL bms_batt_lv;
}BMS_t;

BMS_t BMS;

void adc1_init(void);
void bms_gpio_init(void);

void BMS_Config(void)
{
	bms_gpio_init();
	adc1_init();
	GPIOA->BSRR = LL_GPIO_PIN_6;
}
void BMS_Mainloop(void)
{
	static uint32_t bms_task_last, bms_init_cnt;
	uint32_t now = HAL_GetTick();
	if(now - bms_task_last > 100)
	{
		bms_task_last = HAL_GetTick();
		uint16_t bat = 0;
		for(uint8_t i = 0; i< 4 ;i++)
		{
			LL_ADC_REG_StartConversionSWStart(ADC1);
			while (!LL_ADC_IsActiveFlag_EOCS(ADC1));
			bat += LL_ADC_REG_ReadConversionData12(ADC1);
		}
		bat = bat>>2;
		BMS.f_battery_voltage = (float)(1-BAT_LPF_FACTOR_INDEX) *(float)bat * ADC2BAT_FACTOR + (float)BAT_LPF_FACTOR_INDEX * (float)BMS.f_battery_voltage;
		
		if(BMS.f_battery_voltage>12.0)
		{
			BMS.bms_batt_lv = BAT_LV_VERY_HIGH;
		}
		else if (BMS.f_battery_voltage > 11.1)
		{
			BMS.bms_batt_lv = BAT_LV_HIGH;
		}
		else if (BMS.f_battery_voltage > 10.2)
		{
			BMS.bms_batt_lv = BAT_LV_MID;
		}
		else 
		{
			BMS.bms_batt_lv = BAT_LV_LOW;
		}
		
		switch(BMS.bms_status )
		{
			case BMS_BAT_INIT:
				BAT_LD_GPIO->BSRR = BAT_LD_A_GPIO_PIN;
				BAT_LD_GPIO->BSRR = BAT_LD_B_GPIO_PIN;
				BAT_LD_GPIO->BSRR = BAT_LD_C_GPIO_PIN;
				BAT_LD_GPIO->BSRR = BAT_LD_D_GPIO_PIN;

				bms_init_cnt++;
				if(bms_init_cnt == 50)
				{
					BMS.bms_status = BMS_BAT_DISCHARING;
				}
				break;
			case BMS_BAT_DISCHARING:
				switch(BMS.bms_batt_lv)
				{
					case BAT_LV_VERY_HIGH:
						BAT_LD_GPIO->BSRR = BAT_LD_A_GPIO_PIN <<16;
						BAT_LD_GPIO->BSRR = BAT_LD_B_GPIO_PIN <<16;
						BAT_LD_GPIO->BSRR = BAT_LD_C_GPIO_PIN <<16;
						BAT_LD_GPIO->BSRR = BAT_LD_D_GPIO_PIN <<16;
						break;
					case BAT_LV_HIGH:
						BAT_LD_GPIO->BSRR = BAT_LD_A_GPIO_PIN;
						BAT_LD_GPIO->BSRR = BAT_LD_B_GPIO_PIN <<16;
						BAT_LD_GPIO->BSRR = BAT_LD_C_GPIO_PIN <<16;
						BAT_LD_GPIO->BSRR = BAT_LD_D_GPIO_PIN <<16;

						break;
					case BAT_LV_MID:
						BAT_LD_GPIO->BSRR = BAT_LD_A_GPIO_PIN;
						BAT_LD_GPIO->BSRR = BAT_LD_B_GPIO_PIN;
						BAT_LD_GPIO->BSRR = BAT_LD_C_GPIO_PIN <<16;
						BAT_LD_GPIO->BSRR = BAT_LD_D_GPIO_PIN <<16;

						break;
					case BAT_LV_LOW:
						BAT_LD_GPIO->BSRR = BAT_LD_A_GPIO_PIN;
						BAT_LD_GPIO->BSRR = BAT_LD_B_GPIO_PIN;
						BAT_LD_GPIO->BSRR = BAT_LD_C_GPIO_PIN;
						BAT_LD_GPIO->BSRR = BAT_LD_D_GPIO_PIN <<16;

						break;					
				}
				break;
			case BMS_BAT_CHARGING:
				//missing the charge sense circuit in hardware v1...
				break;
		}
	}
}
void bms_gpio_init(void)
{
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_6);
  LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_0|LL_GPIO_PIN_1|LL_GPIO_PIN_2|LL_GPIO_PIN_10);

	GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = LL_GPIO_PIN_0|LL_GPIO_PIN_1|LL_GPIO_PIN_2|LL_GPIO_PIN_10;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	BAT_LD_GPIO->BSRR = BAT_LD_A_GPIO_PIN;
	BAT_LD_GPIO->BSRR = BAT_LD_B_GPIO_PIN;
	BAT_LD_GPIO->BSRR = BAT_LD_C_GPIO_PIN;
	BAT_LD_GPIO->BSRR = BAT_LD_D_GPIO_PIN;
}

void adc1_init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  LL_ADC_InitTypeDef ADC_InitStruct = {0};
  LL_ADC_REG_InitTypeDef ADC_REG_InitStruct = {0};
  LL_ADC_CommonInitTypeDef ADC_CommonInitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_ADC1);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  /**ADC1 GPIO Configuration
  PA7   ------> ADC1_IN7
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_7;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* ADC1 DMA Init */

  /* ADC1 Init */
	/*
  LL_DMA_SetChannelSelection(DMA2, LL_DMA_STREAM_0, LL_DMA_CHANNEL_0);
  LL_DMA_SetDataTransferDirection(DMA2, LL_DMA_STREAM_0, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
  LL_DMA_SetStreamPriorityLevel(DMA2, LL_DMA_STREAM_0, LL_DMA_PRIORITY_LOW);
  LL_DMA_SetMode(DMA2, LL_DMA_STREAM_0, LL_DMA_MODE_NORMAL);
  LL_DMA_SetPeriphIncMode(DMA2, LL_DMA_STREAM_0, LL_DMA_PERIPH_NOINCREMENT);
  LL_DMA_SetMemoryIncMode(DMA2, LL_DMA_STREAM_0, LL_DMA_MEMORY_INCREMENT);
  LL_DMA_SetPeriphSize(DMA2, LL_DMA_STREAM_0, LL_DMA_PDATAALIGN_HALFWORD);
  LL_DMA_SetMemorySize(DMA2, LL_DMA_STREAM_0, LL_DMA_MDATAALIGN_HALFWORD);
  LL_DMA_DisableFifoMode(DMA2, LL_DMA_STREAM_0);
	*/
  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  ADC_InitStruct.Resolution = LL_ADC_RESOLUTION_12B;
  ADC_InitStruct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
  ADC_InitStruct.SequencersScanMode = LL_ADC_SEQ_SCAN_DISABLE;
  LL_ADC_Init(ADC1, &ADC_InitStruct);
  ADC_REG_InitStruct.TriggerSource = LL_ADC_REG_TRIG_SOFTWARE;
  ADC_REG_InitStruct.SequencerLength = LL_ADC_REG_SEQ_SCAN_DISABLE;
  ADC_REG_InitStruct.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_DISABLE;
  ADC_REG_InitStruct.ContinuousMode = LL_ADC_REG_CONV_SINGLE;
  ADC_REG_InitStruct.DMATransfer = LL_ADC_REG_DMA_TRANSFER_LIMITED;
  LL_ADC_REG_Init(ADC1, &ADC_REG_InitStruct);
  LL_ADC_REG_SetFlagEndOfConversion(ADC1, LL_ADC_REG_FLAG_EOC_UNITARY_CONV);
  ADC_CommonInitStruct.CommonClock = LL_ADC_CLOCK_SYNC_PCLK_DIV4;
  LL_ADC_CommonInit(__LL_ADC_COMMON_INSTANCE(ADC1), &ADC_CommonInitStruct);
  LL_ADC_REG_StartConversionExtTrig(ADC1, LL_ADC_REG_TRIG_EXT_RISING);

  /** Configure Regular Channel
  */
  LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_7);
  LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_7, LL_ADC_SAMPLINGTIME_28CYCLES);
  /* USER CODE BEGIN ADC1_Init 2 */
	LL_ADC_Enable(ADC1);

  /* USER CODE END ADC1_Init 2 */

}
