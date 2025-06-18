#ifndef __IIR_H__
#define __IIR_H__

#include "main.h"

typedef struct {
    float b[4]; 
    float a[3]; 
    float w[3]; 
} IIR_t;
float IIR_Process(IIR_t *filt, float input);
void IIR_Config(IIR_t *filt, 
               float b0, float b1, float b2, float b3,
               float a1, float a2, float a3);

#endif