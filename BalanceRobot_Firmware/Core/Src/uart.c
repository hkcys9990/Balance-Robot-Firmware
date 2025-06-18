#include "uart.h"

#define DMA_RX_LENGTH 20
#define DMA_TX_LENGTH 40
#define FIFO_CHANNEL_MAX 20
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

UART_t debug_uart;
void uart1_init(void);

int fputc(int ch, FILE *stream)
{
	while(!LL_USART_IsActiveFlag_TXE(USART1));
	LL_USART_TransmitData8(USART1,ch);
	while(!LL_USART_IsActiveFlag_TXE(USART2));
	LL_USART_TransmitData8(USART2,ch);

	return ch;
}
void DEBUG_UART_Config(void)
{
	uart1_init();
}
bool DEBUG_UART_FIFO_Register(uint8_t* id)
{
	if(debug_uart.fifo_device_counter == FIFO_CHANNEL_MAX)
	{
		return false;
	}
	*id = debug_uart.fifo_device_counter;
	debug_uart.fifo_device_counter++;
	return true;
}

bool DEBUG_UART_FIFO_TX_sendData(uint8_t id, uint8_t* data, uint8_t length)
{
	if(length >DMA_TX_LENGTH || debug_uart.fifo_tx_index[id]+length >=DMA_TX_LENGTH)
	{
		return false;
	}
	for (uint8_t i = 0; i<length; i++)
	{
		debug_uart.fifo_tx_buffer[id][debug_uart.fifo_tx_index[id]++] = data[i];
	}
	return true;
}

bool DEBUG_UART_FIFO_RX_isOverflowed(uint8_t id)
{
	return (debug_uart.fifo_rx_isOverflow[id] == 0 ? false : true);
}
bool DEBUG_UART_FIFO_RX_isEmpty(uint8_t id)
{
	//return (ble.fifo_rx_buffer_index[id] == ble.uart_rx_buffer_index ? true : false);
	if(debug_uart.fifo_rx_buffer_index[id] == debug_uart.uart_rx_buffer_index )
	{
		return true;
	}else
	{
		return false;
	}
}
uint8_t DEBUG_UART_FIFO_RX_getData(uint8_t id)
{
	uint8_t data = debug_uart.uart_rx_buffer[debug_uart.fifo_rx_buffer_index[id]];
	debug_uart.fifo_rx_buffer_index[id]++;
	debug_uart.fifo_rx_buffer_index[id]%=DMA_RX_LENGTH*2;
	return data;	
}

