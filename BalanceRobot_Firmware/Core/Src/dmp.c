#include "dmp.h"
#include "math.h"
#include "iir.h"

#define RAD_TO_DEG 57.295779513082320876798154814105
#define DMP_UPDATE_FREQ_HZ 1000
#define ANGLE_OFFSET 0.3615
#define TURN_LPF_RATE_INDEX 0.99
#define ROLL_LPF_INDEX 0.1
typedef struct{
	int16_t RAW_X_ACC;
	int16_t RAW_Y_ACC;
	int16_t RAW_Z_ACC;
	int16_t RAW_TEMP;
	int16_t RAW_X_GYRO;
	int16_t RAW_Y_GYRO;
	int16_t RAW_Z_GYRO;
	float AccRoll;
	float GyroRoll;	
	float Roll;
	float Turn;
}DMP_t;

void i2c_init(void);
void mpu6050_write(uint8_t reg ,uint8_t val);
uint8_t mpu6050_read(uint8_t reg);
void mpu6050_readsensor(int16_t* data);
DMP_t dmp;
IIR_t iir_Roll;

void DMP_Config(void)
{
	i2c_init();
	mpu6050_write(0x6b, 0x00);
	IIR_Config(&iir_Roll,0.098531f, 0.295593f, 0.295593f, 0.098531f,-0.577241f, 0.421787f, -0.056297f);//100hz lpf
}
void DMP_Update(void)
{
	mpu6050_readsensor(&dmp.RAW_X_ACC);
	dmp.AccRoll = atan2((float)dmp.RAW_X_ACC, (float)dmp.RAW_Z_ACC)*RAD_TO_DEG;
	dmp.GyroRoll = (float)dmp.RAW_Y_GYRO/131/(DMP_UPDATE_FREQ_HZ) ; // 131 is the sensitivity
	dmp.Roll = IIR_Process(&iir_Roll,(float)(ROLL_LPF_INDEX * (dmp.GyroRoll+dmp.Roll) + (1-ROLL_LPF_INDEX) * dmp.AccRoll));
	dmp.Turn = (float)dmp.RAW_Z_GYRO/131/(DMP_UPDATE_FREQ_HZ)*(1-TURN_LPF_RATE_INDEX)+  dmp.Turn*TURN_LPF_RATE_INDEX;
}

float DMP_GetRollAngle(void)
{
	return dmp.Roll + ANGLE_OFFSET;
}
float DMP_GetTurn(void)
{
	return dmp.Turn;
}
float DMP_GetGyro(void)
{
	return dmp.GyroRoll;
}
float DMP_Temp(void)
{
	return (float)dmp.RAW_TEMP/340 + 36.53; 
}
void DMP_Mainloop(void)
{
	//printf("%d,%d,%d,%d\r\n",dmp.RAW_TEMP,dmp.RAW_X_GYRO,dmp.RAW_Y_GYRO,dmp.RAW_Z_GYRO);
	//printf("%f\r\n",DMP_GetRollAngle());
}
void mpu6050_write(uint8_t reg ,uint8_t val)
{
	while(LL_I2C_IsActiveFlag_BUSY(I2C1));
	LL_I2C_GenerateStartCondition(I2C1);
	while(!LL_I2C_IsActiveFlag_SB(I2C1));
	LL_I2C_TransmitData8(I2C1,0xd0);
	while(!LL_I2C_IsActiveFlag_ADDR(I2C1));
	LL_I2C_ClearFlag_ADDR(I2C1);
	LL_I2C_TransmitData8(I2C1,reg);
	while(!LL_I2C_IsActiveFlag_TXE(I2C1));
	LL_I2C_TransmitData8(I2C1,val);
	while(!LL_I2C_IsActiveFlag_TXE(I2C1));
	LL_I2C_GenerateStopCondition(I2C1);
}

