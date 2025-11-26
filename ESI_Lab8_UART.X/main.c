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

void getDigits(int long tempi)
{
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

void displayTemp(int long tempC, int long tempF)
{
    // Clear display
    clrLCD();
    
    // Home LCD
    homeLCD();
    
    // Display in Celcius
    getDigits(tempC);
    putLCD(digit2+48);
    putLCD(digit1+48);
    putLCD('.');
    putLCD(digit0+48);
    putLCD(' ');
    putLCD('C');
    
    // Advance
    cmdLCD(0b11000000);
    
    // Display in Farenheit
    getDigits(tempF);
    //putLCD(digit2+48); skip leading zero
    putLCD(digit1+48);
    putLCD(digit0+48);
    putLCD(' ');
    putLCD('F');    
}

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
int onesComp, lastRD7;

int main(void)
{
    SYSTEM_Initialize();
    
    initLCD();
    // baud register:
    // baud rate gen val = 16MHz / (H*9600bit/s) - 1
    // Low speed mode (9600bit/s is even lower than 19200 from lecture): H = 16
    // baud rate gen val = 103.166667
    initU2(103);
    // input CTS
    _TRISD6 = 1; // switch 3, RD6
    // input button to switch F/C
    _TRISD7 = 1; // switch 6, RD7
    // one's complement group number init
    onesComp = 0;
    
    int tempRAW, potRAW;
    int long tempC, tempF, voltage;
    
    T1CON=0x8030;
    initADC(0xFFCF); // unmask just channels AN4 and AN5
    TRISA=0xff00; // set all LED pins to outputs to write lower 8 bits of temp value to LED's
    
    while (1)
    {
        tempRAW = readADC(4);
        PORTA = tempRAW;
        // we are working in units of mV and tenths of degree C, so no
        // need to factor in the slope of 10mV per C.
        tempC = (((long)tempRAW * 3300) / 1023) - 500;
        // degrees F (not tenths)
        tempF = (((long)tempRAW * 29700 / 1023) - 2900) / 50;
        displayTemp(tempC, tempF);
        
        // ========== test =================
//        while(CTS);
//        putU2(0x55);
        // ========== group number ==========
//        if(!CTS); // wait for CTS here instead of in putU2
//        {
//            putU2(0x76);
//            putU2(0x79);
//            putU2(0x03);
//            if (!onesComp)
//            {
//                putU2(0x02);
//                // display ones complement next time
//                onesComp = 1;
//            }
//            else
//            {
//                putU2(0x0b);
//                onesComp = 0;
//            } 
//        }
        // =========== temperature ==============
        // Write to 7-segment
        if(_RD7==1)
        {
            if (!lastRD7)
            {
               putU2(0x76); // clear 
            }
            getDigits(tempC);
            putU2(0x79);
            putU2(0x00);
            putU2(digit2+48);
            putU2(0x79);
            putU2(0x01);
            putU2(digit1+48);
            putU2(0x79);
            putU2(0x02);
            putU2(digit0+48);
            putU2(0x79);
            putU2(0x03);
            putU2(0x0C);
            // decimal
            putU2(0x77);
            putU2(0x02);
        }
        else
        {
            if (lastRD7)
            {
               putU2(0x76); // clear 
            }
            putU2(0x81);
//            getDigits(tempF);
////            putU2(0x79);
////            putU2(0x00);
////            putU2(digit2+48);
//            putU2(0x79);
//            putU2(0x01);
//            putU2(digit1+48);
//            putU2(0x79);
//            putU2(0x02);
//            putU2(digit0+48);
//            putU2(0x79);
//            putU2(0x03);
//            putU2(0x0F);
        }
        lastRD7 = _RD7;
        
        // delay
        long int delay = 32250; // 1/2 second
        TMR1=0;while(TMR1<delay);
    }

    return 1;
}