void DEBUG_UART_Mainloop(void) 
{
		if (LL_DMA_IsActiveFlag_TC5(DMA2) || LL_USART_IsActiveFlag_IDLE(USART1))
		{
			if (LL_DMA_IsActiveFlag_TC5(DMA2)) 
			{
				LL_DMA_ClearFlag_TC5(DMA2);
			}
			if(LL_USART_IsActiveFlag_IDLE(USART1))
			{
				LL_USART_ClearFlag_IDLE(USART1);
			}
			LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_5);
			uint8_t data_length = DMA_RX_LENGTH - LL_DMA_GetDataLength(DMA2, LL_DMA_STREAM_5);
			uint8_t* next_buffer_ptr = (LL_DMA_GetMemoryAddress(DMA2, LL_DMA_STREAM_5) == (uint32_t)debug_uart.uart_rx_buffer_a) ? debug_uart.uart_rx_buffer_b : debug_uart.uart_rx_buffer_a;
			uint8_t* current_buffer_ptr = (next_buffer_ptr == debug_uart.uart_rx_buffer_a) ? debug_uart.uart_rx_buffer_b : debug_uart.uart_rx_buffer_a;

			LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_5, DMA_RX_LENGTH);
			LL_DMA_ConfigAddresses(DMA2, LL_DMA_STREAM_5, (uint32_t)&USART1->DR, (uint32_t)next_buffer_ptr, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
			LL_DMA_ClearFlag_TC5(DMA2);
			LL_DMA_ClearFlag_HT5(DMA2);
			LL_DMA_ClearFlag_TE5(DMA2);
			LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_5);
			
			uint16_t remaining_space = DMA_RX_LENGTH * 2 - debug_uart.uart_rx_buffer_index;
			if (data_length <= remaining_space) {
					memcpy(&debug_uart.uart_rx_buffer[debug_uart.uart_rx_buffer_index], current_buffer_ptr, data_length);
			} else {
					memcpy(&debug_uart.uart_rx_buffer[debug_uart.uart_rx_buffer_index], current_buffer_ptr, remaining_space); // ????????
					memcpy(&debug_uart.uart_rx_buffer[0], current_buffer_ptr + remaining_space, data_length - remaining_space); // ????????
			}
			debug_uart.uart_rx_buffer_index += data_length;
			debug_uart.uart_rx_buffer_index %= DMA_RX_LENGTH * 2;
		}					
		
		if(debug_uart.uart_tx_bus_busy == false)
		{
			for(uint8_t i = 0; i<FIFO_CHANNEL_MAX; i++)
			{
				if(debug_uart.fifo_tx_index[i] != 0)
				{
					memcpy(debug_uart.uart_tx_buffer, (uint8_t*)&debug_uart.fifo_tx_buffer[i][0],debug_uart.fifo_tx_index[i]);
					LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_7);					
					LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_7, debug_uart.fifo_tx_index[i]);
					LL_DMA_ConfigAddresses(DMA2, LL_DMA_STREAM_7, (uint32_t)&debug_uart.fifo_tx_buffer[i][0],(uint32_t)&USART1->DR, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
					LL_DMA_ClearFlag_TC7(DMA2);
					LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_7);
					debug_uart.fifo_tx_index[i] = 0;
					debug_uart.uart_tx_bus_busy = true;
					break;
				}
			}
		}
		else 
		{
			if (LL_DMA_IsActiveFlag_TC7(DMA2))
			{
				LL_DMA_ClearFlag_TC7(DMA2);
				debug_uart.uart_tx_bus_busy = false;
			}
		}
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
void uart1_init(void)
{

  LL_USART_InitTypeDef USART_InitStruct = {0};
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  /**USART1 GPIO Configuration
  PA9   ------> USART1_TX
  PA10   ------> USART1_RX
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_9|LL_GPIO_PIN_10;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USART1 interrupt Init */
  //NVIC_SetPriority(USART1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  //NVIC_EnableIRQ(USART1_IRQn);


  /* USART2_RX Init */
  LL_DMA_SetChannelSelection(DMA2, LL_DMA_STREAM_5, LL_DMA_CHANNEL_4);
  LL_DMA_SetDataTransferDirection(DMA2, LL_DMA_STREAM_5, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
  LL_DMA_SetStreamPriorityLevel(DMA2, LL_DMA_STREAM_5, LL_DMA_PRIORITY_LOW);
  LL_DMA_SetMode(DMA2, LL_DMA_STREAM_5, LL_DMA_MODE_NORMAL);
  LL_DMA_SetPeriphIncMode(DMA2, LL_DMA_STREAM_5, LL_DMA_PERIPH_NOINCREMENT);
  LL_DMA_SetMemoryIncMode(DMA2, LL_DMA_STREAM_5, LL_DMA_MEMORY_INCREMENT);
  LL_DMA_SetPeriphSize(DMA2, LL_DMA_STREAM_5, LL_DMA_PDATAALIGN_BYTE);
  LL_DMA_SetMemorySize(DMA2, LL_DMA_STREAM_5, LL_DMA_MDATAALIGN_BYTE);
  LL_DMA_DisableFifoMode(DMA2, LL_DMA_STREAM_5);

  /* USART2_TX Init */
  LL_DMA_SetChannelSelection(DMA2, LL_DMA_STREAM_7, LL_DMA_CHANNEL_4);
  LL_DMA_SetDataTransferDirection(DMA2, LL_DMA_STREAM_7, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
  LL_DMA_SetStreamPriorityLevel(DMA2, LL_DMA_STREAM_7, LL_DMA_PRIORITY_LOW);
  LL_DMA_SetMode(DMA2, LL_DMA_STREAM_7, LL_DMA_MODE_NORMAL);
  LL_DMA_SetPeriphIncMode(DMA2, LL_DMA_STREAM_7, LL_DMA_PERIPH_NOINCREMENT);
  LL_DMA_SetMemoryIncMode(DMA2, LL_DMA_STREAM_7, LL_DMA_MEMORY_INCREMENT);
  LL_DMA_SetPeriphSize(DMA2, LL_DMA_STREAM_7, LL_DMA_PDATAALIGN_BYTE);
  LL_DMA_SetMemorySize(DMA2, LL_DMA_STREAM_7, LL_DMA_MDATAALIGN_BYTE);
  LL_DMA_DisableFifoMode(DMA2, LL_DMA_STREAM_7);

  /* USER CODE BEGIN USART1_Init 1 */
	LL_DMA_ConfigAddresses(DMA2, LL_DMA_STREAM_5, (uint32_t)&USART1->DR, (uint32_t)debug_uart.uart_rx_buffer_a, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
	LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_5, DMA_RX_LENGTH);
	LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_5);

  /* USER CODE END USART1_Init 1 */
  USART_InitStruct.BaudRate = 115200;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART1, &USART_InitStruct);
  LL_USART_ConfigAsyncMode(USART1);
	LL_USART_EnableDMAReq_RX(USART1);
	LL_USART_EnableDMAReq_TX(USART1);
  LL_USART_Enable(USART1);
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}
