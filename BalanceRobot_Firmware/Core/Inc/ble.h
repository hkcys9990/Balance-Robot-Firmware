#ifndef __BLE_H__
#define __BLE_H__

#include "main.h"
#include <string.h> 
#include "motion.h";

void BLE_Config(void)	;
void BLE_UART_Handle(void);

bool BLE_FIFO_Register(uint8_t* id);
bool BLE_FIFO_RX_isOverflowed(uint8_t id);
bool BLE_FIFO_RX_isEmpty(uint8_t id);
uint8_t BLE_FIFO_RX_getData(uint8_t id);
bool BLE_FIFO_TX_sendData(uint8_t id, uint8_t* data, uint8_t length);
void BLE_UART_Mainloop(void);


#endif