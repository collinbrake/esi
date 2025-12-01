/*
 * Lab 10: Combined I2C Thermometer Project
 * 
 * This project implements a complete digital thermometer system using:
 * - TMP102 temperature sensor (I2C address 0x48, A0 to GND)
 * - Sparkfun Serial 7-Segment Display (I2C address 0x71)
 * - Timer1 interrupt for 1-second temperature readings
 * 
 * Features:
 * - Reads TMP102 temperature every 1 second via Timer1 interrupt
 * - Displays temperature on 7-segment display via I2C
 * - Positive temps: XX.X°C format with decimal point (e.g., 23.5)
 * - Negative temps: -XX°C format without decimal (e.g., -12)
 * - Temperature range: -55.0°C to 99.9°C
 * 
 * TMP102 Configuration:
 * - Normal Mode, 4Hz conversion rate, 12-bit resolution
 * - Configuration Register: 0x60A0
 * 
 * I2C Configuration:
 * - 100kHz clock rate (BRG = 157 at 16MHz)
 * - Uses Timer2 and Timer3 for I2C timing delays
 * - Requires 4.7kΩ pull-up resistors on SCL and SDA
 */

#include "mcc_generated_files/system.h"

// Global variables
char digit0, digit1, digit2, digit3;
char isNegative;
int tempRaw;  // Raw temperature value from TMP102
int tempC;    // Temperature in tenths of degrees Celsius (-550 to 999)
volatile char onesec = 0; // Flag set by Timer1 interrupt every 1 second

// TMP102 temperature sensor defines (A0 connected to ground)
#define TMP102_ADDR 0x48          // 7-bit address with A0 to GND
#define TMP102_I2C_WRITE (TMP102_ADDR << 1)
#define TMP102_I2C_READ ((TMP102_ADDR << 1) | 1)
#define TMP102_TEMP_REG 0x00      // Temperature Register pointer
#define TMP102_CONFIG_REG 0x01    // Configuration Register pointer

// 7-segment display defines
#define S7S_ADDR 0x61             // 7-bit address (default for Sparkfun display)
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



// Initialize TMP102 temperature sensor
// Per datasheet: Write to configuration register for 12-bit, continuous conversion
void initTMP102(void)
{
    startI2C();
    sendbyteI2C(TMP102_I2C_WRITE);      // Address + W
    sendbyteI2C(TMP102_CONFIG_REG);                  // Pointer = Config register
    sendbyteI2C(0x00);                  // Config MSB: 12-bit resolution
    sendbyteI2C(0x80);                  // Config LSB: 4Hz conversion rate
    stopI2C();
}

// Read temperature from TMP102 sensor
// Per datasheet: Point to temp register, then read 2 bytes
int readTMP102(void)
{
    int tempHigh, tempLow;
    int timeout;
    
    // Write pointer to temperature register (0x00)
    startI2C();
    sendbyteI2C(TMP102_I2C_WRITE);      // Address + W
    sendbyteI2C(TMP102_TEMP_REG);       // Pointer = Temperature register
    stopI2C();
    
    // Read
    startI2C();
    sendbyteI2C(TMP102_I2C_READ);       // Address + R
    
    // Read MSB with timeout
    I2C1CONbits.RCEN = 1;
    timeout = 1000;
    while(!I2C1STATbits.RBF && timeout--);
    if (timeout == 0) {
        stopI2C();
        return 0x1900; // Return 25°C to indicate timeout
    }
    tempHigh = I2C1RCV;
    I2C1CONbits.ACKDT = 0;   // ACK
    I2C1CONbits.ACKEN = 1;
    
    // Read LSB with timeout
    I2C1CONbits.RCEN = 1;
    timeout = 1000;
    while(!I2C1STATbits.RBF && timeout--);
    if (timeout == 0) {
        stopI2C();
        return 0x1900; // Return 25°C to indicate timeout
    }
    tempLow = I2C1RCV;
    I2C1CONbits.ACKDT = 1;   // NACK
    I2C1CONbits.ACKEN = 1;
    
    stopI2C();
    
    return (tempHigh << 8) | tempLow;
}

