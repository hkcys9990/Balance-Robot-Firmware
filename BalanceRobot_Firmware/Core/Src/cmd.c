#include "cmd.h"
#include "ble.h"


#define CMD_BUFFER_LENGTH 40
typedef struct{
	uint8_t id;
	uint8_t cmd_buffer[CMD_BUFFER_LENGTH];
	uint8_t cmd_index;
}CMD_t;

CMD_t RemoteControl;
void RemoteControl_handle(void)
{
	 while(BLE_FIFO_RX_isEmpty(RemoteControl.id) == false)
	 {
			RemoteControl.cmd_buffer[RemoteControl.cmd_index] = BLE_FIFO_RX_getData(RemoteControl.id);
		 
			if(RemoteControl.cmd_buffer[RemoteControl.cmd_index] == 0x03)
			{
				static uint8_t cmd[20];
				static uint8_t cmd_length;
				static uint8_t cmd_comma_index ;
				cmd_length = 0;
				cmd_comma_index = 0;
				uint32_t cmd_x = 0, cmd_y = 0;
				while(cmd_length < 20)
				{
					cmd_length++;
					if(RemoteControl.cmd_buffer[(RemoteControl.cmd_index + CMD_BUFFER_LENGTH - cmd_length)% (CMD_BUFFER_LENGTH)] == 0x01) 
					{
						for(uint8_t i = 0; i<=cmd_length; i++)
						{
							cmd[i] = RemoteControl.cmd_buffer[(RemoteControl.cmd_index + CMD_BUFFER_LENGTH - cmd_length + i)% (CMD_BUFFER_LENGTH)];
							if(cmd[i] == ',')
							{
								cmd_comma_index = i;
							}
							switch(cmd_comma_index - 4)
							{
								case 1:
									cmd_x = cmd[4] - 48;
									break;
								case 2:
									cmd_x = (cmd[4] - 48)*10 + (cmd[5]-48) ;
									break;
								case 3:
									cmd_x = (cmd[4] - 48)*100 + (cmd[5]-48)*10 + (cmd[6]-48);
									break;
								case 4:
									cmd_x = (cmd[4] - 48)*1000 +(cmd[5] - 48)*100 + (cmd[6] - 48)*10 + (cmd[7] -48);
									break;
							}
							switch(cmd_length - cmd_comma_index - 1)
							{
								case 1:
									cmd_y = cmd[cmd_comma_index+1] - 48;
									break;
								case 2:
									cmd_y = (cmd[cmd_comma_index+1] - 48)*10 + (cmd[cmd_comma_index+2]-48) ;
									break;
								case 3:
									cmd_y = (cmd[cmd_comma_index+1] - 48)*100 + (cmd[cmd_comma_index+2]-48)*10 + (cmd[cmd_comma_index+3]-48);
									break;
								case 4:
									cmd_y = (cmd[cmd_comma_index+1] - 48)*1000 +(cmd[cmd_comma_index+2] - 48)*100 + (cmd[cmd_comma_index+3] - 48)*10 + (cmd[cmd_comma_index+4] -48);
									break;
							}
							Motion_setCommnad(((float)cmd_x-512)/512, ((float)cmd_y-512)/512);
						}
					}
				}			
			}
			RemoteControl.cmd_index++;
			RemoteControl.cmd_index%= CMD_BUFFER_LENGTH;
	 }
}

uint8_t ascii2hex(uint8_t a)
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


