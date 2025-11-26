#include "mcc_generated_files/system.h"

// init ADC
// 1. assign pins - use mask to mask out unwanted channels
// 2. set control registers
// 3. turn ADC on
void initADC(int amask)
{
    AD1PCFG = amask; // should have 1's for unused channels
    AD1CON1 = 0x00E0;
    AD1CON2 = 0x0000;
    AD1CON3 = 0x1F01;
    // leave channel select alone here - use in read
    
    // start the ADC module
    AD1CON1bits.ADON = 1;
}

// read ADC
// 1. assign channel to the AD1CHS register
// 2. set teh AD1CON1bit.SAMP to start the sampling process
// 3. wait for the DONE flag - AD1CONbits.DONE to be set
// 4. clear the DONE bit
// 5. return the digital value 10 bit integer
int readADC(int ch) // ch channel should be in range 0, 15
{
    AD1CHS = ch;
    AD1CON1bits.SAMP = 1;
    while(!AD1CON1bits.DONE);
    AD1CON1bits.DONE = 0;
    return ADC1BUF0;
}

// comment this out to do the pass-through filter
//#define PWM_GEN;

int c = 0;
int dc = 1;
long int adcRaw = 0;
long int ocVal = 0;

void _ISRFAST _T3Interrupt(void)
{
#ifdef PWM_GEN // PWM signal generation application
    if(++c > 4)
    {
        c = 0;
        dc *= 2;
        if (dc > 100)
            dc = 1;
    }
    OC1RS = 4*dc;
#else // pass-through filter application
    // we are converting an ADC range of [0, 1023] to an
    // output compare range of [0, 400]
    OC1RS = ocVal;
#endif
    // reset interrupt flag
    _T3IF = 0;
}

int main(void)
{
    SYSTEM_Initialize();
    
    initADC(0xFFF7); // unmask just channel AN3
    
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
#ifdef PWM_GEN
        Nop();
#else
        adcRaw = readADC(3);
        ocVal = adcRaw*400/1023;
        Nop();
#endif
    }

    return 1;
}