void mpu6050_readsensor(int16_t* data)
{	
		uint8_t r[14];
		while(LL_I2C_IsActiveFlag_BUSY(I2C1));
		LL_I2C_GenerateStartCondition(I2C1);
		while(!LL_I2C_IsActiveFlag_SB(I2C1));
		LL_I2C_TransmitData8(I2C1,0xd0);
		while(!LL_I2C_IsActiveFlag_ADDR(I2C1));
		LL_I2C_ClearFlag_ADDR(I2C1);
		LL_I2C_TransmitData8(I2C1,0x3b);
		while(!LL_I2C_IsActiveFlag_TXE(I2C1));
		LL_I2C_GenerateStartCondition(I2C1);
		while(!LL_I2C_IsActiveFlag_SB(I2C1));
		LL_I2C_TransmitData8(I2C1,0xd1);
		while(!LL_I2C_IsActiveFlag_ADDR(I2C1));
		LL_I2C_ClearFlag_ADDR(I2C1);

		LL_I2C_AcknowledgeNextData(I2C1,LL_I2C_ACK);
		while(!LL_I2C_IsActiveFlag_RXNE(I2C1));//X acc
		r[0] = LL_I2C_ReceiveData8(I2C1);
		while(!LL_I2C_IsActiveFlag_RXNE(I2C1));
		r[1] = LL_I2C_ReceiveData8(I2C1);

		while(!LL_I2C_IsActiveFlag_RXNE(I2C1));//Y acc
		r[2] = LL_I2C_ReceiveData8(I2C1);
		while(!LL_I2C_IsActiveFlag_RXNE(I2C1));
		r[3] = LL_I2C_ReceiveData8(I2C1);

		while(!LL_I2C_IsActiveFlag_RXNE(I2C1));//Z acc
		r[4] = LL_I2C_ReceiveData8(I2C1);
		while(!LL_I2C_IsActiveFlag_RXNE(I2C1));
		r[5] = LL_I2C_ReceiveData8(I2C1);

		while(!LL_I2C_IsActiveFlag_RXNE(I2C1));//temp
		r[6] = LL_I2C_ReceiveData8(I2C1);
		while(!LL_I2C_IsActiveFlag_RXNE(I2C1));
		r[7] = LL_I2C_ReceiveData8(I2C1);

		while(!LL_I2C_IsActiveFlag_RXNE(I2C1));
		r[8] = LL_I2C_ReceiveData8(I2C1);
		while(!LL_I2C_IsActiveFlag_RXNE(I2C1));
		r[9] = LL_I2C_ReceiveData8(I2C1);

		while(!LL_I2C_IsActiveFlag_RXNE(I2C1));
		r[10] = LL_I2C_ReceiveData8(I2C1);
		while(!LL_I2C_IsActiveFlag_RXNE(I2C1));
		r[11] = LL_I2C_ReceiveData8(I2C1);

		while(!LL_I2C_IsActiveFlag_RXNE(I2C1));
		r[12] = LL_I2C_ReceiveData8(I2C1);
		LL_I2C_AcknowledgeNextData(I2C1,LL_I2C_NACK);
		LL_I2C_GenerateStopCondition(I2C1);
		
		while(!LL_I2C_IsActiveFlag_RXNE(I2C1));
		r[13] = LL_I2C_ReceiveData8(I2C1);
		LL_I2C_GenerateStopCondition(I2C1);
		while(LL_I2C_IsActiveFlag_BUSY(I2C1));
		
		data[0] = -(r[0]<<8|r[1]);
		data[1] = -(r[2]<<8|r[3]);
		data[2] = -(r[4]<<8|r[5]);
		data[3] = (r[6]<<8|r[7]);
		data[4] = (r[8]<<8|r[9]);
		data[5] = (r[10]<<8|r[11]);
		data[6] = (r[12]<<8|r[13]);

}


uint8_t mpu6050_read(uint8_t reg)
{
		while(LL_I2C_IsActiveFlag_BUSY(I2C1));
		LL_I2C_GenerateStartCondition(I2C1);
		while(!LL_I2C_IsActiveFlag_SB(I2C1));
		LL_I2C_TransmitData8(I2C1,0xd0);
		while(!LL_I2C_IsActiveFlag_ADDR(I2C1));
		LL_I2C_ClearFlag_ADDR(I2C1);
		LL_I2C_TransmitData8(I2C1,reg);
		while(!LL_I2C_IsActiveFlag_TXE(I2C1));
		LL_I2C_GenerateStartCondition(I2C1);
		while(!LL_I2C_IsActiveFlag_SB(I2C1));
		LL_I2C_TransmitData8(I2C1,0xd1);
		while(!LL_I2C_IsActiveFlag_ADDR(I2C1));
		LL_I2C_ClearFlag_ADDR(I2C1);
		LL_I2C_AcknowledgeNextData(I2C1,LL_I2C_NACK);
		LL_I2C_GenerateStopCondition(I2C1);
		while(!LL_I2C_IsActiveFlag_RXNE(I2C1));
		uint8_t rx = LL_I2C_ReceiveData8(I2C1);
		while(LL_I2C_IsActiveFlag_BUSY(I2C1));
		return rx;
}


