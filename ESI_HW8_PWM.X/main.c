#include "mcc_generated_files/system.h"

int k = 0;

void _ISRFAST _T3Interrupt(void)
{
    // if on positive slope
    if(k < 4)
    {
        OC1RS = 4000*k;
    }
    // else negative slope
    else
    {
        OC1RS = 31998 - 4000*k;
    }
    // increment/reset k
    if(++k >= 8)
    {
        k = 0;
    }
    // reset interrupt flag
    _T3IF = 0;
}

int main(void)
{
    SYSTEM_Initialize();
    
    // 1kHz = 1ms period, prescalar 0
    T3CON=0x8000;
    // 1ms = (PR3+1)*62.5ns*1 => PR3+1 = 1ms / 62.5ns = 16,000
    PR3=16000-1;
    
    OC1R = 0;
    OC1RS = 0;
    
    OC1CON = 0x000E;
    
    _T3IE = 1;
    _T3IF = 0;
    
    while (1)
    {
        Nop();
    }

    return 1;
}