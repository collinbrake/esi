#include "mcc_generated_files/system.h"

void _ISR _T1Interrupt (void)
{
    PORTA=PORTA + 1;
    _T1IF=0; // clear flag
}

int main(void)
{
    SYSTEM_Initialize();

    _T1IP=4; // interrupt priority 4
    TRISA=0xff00;
    _TRISD6=1;
    T1CON=0x8030;
    PR1=16250 - 1; // target 260ms => 260ms/62.5ns/256 (prescaler) = 16,250 cycles.
    
    _T1IF=0; // always clear the flag before use and in the ISR
    _T1IE=1; // enable interrupt
    
    while (1)
    {
        if(_RD6==0)
        {
            PORTA=0; // reset when input D6 is pressed
        }
    }

    return 1;
}