// Convert TMP102 raw value to tenths of degrees Celsius
// TMP102 12-bit format: MSB contains D11-D4, LSB bits 7-4 contain D3-D0
// Resolution: 0.0625°C per LSB
int convertTMP102ToTenthsC(int raw)
{
    int temp;
    
    // Shift right 4 bits to get 12-bit value (bits D11-D0)
    temp = raw >> 4;
    
    // Handle negative temperatures (two's complement, bit 11 is sign)
    if (temp & 0x800) {
        temp |= 0xF000;  // Sign extend to 16 bits
    }
    
    // Convert: 0.0625°C per count = 0.625 tenths per count
    // Multiply by 10, then multiply by 0.0625
    // = temp * 10 * 625 / 10000 = temp * 625 / 1000
    // Simpler: multiply by 5, divide by 8
    temp = (temp * 5) / 8;
    
    return temp;
}

// Display temperature on 7-segment display via I2C
void displayTemp7S(void)
{
    int timeout;
    
    // Get the digits of temperature
    getDigits(tempC);
    
    _RF2 = 1; // Trigger signal for logic analyzer
    startI2C();
    sendbyteI2C(S7S_I2C_WRITE); // Send device address + write bit
    
    // Check for ACK with timeout (device may not respond if wrong address)
    timeout = 1000;
    while(I2C1STATbits.ACKSTAT && timeout--); // Wait for ACK or timeout
    
    if (timeout == 0) {
        // No ACK received - wrong address, device not present
        stopI2C();
        _RF2 = 0;
        TMR3=0;while(TMR3<1600); // Small delay before retry
        return; // Exit and will retry on next call
    }
    
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
        sendbyteI2C(0b00000010);
    }
    while(I2C1STATbits.ACKSTAT);
    TMR3=0;while(TMR3<1600);
    
    stopI2C();
    
    TMR3=0;while(TMR3<1600);
    _RF2 = 0;
    TMR3=0;while(TMR3<1600);
}

// Timer1 Interrupt Service Routine
// Sets onesec flag every 1 second
void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void)
{
    onesec = 1;             // Set flag to indicate 1 second has elapsed
    IFS0bits.T1IF = 0;      // Clear Timer1 interrupt flag
}

// Initialize Timer1 for 1-second interrupts
// Assumes 16MHz oscillator (FCY = 8MHz instruction cycle)
void initTimer1(void)
{
    T1CON = 0x0000;         // Stop timer, clear settings
    T1CONbits.TCKPS = 0b11; // Prescaler 1:256
    // Timer frequency: FCY / prescaler = 8MHz / 256 = 31.25 kHz
    // For 1 second: 31250 counts
    PR1 = 31250;            // Period register for 1 second
    TMR1 = 0;               // Clear timer counter
    IFS0bits.T1IF = 0;      // Clear interrupt flag
    IEC0bits.T1IE = 1;      // Enable Timer1 interrupt
    T1CONbits.TON = 1;      // Start Timer1
}

int main(void)
{
    // Initialize system
    SYSTEM_Initialize();
    
    // Initialize I2C (100kHz at 16MHz system clock)
    initI2C(157);
    
    // Initialize TMP102 temperature sensor
    initTMP102();
    
    // Initialize Timer1 for 1-second interrupts
    initTimer1();
    
    // Configure RF2 as output for logic analyzer trigger
    TRISFbits.TRISF2 = 0;
    _RF2 = 0;
    
    // Initialize variables
    tempRaw = 0;
    tempC = 123;
    onesec = 0;
    
    while(1)
    {
        // Check if 1 second has elapsed (flag set by Timer1 interrupt)
        if (onesec) {
            // Read temperature from TMP102 sensor
            tempRaw = readTMP102();
            
            // Convert raw value to tenths of degrees Celsius
            tempC = convertTMP102ToTenthsC(tempRaw) | 0x01;
            
            // Clear the one-second flag
            onesec = 0;
        }
        
        // Continuously display temperature on 7-segment display
        displayTemp7S();
        
        // Small delay between display updates
        TMR3=0;while(TMR3<16000); // ~1ms delay
    }
}
