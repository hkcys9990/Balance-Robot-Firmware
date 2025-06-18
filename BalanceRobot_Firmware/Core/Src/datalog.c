#include "datalog.h"
#define LOCAL_RAM_PHYSICAL_SIZE 512
#define GLOBAL_RAM_PHYSICAL_SIZE 4096
#define MAX_CYCLE_LOG_LENGTH 16

typedef struct {
    uint32_t log_state;// following
    uint32_t cycle_log_length;
    uint32_t cycle_log_memory_addr[MAX_CYCLE_LOG_LENGTH];
    uint32_t downsample_rate;//new added
    uint32_t downsample_counter;//new added
    uint32_t trigger_mode;//new added, 0: equal, 1:condition large than pattern, 2: condition smaller than pattern, follow DatalogTriggerMode
    uint32_t trigger_hold_cycle;//new added
    uint32_t trigger_hold_counter;//new added
    uint32_t trigger_manual_request;
    uint32_t trigger_condition;
    uint32_t trigger_condition_mask;
    uint32_t trigger_condition_pattern;
    uint32_t local_ram[LOCAL_RAM_PHYSICAL_SIZE];
    uint32_t local_ram_write_pointer;
    uint32_t local_ram_read_pointer;
    uint32_t local_ram_buffer_validity;
    uint32_t *global_ram_pointer;
    uint32_t global_ram_write_pointer;
    uint32_t global_ram_start_read_pointer;
    uint32_t global_ram_read_pointer;
		uint32_t glocal_ram_size;
}Datalog_t;

Datalog_t g_Datalog;
uint32_t Datalog_Globalsharememory[GLOBAL_RAM_PHYSICAL_SIZE];

void Datalog_SetLogData(uint8_t log_length, uint32_t* log_data_addr)
{
	if(log_length == 0 || log_length>16)
	{
		log_length = 1;
	}
	g_Datalog.cycle_log_length = log_length;
	for(uint8_t i =0 ; i<g_Datalog.cycle_log_length; i++)
	{
		g_Datalog.cycle_log_memory_addr[i] = log_data_addr[i];
	}	
}
void Datalog_SetStart(void)
{
	g_Datalog.log_state = DATALOG_STATE_READY;
}

void Datalog_SetStop(void)
{
	g_Datalog.trigger_manual_request = 1;
}

DatalogLogState Datalog_GetDatalogStatus(void)
{
	return g_Datalog.log_state;
}

