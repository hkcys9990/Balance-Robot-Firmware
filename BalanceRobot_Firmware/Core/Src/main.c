#include "main.h"
#include "motion.h"
#include "ble.h"
#include "blecmd.h"
#include "stdio.h"
#include "datalog.h"
#include "uart.h"
#include "sysclk.h"
#include "debug.h"
#include "gpio.h"
#include "adc.h"
#include "bms.h"
#include "iir.h"
int main(void)
{
  HAL_Init();
  SystemClock_Config();
	HAL_Delay(1000);
  MX_GPIO_Init();
  Debug_Config();
	Datalog_Config();
	BLE_Config();
	BLECMD_Config();
	DMP_Config();
	Encoder_Config();
	MotorDrive_Config();
	BMS_Config();
	Motion_Config();
  while (1)
  {
		BLE_UART_Mainloop();		
		BLECMD_Mainloop();
		Datalog_MainLoop();
		Motion_Mainloop();
		DMP_Mainloop();
		BMS_Mainloop();
  }
}


