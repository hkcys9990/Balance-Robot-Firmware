#ifndef __DATALOG_H__
#define __DATALOG_H__

#include "main.h"


typedef enum{
    DATALOG_STATE_STOP_TRIGGERED,
    DATALOG_STATE_STOP_OVERFLOWED,
    DATALOG_STATE_READY,
    DATALOG_STATE_TIRGGERED
}DatalogLogState;

typedef enum
{
    OVERFLOWED,
    VALID
}DatalogBufferDataValidity;

typedef enum
{
    TRIGGERMODE_EQUAL,
    TRIGGERMODE_LARGERTHAN,
    TRIGGERMODE_SMALLERTHAN
}DatalogTriggerMode;

void Datalog_Config(void);
bool Datalog_MainLoop(void);
void Datalog_SetLogData(uint8_t log_length, uint32_t* log_data_addr);
void Datalog_Handler(void);
DatalogLogState Datalog_GetDatalogStatus(void);
void Datalog_SetStop(void);
void Datalog_SetStart(void);
uint32_t Datalog_GetDatalogData(void);
uint32_t Datalog_GetDatalogSize(void);
uint32_t Datalog_GetDatalogCylcleLen(void);


#endif