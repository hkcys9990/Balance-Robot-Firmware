#include "iir.h"


void IIR_Config(IIR_t *filt, 
               float b0, float b1, float b2, float b3,
               float a1, float a2, float a3) 
{
    filt->b[0] = b0;
    filt->b[1] = b1;
    filt->b[2] = b2;
    filt->b[3] = b3;
    filt->a[0] = a1;
    filt->a[1] = a2;
    filt->a[2] = a3;
    for(int i = 0; i < 3; i++) {
        filt->w[i] = 0.0;
    }
}
float IIR_Process(IIR_t *filt, float input) 
{
    float w0 = input 
              - filt->a[0] * filt->w[0] 
              - filt->a[1] * filt->w[1] 
              - filt->a[2] * filt->w[2];
    float output = filt->b[0] * w0 
                  + filt->b[1] * filt->w[0] 
                  + filt->b[2] * filt->w[1] 
                  + filt->b[3] * filt->w[2];
    filt->w[2] = filt->w[1];
    filt->w[1] = filt->w[0];
    filt->w[0] = w0;
    
    return output;
}