void i2c_init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  LL_I2C_InitTypeDef I2C_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  /**I2C1 GPIO Configuration
  PB8   ------> I2C1_SCL
  PB9   ------> I2C1_SDA
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_8|LL_GPIO_PIN_9;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_4;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);

  /* I2C1 DMA Init */

  /* I2C1_RX Init */
  /*
	LL_DMA_SetChannelSelection(DMA1, LL_DMA_STREAM_0, LL_DMA_CHANNEL_1);
  LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_STREAM_0, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
  LL_DMA_SetStreamPriorityLevel(DMA1, LL_DMA_STREAM_0, LL_DMA_PRIORITY_LOW);
  LL_DMA_SetMode(DMA1, LL_DMA_STREAM_0, LL_DMA_MODE_NORMAL);
  LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_STREAM_0, LL_DMA_PERIPH_NOINCREMENT);
  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_STREAM_0, LL_DMA_MEMORY_INCREMENT);
  LL_DMA_SetPeriphSize(DMA1, LL_DMA_STREAM_0, LL_DMA_PDATAALIGN_BYTE);
  LL_DMA_SetMemorySize(DMA1, LL_DMA_STREAM_0, LL_DMA_MDATAALIGN_BYTE);
  LL_DMA_DisableFifoMode(DMA1, LL_DMA_STREAM_0);
	
  LL_DMA_ConfigAddresses(DMA1, LL_DMA_STREAM_0,(uint32_t)&I2C1->DR, (uint32_t)(r), LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
	*/
  /* I2C1_TX Init */
  /*
	LL_DMA_SetChannelSelection(DMA1, LL_DMA_STREAM_7, LL_DMA_CHANNEL_1);
  LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_STREAM_7, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
  LL_DMA_SetStreamPriorityLevel(DMA1, LL_DMA_STREAM_7, LL_DMA_PRIORITY_LOW);
  LL_DMA_SetMode(DMA1, LL_DMA_STREAM_7, LL_DMA_MODE_NORMAL);
  LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_STREAM_7, LL_DMA_PERIPH_NOINCREMENT);
  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_STREAM_7, LL_DMA_MEMORY_INCREMENT);
  LL_DMA_SetPeriphSize(DMA1, LL_DMA_STREAM_7, LL_DMA_PDATAALIGN_BYTE);
  LL_DMA_SetMemorySize(DMA1, LL_DMA_STREAM_7, LL_DMA_MDATAALIGN_BYTE);
	
	LL_DMA_ConfigAddresses(DMA1, LL_DMA_STREAM_7, (uint32_t)(I2C_TXBuffer), (uint32_t)&I2C1->DR, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);

  LL_DMA_DisableFifoMode(DMA1, LL_DMA_STREAM_7);
	*/
	
  /* USER CODE BEGIN I2C1_Init 1 */
	//LL_DMA_EnableIT_TC(DMA1, LL_DMA_STREAM_7);
	//LL_DMA_EnableIT_TE(DMA1, LL_DMA_STREAM_7);
  //LL_DMA_EnableIT_TC(DMA1, LL_DMA_STREAM_0);
  //LL_DMA_EnableIT_TE(DMA1, LL_DMA_STREAM_0);

  /* USER CODE END I2C1_Init 1 */
  /** I2C Initialization
  */
  LL_I2C_DisableOwnAddress2(I2C1);
  LL_I2C_DisableGeneralCall(I2C1);
  LL_I2C_EnableClockStretching(I2C1);
  I2C_InitStruct.PeripheralMode = LL_I2C_MODE_I2C;
  I2C_InitStruct.ClockSpeed = 400000;
  I2C_InitStruct.DutyCycle = LL_I2C_DUTYCYCLE_2;
  I2C_InitStruct.OwnAddress1 = 0;
  I2C_InitStruct.TypeAcknowledge = LL_I2C_ACK;
  I2C_InitStruct.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
  LL_I2C_Init(I2C1, &I2C_InitStruct);
  LL_I2C_SetOwnAddress2(I2C1, 0);
  /* USER CODE BEGIN I2C1_Init 2 */
	/*
  NVIC_SetPriority(I2C1_EV_IRQn, 0xf);
  NVIC_EnableIRQ(I2C1_EV_IRQn);
  NVIC_SetPriority(I2C1_ER_IRQn,0xf);
  NVIC_EnableIRQ(I2C1_ER_IRQn);
  LL_I2C_EnableIT_EVT(I2C1);
  LL_I2C_EnableIT_ERR(I2C1);
	*/
  LL_I2C_Enable(I2C1);
  /* USER CODE END I2C1_Init 2 */
}