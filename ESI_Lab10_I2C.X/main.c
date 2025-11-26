#include "mcc_generated_files/system.h"

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

void initI2C(int BRG)
{
    T2CON = 0x8000;
    T3CON = 0x8000;
    I2C1BRG = BRG;
    I2C1CONbits.I2CEN = 1;
}

void startI2C(void)
{
    TMR3=0;while(TMR3<160); // 10us
    I2C1CONbits.SEN = 1;
    while(I2C1CONbits.SEN); // wait until hw clears SEN
    TMR3=0;while(TMR3<160); // 10us
}

void stopI2C(void)
{
    TMR3=0;while(TMR3<160); // 10us
    I2C1CONbits.PEN = 1;
    while(I2C1CONbits.PEN); // wait until hw clears PEN
    TMR3=0;while(TMR3<160); // 10us
}

char getbyteI2C(void)
{
    I2C1CONbits.RCEN = 1; // enable receiver
    while(!I2C1STATbits.RBF); // wait until full
    I2C1CONbits.ACKEN = 1; // acknowledge - uses DT reg
    TMR3=0;while(TMR3<160); // 10us
    return I2C1RCV;
}

void sendbyteI2C(char data)
{
    // wait until TX buffer is empty
    while(I2C1STATbits.TBF);
    I2C1TRN = data;
    TMR3=0;while(TMR3<160); // 10us
}

int tempRaw, tempC;

#define S7S_ADDR 0x71     // 7-bit address
#define S7S_I2C_WRITE (S7S_ADDR << 1) // address of 7 segment + write

void displayTemp7S()
{
    // get the digits of temperature
    getDigits(tempC);
    
    _RF2 = 1; // trigger for scope
    startI2C();
    sendbyteI2C(S7S_I2C_WRITE);

    // wait for ACK
    while(I2C1STATbits.ACKSTAT); // wait until it is pulled low by secondary
    TMR3=0;while(TMR3<1600); // 100us so we can see ACK frame by itself
    
    sendbyteI2C(digit2 + '0');
    // wait for ACK
    while(I2C1STATbits.ACKSTAT); // wait until it is pulled low by secondary
    TMR3=0;while(TMR3<1600); // 100us so we can see ACK frame by itself

    sendbyteI2C(digit1 + '0');
    // wait for ACK
    while(I2C1STATbits.ACKSTAT); // wait until it is pulled low by secondary
    TMR3=0;while(TMR3<1600); // 100us so we can see ACK frame by itself
    
    sendbyteI2C(digit0 + '0');
    // wait for ACK
    while(I2C1STATbits.ACKSTAT); // wait until it is pulled low by secondary
    TMR3=0;while(TMR3<1600); // 100us so we can see ACK frame by itself
    
    sendbyteI2C(' ');  
    // wait for ACK
    while(I2C1STATbits.ACKSTAT); // wait until it is pulled low by secondary
    TMR3=0;while(TMR3<1600); // 100us so we can see ACK frame by itself
    
    stopI2C();

    TMR3=0;while(TMR3<1600); // 100us so we can see
    _RF2 = 0;
    TMR3=0;while(TMR3<1600); // 100us so we can see
    
    _RF2 = 1; // trigger for scope
    startI2C();
    sendbyteI2C(S7S_I2C_WRITE);
    // wait for ACK
    while(I2C1STATbits.ACKSTAT); // wait until it is pulled low by secondary
    TMR3=0;while(TMR3<1600); // 100us so we can see ACK frame by itself
    sendbyteI2C(0x77); // decimal control
    // wait for ACK
    while(I2C1STATbits.ACKSTAT); // wait until it is pulled low by secondary
    TMR3=0;while(TMR3<1600); // 100us so we can see ACK frame by itself
    sendbyteI2C(0b00000100); // DP after 2nd digit)
    // wait for ACK
    while(I2C1STATbits.ACKSTAT); // wait until it is pulled low by secondary
    TMR3=0;while(TMR3<1600); // 100us so we can see ACK frame by itself
    
    TMR3=0;while(TMR3<1600); // 100us so we can see
    _RF2 = 0;
    TMR3=0;while(TMR3<1600); // 100us so we can see
}

int main(void)
{
    SYSTEM_Initialize();
    initI2C(157);
    TRISFbits.TRISF2 = 0; // use RF2 as trigger output for scope
    _RF2 = 0; // init to 0
    
    // init vars
    tempRaw = 0;
    tempC = 123; //test 7-segment
    
    // init timer
    
    while(1)
    {
//        tempRAW = readADC(4);
//        PORTA = tempRAW;
//        // we are working in units of mV and tenths of degree C, so no
//        // need to factor in the slope of 10mV per C.
//        tempC = (((long)tempRAW * 3300) / 1023) - 500;
        
        displayTemp7S();
        
//        return;
        
    }
}