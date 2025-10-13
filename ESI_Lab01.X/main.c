#include "mcc_generated_files/system.h"

// group #2 => delay 81us
// 7 instruction cycles per for loop iteration x 62.5ns x DELAY = 81 us
#define DELAY_us 185

// group #2 => delay 260ms
// average 13 instruction cycles per for loop iteration with long int math x 62.5ns x DELAY = 260 ms
#define DELAY_ms 320988

int main(void)
{
    SYSTEM_Initialize();
    TRISA=0x0000;
    PORTA=0x0000;
    long int k;

    while (1)
    {
        // block
        for (k=0; k<DELAY_ms; k++)
            Nop();
        // increment
        PORTA++;
    }

    return 1;
}