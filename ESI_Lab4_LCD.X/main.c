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

void displayNames()
{
    // Init the T output signal
    TRISA=0xff00;
    PORTA=0xff00;
    
    // Signal start sending characters with T
    PORTA=0xff01;
    
    // Clear display
    clrLCD();
    
    // Home LCD
    homeLCD();
    
    // Write first last name
    putLCD('T');
    putLCD('I');
    putLCD('B');
    putLCD('B');
    putLCD('S');
    
    // Advance
    cmdLCD(0b11000000);
    
    // Write second last name
    putLCD('B');
    putLCD('R');
    putLCD('A');
    putLCD('K');
    putLCD('E');
    
    // Set T low - chars written
    PORTA=0xff00;
}

char digit0, digit1, digit2;

void displayNumber(float temp)
{
    // Clear display
    clrLCD();
    
    // Home LCD
    homeLCD();
    
    // convert to int x 10
    int tempi = temp*10;
    
    // first digit
    digit0 = tempi % 10;
    tempi = tempi / 10;
    
    // second digit
    digit1 = tempi % 10;
    tempi = tempi / 10;
    
    // third digit
    digit2 = tempi % 10;
    tempi = tempi / 10;
    
    // dot for decimal place
    putLCD(digit2+48);
    putLCD(digit1+48);
    putLCD('.');
    putLCD(digit0+48);    
}

int main(void)
{
    // set T1CON
    // delay 30ms
    // initialize pmp
    // init LCD
    // Write the value 0011 1000 to the instruction register - line and character setup
    // wait 37us
    // write 0000 1100 to instruction - display on, hide cursor
    // wait 37us
    // write 0000 0001 to inst. clear display
    // wait 1.52ms - add home and clear times
    // write 0000 0110 to inst - automatically increment cursor, not shifting
    // wait 37 us
    SYSTEM_Initialize();
    initLCD();
    
    //displayNames();
    displayNumber(82.5);
    
    
    

    while (1)
    {
        // Add your application code
    }

    return 1;
}
/**
 End of File
*/

