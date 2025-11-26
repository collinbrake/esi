#include "mcc_generated_files/system.h"

#define CTS _RD6 // - port D bit 6
#define RTS _RF12 // output saying we are ready to recieve a UART transmission

// init UART 2
void initU2(int BRG)
{
    U2BRG = BRG; // set baud rate generator register to value for 19200 bits/s
    U2MODE = 0x8000;
    U2STA = 0x0400;
    _TRISF12 = 0; // RTS is an output
    RTS = 1; // active low signal, we are not ready to rx yet
}

// transmit a byte on UART 2
// return what was transmitted (echo)
char putU2(char c)
{
//    while(CTS); // wait for CTS assert (active low) - flow control
    while(U2STAbits.UTXBF); // wait for buffer to be not full
    U2TXREG = c; // tx c
    return c; // return c
}

// recieve
char getU2(void)
{
    RTS = 0; // tell transmitter we are ready
    while(!U2STAbits.URXDA); // what until rx buffer full
    RTS = 1;
    return U2RXREG;
}

long int k;

int main(void)
{
    SYSTEM_Initialize();
    
    initU2(51);
    
    
    while (1)
    {
        while(CTS); // wait for CTS here instead of in putU2
        putU2(0x55); // alternate 0/1/0/1
        for(k=0;k<1200;k++); // delay
        putU2(0xB3);
        for(k=0;k<300000;k++); // delay for 300,000 for USB
    }

    return 1;
}