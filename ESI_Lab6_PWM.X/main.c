#include "mcc_generated_files/system.h"

int c = 0;
int dc = 1;

void _ISRFAST _T3Interrupt(void)
{
    if(++c > 4)
    {
        c = 0;
        dc *= 2;
        if (dc > 100)
            dc = 1;
    }
    OC1RS = 4*dc;
    // reset interrupt flag
    _T3IF = 0;
}

int main(void)
{
    SYSTEM_Initialize();
    
    // 25us period, prescalar 1
    T3CON=0x8000;
    // 25us = (PR3+1)*62.5ns*1 => PR3+1 = 25us / 62.5ns = 400
    PR3=400-1;
    
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