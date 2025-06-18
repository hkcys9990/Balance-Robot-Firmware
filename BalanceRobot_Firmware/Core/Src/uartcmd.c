#include "uartcmd.h"

#define CMD_BUFFER_LENGTH 40
typedef struct{
	uint8_t id;
	uint8_t cmd_buffer[CMD_BUFFER_LENGTH];
	uint8_t cmd_index;
}CMD_t;


uint8_t ascii2hex2(uint8_t a)
{
    if (a >= '0' && a <= '9') {
        return a - '0';
    } else if (a >= 'A' && a <= 'F') {
        return a - 'A' + 10;
    } else if (a >= 'a' && a <= 'f') {
        return a - 'a' + 10;
    }
    return 0; // Handle non-hex characters if necessary
}


CMD_t UartGeneralWR;
void UartGeneralWR_handle(void)
{
	__IO uint8_t cmd[19] = {'@','W','A','A','A','A','A','A','A','A','D','D','D','D','D','D','D','D','#'};
	__IO const uint8_t ascii[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	__IO static uint32_t addr;
	__IO static uint32_t data;
	while(DEBUG_UART_FIFO_RX_isEmpty(UartGeneralWR.id) == false)
	{
		UartGeneralWR.cmd_buffer[UartGeneralWR.cmd_index] = DEBUG_UART_FIFO_RX_getData(UartGeneralWR.id);
		//@WAAAAAAAADDDDDDDD#->@WAAAAAAAADDDDDDDD#
		if(UartGeneralWR.cmd_buffer[UartGeneralWR.cmd_index] == '#')
		{
			if(UartGeneralWR.cmd_buffer[(UartGeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 18)%CMD_BUFFER_LENGTH ] == '@')
			{
				data = (uint32_t)ascii2hex2(UartGeneralWR.cmd_buffer[(UartGeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 1)%CMD_BUFFER_LENGTH ]);
				data |= (uint32_t)ascii2hex2(UartGeneralWR.cmd_buffer[(UartGeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 2)%CMD_BUFFER_LENGTH ])<<4;
				data |= (uint32_t)ascii2hex2(UartGeneralWR.cmd_buffer[(UartGeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 3)%CMD_BUFFER_LENGTH ])<<8;
				data |= (uint32_t)ascii2hex2(UartGeneralWR.cmd_buffer[(UartGeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 4)%CMD_BUFFER_LENGTH ])<<12;
				data |= (uint32_t)ascii2hex2(UartGeneralWR.cmd_buffer[(UartGeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 5)%CMD_BUFFER_LENGTH ])<<16;
				data |= (uint32_t)ascii2hex2(UartGeneralWR.cmd_buffer[(UartGeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 6)%CMD_BUFFER_LENGTH ])<<20;
				data |= (uint32_t)ascii2hex2(UartGeneralWR.cmd_buffer[(UartGeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 7)%CMD_BUFFER_LENGTH ])<<24;
				data |= (uint32_t)ascii2hex2(UartGeneralWR.cmd_buffer[(UartGeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 8)%CMD_BUFFER_LENGTH ])<<28;
				addr = (uint32_t)ascii2hex2(UartGeneralWR.cmd_buffer[(UartGeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 9)%CMD_BUFFER_LENGTH ]);
				addr |= (uint32_t)ascii2hex2(UartGeneralWR.cmd_buffer[(UartGeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 10)%CMD_BUFFER_LENGTH ])<<4;
				addr |= (uint32_t)ascii2hex2(UartGeneralWR.cmd_buffer[(UartGeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 11)%CMD_BUFFER_LENGTH ])<<8;
				addr |= (uint32_t)ascii2hex2(UartGeneralWR.cmd_buffer[(UartGeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 12)%CMD_BUFFER_LENGTH ])<<12;
				addr |= (uint32_t)ascii2hex2(UartGeneralWR.cmd_buffer[(UartGeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 13)%CMD_BUFFER_LENGTH ])<<16;
				addr |= (uint32_t)ascii2hex2(UartGeneralWR.cmd_buffer[(UartGeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 14)%CMD_BUFFER_LENGTH ])<<20;
				addr |= (uint32_t)ascii2hex2(UartGeneralWR.cmd_buffer[(UartGeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 15)%CMD_BUFFER_LENGTH ])<<24;
				addr |= (uint32_t)ascii2hex2(UartGeneralWR.cmd_buffer[(UartGeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 16)%CMD_BUFFER_LENGTH ])<<28;
				*(uint32_t*)addr = data;
				cmd[2] = ascii[(uint8_t)(addr>>28)&0xf];
				cmd[3] = ascii[(uint8_t)(addr>>24)&0xf];
				cmd[4] = ascii[(uint8_t)(addr>>20)&0xf];
				cmd[5] = ascii[(uint8_t)(addr>>16)&0xf];
				cmd[6] = ascii[(uint8_t)(addr>>12)&0xf];
				cmd[7] = ascii[(uint8_t)(addr>>8)&0xf];
				cmd[8] = ascii[(uint8_t)(addr>>4)&0xf];
				cmd[9] = ascii[(uint8_t)(addr)&0xf];
				cmd[10] = ascii[(uint8_t)(data>>28)&0xf];
				cmd[11] = ascii[(uint8_t)(data>>24)&0xf];
				cmd[12] = ascii[(uint8_t)(data>>20)&0xf];
				cmd[13] = ascii[(uint8_t)(data>>16)&0xf];
				cmd[14] = ascii[(uint8_t)(data>>12)&0xf];
				cmd[15] = ascii[(uint8_t)(data>>8)&0xf];
				cmd[16] = ascii[(uint8_t)(data>>4)&0xf];
				cmd[17] = ascii[(uint8_t)(data)&0xf];
				DEBUG_UART_FIFO_TX_sendData(UartGeneralWR.id, cmd, 19);
			}
			//@RAAAAAAAA#->@RAAAAAAAADDDDDDDD#
			else if(UartGeneralWR.cmd_buffer[(UartGeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 10)%CMD_BUFFER_LENGTH ] == '@') 
			{
				addr = (uint32_t)ascii2hex2(UartGeneralWR.cmd_buffer[(UartGeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 1)%CMD_BUFFER_LENGTH ]);
				addr |= (uint32_t)ascii2hex2(UartGeneralWR.cmd_buffer[(UartGeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 2)%CMD_BUFFER_LENGTH ])<<4;
				addr |= (uint32_t)ascii2hex2(UartGeneralWR.cmd_buffer[(UartGeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 3)%CMD_BUFFER_LENGTH ])<<8;
				addr |= (uint32_t)ascii2hex2(UartGeneralWR.cmd_buffer[(UartGeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 4)%CMD_BUFFER_LENGTH ])<<12;
				addr |= (uint32_t)ascii2hex2(UartGeneralWR.cmd_buffer[(UartGeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 5)%CMD_BUFFER_LENGTH ])<<16;
				addr |= (uint32_t)ascii2hex2(UartGeneralWR.cmd_buffer[(UartGeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 6)%CMD_BUFFER_LENGTH ])<<20;
				addr |= (uint32_t)ascii2hex2(UartGeneralWR.cmd_buffer[(UartGeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 7)%CMD_BUFFER_LENGTH ])<<24;
				addr |= (uint32_t)ascii2hex2(UartGeneralWR.cmd_buffer[(UartGeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 8)%CMD_BUFFER_LENGTH ])<<28;
				data =  *(uint32_t*)addr; 
				cmd[1] = 'R';
				cmd[2] = ascii[(uint8_t)(addr>>28)&0xf];
				cmd[3] = ascii[(uint8_t)(addr>>24)&0xf];
				cmd[4] = ascii[(uint8_t)(addr>>20)&0xf];
				cmd[5] = ascii[(uint8_t)(addr>>16)&0xf];
				cmd[6] = ascii[(uint8_t)(addr>>12)&0xf];
				cmd[7] = ascii[(uint8_t)(addr>>8)&0xf];
				cmd[8] = ascii[(uint8_t)(addr>>4)&0xf];
				cmd[9] = ascii[(uint8_t)(addr)&0xf];
				cmd[10] = ascii[(uint8_t)(data>>28)&0xf];
				cmd[11] = ascii[(uint8_t)(data>>24)&0xf];
				cmd[12] = ascii[(uint8_t)(data>>20)&0xf];
				cmd[13] = ascii[(uint8_t)(data>>16)&0xf];
				cmd[14] = ascii[(uint8_t)(data>>12)&0xf];
				cmd[15] = ascii[(uint8_t)(data>>8)&0xf];
				cmd[16] = ascii[(uint8_t)(data>>4)&0xf];
				cmd[17] = ascii[(uint8_t)(data)&0xf];
				DEBUG_UART_FIFO_TX_sendData(UartGeneralWR.id, cmd, 19);
			}
		}
		UartGeneralWR.cmd_index++;
		UartGeneralWR.cmd_index%= CMD_BUFFER_LENGTH;
	}
}
void UARTCMD_Config(void)
{
	if(DEBUG_UART_FIFO_Register(&UartGeneralWR.id) == false)
	{
		while(1);
	}
}
void UARTCMD_Mainloop(void)
{
	UartGeneralWR_handle();
}
