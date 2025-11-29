/*
 * Lab 10.3: Serial Seven Segment Display via I2C
 * 
 * This project displays a temperature value on the Sparkfun Serial 7-Segment Display
 * using the I2C protocol.
 * 
 * Features:
 * - Displays value from tempC variable (in tenths of degrees Celsius)
 * - Positive temps: XX.X°C format with decimal point (e.g., 23.5)
 * - Negative temps: -XX format without decimal (e.g., -12)
 * - Temperature range: -550 to 999 (representing -55.0°C to 99.9°C)
 * 
 * 7-Segment Display:
 * - I2C address: 0x71 (default)
 * - Requires 4.7kΩ pull-up resistors on SCL and SDA
 * 
 * I2C Configuration:
 * - 100kHz clock rate (BRG = 157 at 16MHz)
 * - Uses Timer2 and Timer3 for I2C timing delays
 */

#include "mcc_generated_files/system.h"

// Global variables
char digit0, digit1, digit2, digit3;
char isNegative;
int tempC; // Temperature in tenths of degrees Celsius (-550 to 999)

// 7-segment display defines
#define S7S_ADDR 0x71     // 7-bit address (default for Sparkfun display)
#define S7S_I2C_WRITE (S7S_ADDR << 1) // 7-bit address shifted left + write bit

// Extract individual digits from temperature value
void getDigits(int long tempi)
{
    // Check if negative
    if (tempi < 0) {
        isNegative = 1;
        tempi = -tempi; // make positive for digit extraction
    } else {
        isNegative = 0;
    }
    
    // Extract digits (tenths of degrees)
    // digit0 = tenths place (right of decimal)
    digit0 = tempi % 10;
    tempi = tempi / 10;
    
    // digit1 = ones place (left of decimal)
    digit1 = tempi % 10;
    tempi = tempi / 10;
    
    // digit2 = tens place
    digit2 = tempi % 10;
    tempi = tempi / 10;
    
    // digit3 = hundreds place (if needed)
    digit3 = tempi % 10;
}

// Initialize I2C module
void initI2C(int BRG)
{
    T2CON = 0x8000;  // Enable Timer2 for delays
    T3CON = 0x8000;  // Enable Timer3 for delays
    I2C1BRG = BRG;   // Set baud rate
    I2C1CONbits.I2CEN = 1; // Enable I2C module
}

// Generate I2C start condition
void startI2C(void)
{
    TMR3=0;while(TMR3<160); // 10us delay
    I2C1CONbits.SEN = 1;    // Initiate start condition
    while(I2C1CONbits.SEN); // Wait until hardware clears SEN
    TMR3=0;while(TMR3<160); // 10us delay
}

// Generate I2C stop condition
void stopI2C(void)
{
    TMR3=0;while(TMR3<160); // 10us delay
    I2C1CONbits.PEN = 1;    // Initiate stop condition
    while(I2C1CONbits.PEN); // Wait until hardware clears PEN
    TMR3=0;while(TMR3<160); // 10us delay
}

// Send a byte via I2C
void sendbyteI2C(char data)
{
    while(I2C1STATbits.TBF); // Wait until TX buffer is empty
    I2C1TRN = data;          // Load data into transmit register
    TMR3=0;while(TMR3<160);  // 10us delay
}

// Display temperature on 7-segment display via I2C
void displayTemp7S(void)
{
    // Get the digits of temperature
    getDigits(tempC);
    
    _RF2 = 1; // Trigger signal for logic analyzer
    startI2C();
    sendbyteI2C(S7S_I2C_WRITE); // Send device address + write bit
    while(I2C1STATbits.ACKSTAT); // Wait for ACK from display
    TMR3=0;while(TMR3<1600); // 100us delay for logic analyzer visibility
    
    // Display format depends on sign
    if (isNegative) {
        // Negative: show "-XXC" (no decimal, no tenths digit)
        sendbyteI2C('-');
        while(I2C1STATbits.ACKSTAT);
        TMR3=0;while(TMR3<1600);
        
        sendbyteI2C(digit2 + '0');
        while(I2C1STATbits.ACKSTAT);
        TMR3=0;while(TMR3<1600);
        
        sendbyteI2C(digit1 + '0');
        while(I2C1STATbits.ACKSTAT);
        TMR3=0;while(TMR3<1600);
        
        sendbyteI2C('C');
        while(I2C1STATbits.ACKSTAT);
        TMR3=0;while(TMR3<1600);
    } else {
        // Positive: show "XX.XC" 
        // Sparkfun display fills from left to right
        // Send leftmost character first
        sendbyteI2C(digit2 + '0'); // tens place (2)
        while(I2C1STATbits.ACKSTAT);
        TMR3=0;while(TMR3<1600);
        
        // Send ones place (3)
        sendbyteI2C(digit1 + '0');
        while(I2C1STATbits.ACKSTAT);
        TMR3=0;while(TMR3<1600);

        // Send tenths place (5)
        sendbyteI2C(digit0 + '0');
        while(I2C1STATbits.ACKSTAT);
        TMR3=0;while(TMR3<1600);
        
        // Send rightmost character (C)
        sendbyteI2C('C');
        while(I2C1STATbits.ACKSTAT);
        TMR3=0;while(TMR3<1600);
    }
    
    stopI2C();

    TMR3=0;while(TMR3<1600); // 100us delay
    _RF2 = 0;
    TMR3=0;while(TMR3<1600);
    
    // Set decimal point control
    _RF2 = 1;
    startI2C();
    sendbyteI2C(S7S_I2C_WRITE);
    while(I2C1STATbits.ACKSTAT);
    TMR3=0;while(TMR3<1600);
    
    sendbyteI2C(0x77); // Decimal control command
    while(I2C1STATbits.ACKSTAT);
    TMR3=0;while(TMR3<1600);
    
    if (isNegative) {
        // No decimal point for negative temps
        sendbyteI2C(0b00000000);
    } else {
        // Display shows: [2][3].[5][C] in positions [0][1][2][3] left to right
        // We want decimal after position 1 (after the '3')
        sendbyteI2C(0b00000010);
    }
    while(I2C1STATbits.ACKSTAT);
    TMR3=0;while(TMR3<1600);
    
    stopI2C();
    
    TMR3=0;while(TMR3<1600);
    _RF2 = 0;
    TMR3=0;while(TMR3<1600);
}

int main(void)
{
    // Initialize system
    SYSTEM_Initialize();
    
    // Initialize I2C (100kHz at 16MHz system clock)
    initI2C(157);
    
    // Configure RF2 as output for logic analyzer trigger
    TRISFbits.TRISF2 = 0;
    _RF2 = 0;
    
    // Initialize tempC with a test value
    // Change this value to test different temperatures
    tempC = -235; // Represents 23.5°C
    
    while(1)
    {
        // Display temperature on 7-segment display
        displayTemp7S();
        
        // Add delay between updates (optional)
        TMR3=0;while(TMR3<16000); // ~1ms delay
    }
}
