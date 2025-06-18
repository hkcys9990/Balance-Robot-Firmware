#include "ble.h"
#define BLE_GPIO GPIOA
#define BLE_RELOAD_PIN LL_GPIO_PIN_5
#define BLE_WAKE_PIN LL_GPIO_PIN_1
#define BLE_RESET_PIN LL_GPIO_PIN_4
#define DMA_RX_LENGTH 20
#define DMA_TX_LENGTH 40
#define FIFO_CHANNEL_MAX 20
void uart2_init(void);
void ble_reload(void);
void ble_wake(void);
void ble_disablereset(void);
void ble_gpio_init(void);

typedef struct{
	bool uart_tx_bus_busy;
	uint8_t uart_tx_buffer[DMA_TX_LENGTH];
	uint8_t uart_rx_buffer_index;
	uint8_t uart_rx_buffer[2*DMA_RX_LENGTH];
	uint8_t uart_rx_buffer_a[DMA_RX_LENGTH];
	uint8_t uart_rx_buffer_b[DMA_RX_LENGTH];
	uint8_t fifo_rx_buffer_index[FIFO_CHANNEL_MAX];//[channel][index]
	uint8_t fifo_rx_isOverflow[FIFO_CHANNEL_MAX];//[channel][is_overflowed]
	
	uint8_t fifo_tx_buffer[FIFO_CHANNEL_MAX][DMA_TX_LENGTH];
	uint8_t fifo_tx_index[FIFO_CHANNEL_MAX];
	
	uint8_t fifo_device_counter;
	
}UART_t;

UART_t ble;
void BLE_Config(void)
{
	uart2_init();
	ble_gpio_init();
	ble_reload();
	ble_wake();
	ble_disablereset();
}
bool BLE_FIFO_Register(uint8_t* id)
{
	if(ble.fifo_device_counter == FIFO_CHANNEL_MAX)
	{
		return false;
	}
	*id = ble.fifo_device_counter;
	ble.fifo_device_counter++;
	return true;
}

bool BLE_FIFO_TX_sendData(uint8_t id, uint8_t* data, uint8_t length)
{
	if(ble.uart_tx_bus_busy == true)
	{
		return false;
	}
	if(length >DMA_TX_LENGTH || ble.fifo_tx_index[id]+length >=DMA_TX_LENGTH)
	{
		return false;
	}
	for (uint8_t i = 0; i<length; i++)
	{
		ble.fifo_tx_buffer[id][ble.fifo_tx_index[id]++] = data[i];
	}
	return true;
}

bool BLE_FIFO_RX_isOverflowed(uint8_t id)
{
	return (ble.fifo_rx_isOverflow[id] == 0 ? false : true);
}
bool BLE_FIFO_RX_isEmpty(uint8_t id)
{
	//return (ble.fifo_rx_buffer_index[id] == ble.uart_rx_buffer_index ? true : false);
	if(ble.fifo_rx_buffer_index[id] == ble.uart_rx_buffer_index )
	{
		return true;
	}else
	{
		return false;
	}
}
uint8_t BLE_FIFO_RX_getData(uint8_t id)
{
	uint8_t data = ble.uart_rx_buffer[ble.fifo_rx_buffer_index[id]];
	ble.fifo_rx_buffer_index[id]++;
	ble.fifo_rx_buffer_index[id]%=DMA_RX_LENGTH*2;
	return data;	
}

void BLE_UART_Mainloop(void) 
{
		if (LL_DMA_IsActiveFlag_TC5(DMA1) || LL_USART_IsActiveFlag_IDLE(USART2))
		{
			if (LL_DMA_IsActiveFlag_TC5(DMA1)) 
			{
				LL_DMA_ClearFlag_TC5(DMA1);
			}
			if(LL_USART_IsActiveFlag_IDLE(USART2))
			{
				LL_USART_ClearFlag_IDLE(USART2);
			}
			LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_5);
			uint8_t data_length = DMA_RX_LENGTH - LL_DMA_GetDataLength(DMA1, LL_DMA_STREAM_5);
			uint8_t* next_buffer_ptr = (LL_DMA_GetMemoryAddress(DMA1, LL_DMA_STREAM_5) == (uint32_t)ble.uart_rx_buffer_a) ? ble.uart_rx_buffer_b : ble.uart_rx_buffer_a;
			uint8_t* current_buffer_ptr = (next_buffer_ptr == ble.uart_rx_buffer_a) ? ble.uart_rx_buffer_b : ble.uart_rx_buffer_a;

			LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_5, DMA_RX_LENGTH);
			LL_DMA_ConfigAddresses(DMA1, LL_DMA_STREAM_5, (uint32_t)&USART2->DR, (uint32_t)next_buffer_ptr, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
			LL_DMA_ClearFlag_TC5(DMA1);
			LL_DMA_ClearFlag_HT5(DMA1);
			LL_DMA_ClearFlag_TE5(DMA1);
			LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_5);
			
			uint16_t remaining_space = DMA_RX_LENGTH * 2 - ble.uart_rx_buffer_index;
			if (data_length <= remaining_space) {
					memcpy(&ble.uart_rx_buffer[ble.uart_rx_buffer_index], current_buffer_ptr, data_length);
			} else {
					memcpy(&ble.uart_rx_buffer[ble.uart_rx_buffer_index], current_buffer_ptr, remaining_space); // ????????
					memcpy(&ble.uart_rx_buffer[0], current_buffer_ptr + remaining_space, data_length - remaining_space); // ????????
			}
			ble.uart_rx_buffer_index += data_length;
			ble.uart_rx_buffer_index %= DMA_RX_LENGTH * 2;
		}					
		
		if(ble.uart_tx_bus_busy == false)
		{
			for(uint8_t i = 0; i<FIFO_CHANNEL_MAX; i++)
			{
				if(ble.fifo_tx_index[i] != 0)
				{
					memcpy(ble.uart_tx_buffer, (uint8_t*)&ble.fifo_tx_buffer[i][0],ble.fifo_tx_index[i]);
					LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_6);					
					LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_6, ble.fifo_tx_index[i]);
					LL_DMA_ConfigAddresses(DMA1, LL_DMA_STREAM_6, (uint32_t)&ble.fifo_tx_buffer[i][0],(uint32_t)&USART2->DR, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
					LL_DMA_ClearFlag_TC6(DMA1);
					LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_6);
					ble.fifo_tx_index[i] = 0;
					ble.uart_tx_bus_busy = true;
					break;
				}
			}
		}
		else 
		{
			if (LL_DMA_IsActiveFlag_TC6(DMA1))
			{
				LL_DMA_ClearFlag_TC6(DMA1);
				ble.uart_tx_bus_busy = false;
			}
		}
}