CMD_t GeneralWR;
void GeneralWR_handle(void)
{
	__IO uint8_t cmd[19] = {'@','W','A','A','A','A','A','A','A','A','D','D','D','D','D','D','D','D','#'};
	__IO const uint8_t ascii[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	__IO static uint32_t addr;
	__IO static uint32_t data;
	while(BLE_FIFO_RX_isEmpty(GeneralWR.id) == false)
	{
		GeneralWR.cmd_buffer[GeneralWR.cmd_index] = BLE_FIFO_RX_getData(GeneralWR.id);
		//@WAAAAAAAADDDDDDDD#->@WAAAAAAAADDDDDDDD#
		if(GeneralWR.cmd_buffer[GeneralWR.cmd_index] == '#')
		{
			if(GeneralWR.cmd_buffer[(GeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 18)%CMD_BUFFER_LENGTH ] == '@')
			{
				data = (uint32_t)ascii2hex(GeneralWR.cmd_buffer[(GeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 1)%CMD_BUFFER_LENGTH ]);
				data |= (uint32_t)ascii2hex(GeneralWR.cmd_buffer[(GeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 2)%CMD_BUFFER_LENGTH ])<<4;
				data |= (uint32_t)ascii2hex(GeneralWR.cmd_buffer[(GeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 3)%CMD_BUFFER_LENGTH ])<<8;
				data |= (uint32_t)ascii2hex(GeneralWR.cmd_buffer[(GeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 4)%CMD_BUFFER_LENGTH ])<<12;
				data |= (uint32_t)ascii2hex(GeneralWR.cmd_buffer[(GeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 5)%CMD_BUFFER_LENGTH ])<<16;
				data |= (uint32_t)ascii2hex(GeneralWR.cmd_buffer[(GeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 6)%CMD_BUFFER_LENGTH ])<<20;
				data |= (uint32_t)ascii2hex(GeneralWR.cmd_buffer[(GeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 7)%CMD_BUFFER_LENGTH ])<<24;
				data |= (uint32_t)ascii2hex(GeneralWR.cmd_buffer[(GeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 8)%CMD_BUFFER_LENGTH ])<<28;
				addr = (uint32_t)ascii2hex(GeneralWR.cmd_buffer[(GeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 9)%CMD_BUFFER_LENGTH ]);
				addr |= (uint32_t)ascii2hex(GeneralWR.cmd_buffer[(GeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 10)%CMD_BUFFER_LENGTH ])<<4;
				addr |= (uint32_t)ascii2hex(GeneralWR.cmd_buffer[(GeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 11)%CMD_BUFFER_LENGTH ])<<8;
				addr |= (uint32_t)ascii2hex(GeneralWR.cmd_buffer[(GeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 12)%CMD_BUFFER_LENGTH ])<<12;
				addr |= (uint32_t)ascii2hex(GeneralWR.cmd_buffer[(GeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 13)%CMD_BUFFER_LENGTH ])<<16;
				addr |= (uint32_t)ascii2hex(GeneralWR.cmd_buffer[(GeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 14)%CMD_BUFFER_LENGTH ])<<20;
				addr |= (uint32_t)ascii2hex(GeneralWR.cmd_buffer[(GeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 15)%CMD_BUFFER_LENGTH ])<<24;
				addr |= (uint32_t)ascii2hex(GeneralWR.cmd_buffer[(GeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 16)%CMD_BUFFER_LENGTH ])<<28;
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
				BLE_FIFO_TX_sendData(GeneralWR.id, cmd, 19);
			}
			//@RAAAAAAAA#->@RAAAAAAAADDDDDDDD#
			else if(GeneralWR.cmd_buffer[(GeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 10)%CMD_BUFFER_LENGTH ] == '@') 
			{
				addr = (uint32_t)ascii2hex(GeneralWR.cmd_buffer[(GeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 1)%CMD_BUFFER_LENGTH ]);
				addr |= (uint32_t)ascii2hex(GeneralWR.cmd_buffer[(GeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 2)%CMD_BUFFER_LENGTH ])<<4;
				addr |= (uint32_t)ascii2hex(GeneralWR.cmd_buffer[(GeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 3)%CMD_BUFFER_LENGTH ])<<8;
				addr |= (uint32_t)ascii2hex(GeneralWR.cmd_buffer[(GeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 4)%CMD_BUFFER_LENGTH ])<<12;
				addr |= (uint32_t)ascii2hex(GeneralWR.cmd_buffer[(GeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 5)%CMD_BUFFER_LENGTH ])<<16;
				addr |= (uint32_t)ascii2hex(GeneralWR.cmd_buffer[(GeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 6)%CMD_BUFFER_LENGTH ])<<20;
				addr |= (uint32_t)ascii2hex(GeneralWR.cmd_buffer[(GeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 7)%CMD_BUFFER_LENGTH ])<<24;
				addr |= (uint32_t)ascii2hex(GeneralWR.cmd_buffer[(GeneralWR.cmd_index+ CMD_BUFFER_LENGTH - 8)%CMD_BUFFER_LENGTH ])<<28;
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
				BLE_FIFO_TX_sendData(GeneralWR.id, cmd, 19);
			}
		}
		GeneralWR.cmd_index++;
		GeneralWR.cmd_index%= CMD_BUFFER_LENGTH;
	}
}

void CMD_Handler(void)
{
	RemoteControl_handle();
	GeneralWR_handle();
}

void CMD_Config(void)
{
	if(BLE_FIFO_Register(&RemoteControl.id) == false)
	{
		while(1);
	}
	if(BLE_FIFO_Register(&GeneralWR.id) == false)
	{
		while(1);
	}
}