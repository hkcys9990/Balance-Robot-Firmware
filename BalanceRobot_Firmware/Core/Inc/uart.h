#ifndef __UART_H__
#define __UART_H__

#include "main.h"
#include "stdio.h"
void DEBUG_UART_Config(void);
bool DEBUG_UART_FIFO_Register(uint8_t* id);
bool DEBUG_UART_FIFO_TX_sendData(uint8_t id, uint8_t* data, uint8_t length);
bool DEBUG_UART_FIFO_RX_isOverflowed(uint8_t id);
bool DEBUG_UART_FIFO_RX_isEmpty(uint8_t id);
uint8_t DEBUG_UART_FIFO_RX_getData(uint8_t id);
void DEBUG_UART_Mainloop(void);
#endif