void ble_reload(void)
{
	BLE_GPIO->BSRR = BLE_RELOAD_PIN;
	HAL_Delay(1);
	BLE_GPIO->BSRR = BLE_RELOAD_PIN<<16;
	HAL_Delay(100);
	BLE_GPIO->BSRR = BLE_RELOAD_PIN;
}
void ble_wake(void)
{
	BLE_GPIO->BSRR = BLE_WAKE_PIN;
}
void ble_disablereset(void)
{
	BLE_GPIO->BSRR = BLE_RESET_PIN;
}
void ble_gpio_init(void)
{
	
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

	
  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_1 | LL_GPIO_PIN_4|LL_GPIO_PIN_5);

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_1|LL_GPIO_PIN_4 | LL_GPIO_PIN_5);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_1|LL_GPIO_PIN_4 | LL_GPIO_PIN_5;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void uart2_init(void)
{
	LL_USART_InitTypeDef USART_InitStruct = {0};
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  /**USART2 GPIO Configuration
  PA2   ------> USART2_TX
  PA3   ------> USART2_RX
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_2|LL_GPIO_PIN_3;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USART2 DMA Init */

  /* USART2_RX Init */
  LL_DMA_SetChannelSelection(DMA1, LL_DMA_STREAM_5, LL_DMA_CHANNEL_4);
  LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_STREAM_5, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
  LL_DMA_SetStreamPriorityLevel(DMA1, LL_DMA_STREAM_5, LL_DMA_PRIORITY_LOW);
  LL_DMA_SetMode(DMA1, LL_DMA_STREAM_5, LL_DMA_MODE_NORMAL);
  LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_STREAM_5, LL_DMA_PERIPH_NOINCREMENT);
  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_STREAM_5, LL_DMA_MEMORY_INCREMENT);
  LL_DMA_SetPeriphSize(DMA1, LL_DMA_STREAM_5, LL_DMA_PDATAALIGN_BYTE);
  LL_DMA_SetMemorySize(DMA1, LL_DMA_STREAM_5, LL_DMA_MDATAALIGN_BYTE);
  LL_DMA_DisableFifoMode(DMA1, LL_DMA_STREAM_5);

  /* USART2_TX Init */
  LL_DMA_SetChannelSelection(DMA1, LL_DMA_STREAM_6, LL_DMA_CHANNEL_4);
  LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_STREAM_6, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
  LL_DMA_SetStreamPriorityLevel(DMA1, LL_DMA_STREAM_6, LL_DMA_PRIORITY_LOW);
  LL_DMA_SetMode(DMA1, LL_DMA_STREAM_6, LL_DMA_MODE_NORMAL);
  LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_STREAM_6, LL_DMA_PERIPH_NOINCREMENT);
  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_STREAM_6, LL_DMA_MEMORY_INCREMENT);
  LL_DMA_SetPeriphSize(DMA1, LL_DMA_STREAM_6, LL_DMA_PDATAALIGN_BYTE);
  LL_DMA_SetMemorySize(DMA1, LL_DMA_STREAM_6, LL_DMA_MDATAALIGN_BYTE);
  LL_DMA_DisableFifoMode(DMA1, LL_DMA_STREAM_6);

  /* USART2 interrupt Init */
  //NVIC_SetPriority(USART2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),1, 0));
  //NVIC_EnableIRQ(USART2_IRQn);
	LL_DMA_ConfigAddresses(DMA1, LL_DMA_STREAM_5, (uint32_t)&USART2->DR, (uint32_t)ble.uart_rx_buffer_a, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
	LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_5, DMA_RX_LENGTH);
	LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_5);

  /* USER CODE BEGIN USART2_Init 1 */
	//LL_USART_EnableIT_RXNE(USART2);
	
  /* USER CODE END USART2_Init 1 */
  USART_InitStruct.BaudRate =57600;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART2, &USART_InitStruct);
  LL_USART_ConfigAsyncMode(USART2);
	LL_USART_EnableDMAReq_RX(USART2);
	LL_USART_EnableDMAReq_TX(USART2);
  LL_USART_Enable(USART2);

}