uint32_t Datalog_GetDatalogData(void)
{
	uint32_t data = g_Datalog.global_ram_pointer[g_Datalog.global_ram_start_read_pointer];
	g_Datalog.global_ram_start_read_pointer++;
	g_Datalog.global_ram_start_read_pointer%=GLOBAL_RAM_PHYSICAL_SIZE;
	return data;
}
uint32_t Datalog_GetDatalogSize(void)
{
	return g_Datalog.glocal_ram_size;
}
uint32_t Datalog_GetDatalogCylcleLen(void)
{
	return g_Datalog.cycle_log_length;
}
void Datalog_Config(void)
{
    g_Datalog.log_state = DATALOG_STATE_READY;
    g_Datalog.cycle_log_length = 16;
    g_Datalog.global_ram_pointer = (uint32_t *)Datalog_Globalsharememory;
    g_Datalog.trigger_condition_mask = 0;
    g_Datalog.trigger_condition_pattern = 0xffffffff;
    g_Datalog.local_ram_write_pointer = 0;
    g_Datalog.local_ram_read_pointer = 0;
    g_Datalog.local_ram_buffer_validity = VALID;
    g_Datalog.global_ram_write_pointer = 0;
    g_Datalog.global_ram_read_pointer = 0;
    g_Datalog.trigger_manual_request = 0;
    g_Datalog.downsample_rate = 9;
    g_Datalog.trigger_mode = TRIGGERMODE_EQUAL;
    g_Datalog.trigger_hold_cycle = 0;
		g_Datalog.glocal_ram_size = GLOBAL_RAM_PHYSICAL_SIZE;
}
bool Datalog_MainLoop(void)
{
    if((g_Datalog.log_state == DATALOG_STATE_READY) || (g_Datalog.log_state == DATALOG_STATE_TIRGGERED))
    {
        if(g_Datalog.local_ram_buffer_validity == OVERFLOWED)
        {
            g_Datalog.local_ram_buffer_validity = VALID;
            g_Datalog.log_state = DATALOG_STATE_STOP_OVERFLOWED;
            return false;
        }
        //latch all the data from share memory to function local memory
        uint32_t cpu_local_ram_write_pointer = g_Datalog.local_ram_write_pointer;
        uint32_t cpu_local_ram_read_pointer = g_Datalog.local_ram_read_pointer;
        uint32_t cpu_global_ram_write_pointer = g_Datalog.global_ram_write_pointer;
        uint32_t cpu_global_ram_read_pointer = g_Datalog.global_ram_read_pointer;
        uint32_t cpu_cycle_log_length = g_Datalog.cycle_log_length;
        while(cpu_local_ram_read_pointer != cpu_local_ram_write_pointer)
        {
            if(cpu_global_ram_write_pointer + cpu_cycle_log_length > GLOBAL_RAM_PHYSICAL_SIZE)
            {
                uint32_t i;
                uint32_t l = (GLOBAL_RAM_PHYSICAL_SIZE - cpu_global_ram_write_pointer);
								
                for(i = 0; i < l; i++)
                {
                    g_Datalog.global_ram_pointer[i + cpu_global_ram_write_pointer] = g_Datalog.local_ram[ i + cpu_local_ram_read_pointer];
                }
                l = (cpu_global_ram_write_pointer + cpu_cycle_log_length - GLOBAL_RAM_PHYSICAL_SIZE);
                for( i =0 ; i<l ;i++)
                {
                    g_Datalog.global_ram_pointer[i] = g_Datalog.local_ram[cpu_local_ram_read_pointer + GLOBAL_RAM_PHYSICAL_SIZE - cpu_global_ram_write_pointer + i];
                }
								
                //memcpy( (uint32_t*)(g_Datalog.global_ram_pointer+cpu_global_ram_write_pointer), (uint32_t*)(g_Datalog.local_ram + cpu_local_ram_read_pointer),(GLOBAL_RAM_PHYSICAL_SIZE - cpu_global_ram_write_pointer));
                //memcpy( (uint32_t*)(g_Datalog.global_ram_pointer), (uint32_t*)(g_Datalog.local_ram + cpu_local_ram_read_pointer + GLOBAL_RAM_PHYSICAL_SIZE - cpu_global_ram_write_pointer),(cpu_global_ram_write_pointer + cpu_cycle_log_length - GLOBAL_RAM_PHYSICAL_SIZE));
            }else
            {
                uint32_t i;
                uint32_t l = cpu_cycle_log_length;
								
                for(i = 0; i< l; i++)
                {
                    g_Datalog.global_ram_pointer[cpu_global_ram_write_pointer + i] = g_Datalog.local_ram[cpu_local_ram_read_pointer + i];
                } 
								
                //memcpy( (uint32_t*)(g_Datalog.global_ram_pointer+cpu_global_ram_write_pointer),(uint32_t*)(g_Datalog.local_ram +cpu_local_ram_read_pointer), cpu_cycle_log_length);
            }
            cpu_global_ram_write_pointer += cpu_cycle_log_length;
            cpu_global_ram_write_pointer %= GLOBAL_RAM_PHYSICAL_SIZE;
            cpu_local_ram_read_pointer += MAX_CYCLE_LOG_LENGTH;
            cpu_local_ram_read_pointer %= LOCAL_RAM_PHYSICAL_SIZE;
        }
        cpu_global_ram_read_pointer = cpu_global_ram_write_pointer + GLOBAL_RAM_PHYSICAL_SIZE%cpu_cycle_log_length;
        cpu_global_ram_read_pointer %= GLOBAL_RAM_PHYSICAL_SIZE;
        g_Datalog.local_ram_read_pointer = cpu_local_ram_read_pointer;
        g_Datalog.global_ram_write_pointer = cpu_global_ram_write_pointer;
        g_Datalog.global_ram_read_pointer = cpu_global_ram_read_pointer;

    }
    return true;
}


