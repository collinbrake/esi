#include "mcc_generated_files/system.h"

void initLCD(void)
{
    T1CON = 0x8030;
    // delay > 30ms - 16us/count -> 1875 counts. Let's just say 2000 counts, 32ms
    TMR1=0; while(TMR1<2000);
    // init PMP
    PMCON=0x8303;
    PMMODE=0x03FF;
    PMAEN=0x0001;
    
    // PMA address - PMP address 0-13
    PMADDR=0; // RS line - set to instruction register
    PMDIN1=0b00111000; // write line and character setup to instruction reg
    TMR1=0;while(TMR1<3); // delay 48us
    
    // leaving RS the same and changing data, will cause another automatic write
    PMDIN1=0xc; // you will see the enable line going high then low - the enable line going low again actually writes the value out there
    TMR1=0;while(TMR1<3); // delay 48us
    
    // clear screen
    PMDIN1=0x1;
    TMR1=0;while(TMR1<110); // just do a bit more than 1.52ms
    
    // increment and don't shift
    PMDIN1 = 0b00001100;
    TMR1=0;while(TMR1<3); // delay 48us
}

char readLCD(int addr)
{
    // buffer in 44780 that always has valu from previous read - you get junk value first read
    // when reading instruction or data register, do it twice]
    int dummy;
    while(PMMODEbits.BUSY); // make sure PMP is ready to read - as long as busy is 1, it's busy
    PMADDR=addr;  // address is really only one bit for us - 0/1, instruction/data - could use the others 15 bits of PMP in different applications
    dummy=PMDIN1; // causes PMP to read on bus - get junk data out of the way - any time you assign from it it does PMP thing
    while(PMMODEbits.BUSY); // make sure PMP is ready after first read
    return(PMDIN1); // causes an actual PMP read bus like assignment to   
}

// when you read instruction register, you get busy bit on 7th bit and address on the rest.
// and with 0x80 to get only busy bit
#define BusyLCD() readLCD(0) & 0x80

void writeLCD(int addr, char c)
{
    while(BusyLCD());
    while(PMMODEbits.BUSY);
    PMADDR=addr;
    PMDIN1=c;
}

#define putLCD(d) writeLCD(1, (d))
#define cmdLCD(c) writeLCD(0, (c))
#define homeLCD() writeLCD(0,2)
#define clrLCD() writeLCD(0,1)

char digit0, digit1, digit2;

void getDigits(float num)
{
    // convert to int x 10
    int tempi = num*10;
    
    // first digit
    digit0 = tempi % 10;
    tempi = tempi / 10;
    
    // second digit
    digit1 = tempi % 10;
    tempi = tempi / 10;
    
    // third digit
    digit2 = tempi % 10;
    tempi = tempi / 10;
}

float toC(int tempRaw)
{
    return (tempRaw-500) / 10; // from temp sensor datasheet
}

float toF(int tempRaw)
{
    return tempRaw;
}

void displayTemp(int temp)
{
    // Clear display
    clrLCD();
    
    // Home LCD
    homeLCD();
    
    // Display in Celcius
    getDigits(toC(temp));
    putLCD(digit2+48);
    putLCD(digit1+48);
    putLCD('.');
    putLCD(digit0+48);
    putLCD(' ');
    putLCD('C');
    
    // Advance
    cmdLCD(0b11000000);
    
    // Display in Farenheit
    getDigits(toF(temp));
    putLCD(digit2+48);
    putLCD(digit1+48);
    putLCD('.');
    putLCD(digit0+48);
    putLCD(' ');
    putLCD('F');    
}

void _ISR _T1Interrupt (void)
{
    int tempRAW = 82;
    
    // start sample time
    _SAMP=1;
    Nop();
    // wait for valid reading
    while(!_DONE){}
    // get result of buffer
    tempRAW = ADC1BUF0;
    
    int tempmV = tempRAW * 3300 / 1023; // mV
    
    displayTemp(tempmV);

    // Display raw temp on LED's
    PORTA=tempRAW;
    _T1IF=0; // clear flag
}

int main(void)
{
    SYSTEM_Initialize();
    initLCD();
    
    // Init LED
    TRISA=0xff00;
    
    // Init ISR to update every half second
    _T1IP=4; // interrupt priority 4
    T1CON=0x8030;
    PR1=31250 - 1; // target 500ms => 500ms/62.5ns/256 (prescaler) = 31,250 cycles.
    
    _T1IF=0; // always clear the flag before use and in the ISR
    _T1IE=1; // enable interrupt
    
    // Init ADC
    AD1CON1 = 0x80E0; // 0x8 turns on the ADC
    AD1CON2 = 0x0000;
    AD1CHS = 0x0004; // set to temp sensor channel - pin 21, AN4
    AD1CON3 = 0x1F01;
    
    while (1)
    {
    }

    return 1;
}