void Datalog_Handler(void)
{
    if( g_Datalog.downsample_counter >= g_Datalog.downsample_rate)
     {
         if((g_Datalog.log_state == DATALOG_STATE_READY) || (g_Datalog.log_state == DATALOG_STATE_TIRGGERED) && (g_Datalog.local_ram_buffer_validity == VALID))
         {
             uint32_t cla_local_ram_write_pointer = g_Datalog.local_ram_write_pointer;
             uint32_t cla_local_ram_read_pointer = g_Datalog.local_ram_read_pointer;
             if((cla_local_ram_write_pointer < cla_local_ram_read_pointer) && ((cla_local_ram_write_pointer+MAX_CYCLE_LOG_LENGTH) >= cla_local_ram_read_pointer)) // Check whether the next 16 set data reach the tail of the buffer
             {
                     g_Datalog.local_ram_buffer_validity = OVERFLOWED;
                     return;
             }
             // copy 16 set 32-bit variable to buffer
             g_Datalog.local_ram[cla_local_ram_write_pointer++] = (uint32_t)*(uint32_t*)(g_Datalog.cycle_log_memory_addr[0]);
             g_Datalog.local_ram[cla_local_ram_write_pointer++] = (uint32_t)*(uint32_t*)(g_Datalog.cycle_log_memory_addr[1]);
             g_Datalog.local_ram[cla_local_ram_write_pointer++] = (uint32_t)*(uint32_t*)(g_Datalog.cycle_log_memory_addr[2]);
             g_Datalog.local_ram[cla_local_ram_write_pointer++] = (uint32_t)*(uint32_t*)(g_Datalog.cycle_log_memory_addr[3]);
             g_Datalog.local_ram[cla_local_ram_write_pointer++] = (uint32_t)*(uint32_t*)(g_Datalog.cycle_log_memory_addr[4]);
             g_Datalog.local_ram[cla_local_ram_write_pointer++] = (uint32_t)*(uint32_t*)(g_Datalog.cycle_log_memory_addr[5]);
             g_Datalog.local_ram[cla_local_ram_write_pointer++] = (uint32_t)*(uint32_t*)(g_Datalog.cycle_log_memory_addr[6]);
             g_Datalog.local_ram[cla_local_ram_write_pointer++] = (uint32_t)*(uint32_t*)(g_Datalog.cycle_log_memory_addr[7]);

             g_Datalog.local_ram[cla_local_ram_write_pointer++] = (uint32_t)*(uint32_t*)(g_Datalog.cycle_log_memory_addr[8]);
             g_Datalog.local_ram[cla_local_ram_write_pointer++] = (uint32_t)*(uint32_t*)(g_Datalog.cycle_log_memory_addr[9]);
             g_Datalog.local_ram[cla_local_ram_write_pointer++] = (uint32_t)*(uint32_t*)(g_Datalog.cycle_log_memory_addr[10]);
             g_Datalog.local_ram[cla_local_ram_write_pointer++] = (uint32_t)*(uint32_t*)(g_Datalog.cycle_log_memory_addr[11]);
             g_Datalog.local_ram[cla_local_ram_write_pointer++] = (uint32_t)*(uint32_t*)(g_Datalog.cycle_log_memory_addr[12]);
             g_Datalog.local_ram[cla_local_ram_write_pointer++] = (uint32_t)*(uint32_t*)(g_Datalog.cycle_log_memory_addr[13]);
             g_Datalog.local_ram[cla_local_ram_write_pointer++] = (uint32_t)*(uint32_t*)(g_Datalog.cycle_log_memory_addr[14]);
             g_Datalog.local_ram[cla_local_ram_write_pointer++] = (uint32_t)*(uint32_t*)(g_Datalog.cycle_log_memory_addr[15]);

             cla_local_ram_write_pointer &= LOCAL_RAM_PHYSICAL_SIZE - 1;
             //if((~cla_local_ram_read_pointer) & (~cla_local_ram_write_pointer) == 0xffffffff)
             if((cla_local_ram_read_pointer==0) && (cla_local_ram_write_pointer==0))
             {
                 g_Datalog.local_ram_buffer_validity = OVERFLOWED;
             }
             g_Datalog.local_ram_write_pointer  = cla_local_ram_write_pointer;//+= MAX_CYCLE_LOG_LENGTH;
         }
         g_Datalog.downsample_counter = 0;
     }
     else
     {
         g_Datalog.downsample_counter++;
     }
		switch(g_Datalog.log_state)
		{
				case DATALOG_STATE_READY:
				{
						switch(g_Datalog.trigger_mode)
						{
								case TRIGGERMODE_EQUAL:
										if((((uint32_t)*(uint32_t*)g_Datalog.trigger_condition)&(~g_Datalog.trigger_condition_mask)) == g_Datalog.trigger_condition_pattern  || g_Datalog.trigger_manual_request == 1)
										{
												g_Datalog.trigger_manual_request = 0;
												g_Datalog.trigger_hold_counter = g_Datalog.trigger_hold_cycle;
												g_Datalog.log_state = DATALOG_STATE_TIRGGERED;
										}
										break;
								case TRIGGERMODE_LARGERTHAN:
										if((((uint32_t)*(uint32_t*)g_Datalog.trigger_condition)&(~g_Datalog.trigger_condition_mask)) > g_Datalog.trigger_condition_pattern  || g_Datalog.trigger_manual_request == 1)
										{
												g_Datalog.trigger_manual_request = 0;
												g_Datalog.trigger_hold_counter = g_Datalog.trigger_hold_cycle;
												g_Datalog.log_state = DATALOG_STATE_TIRGGERED;
										}
										break;
								case TRIGGERMODE_SMALLERTHAN:
										if((((uint32_t)*(uint32_t*)g_Datalog.trigger_condition)&(~g_Datalog.trigger_condition_mask)) < g_Datalog.trigger_condition_pattern  || g_Datalog.trigger_manual_request == 1)
										{
												g_Datalog.trigger_manual_request = 0;
												g_Datalog.trigger_hold_counter = g_Datalog.trigger_hold_cycle;
												g_Datalog.log_state = DATALOG_STATE_TIRGGERED;
										}
										break;
						}
						break;
				}
				case DATALOG_STATE_TIRGGERED:
				{
						if(g_Datalog.trigger_hold_counter == 0x00)
						{
								g_Datalog.global_ram_start_read_pointer  = g_Datalog.global_ram_read_pointer;
								g_Datalog.log_state = DATALOG_STATE_STOP_TRIGGERED;
						}else
						{
								g_Datalog.trigger_hold_counter--;
						}
						break;
				}
		}
}

