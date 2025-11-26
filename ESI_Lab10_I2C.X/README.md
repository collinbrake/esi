Project 10: I2C
In this project, you will use the I2C protocol to communicate with a temperature sensor and the
serial seven segment display on the same I2C bus.
10.1 Project 10 Checklists
Project 10: Demonstration Checklist
⃝ Have the in-real time logic analyzer measurement of one message or the series of messages in
which the value of tempC is being sent to the serial seven segment display from the Serial
Seven Segment Display via I2C project ready.
□ Be prepared to show the in-real-time logic analyzer measurement of the series of messages for
the combined I2C project.
Project 10: Documentation Checklist
□ A block diagram/schematic of the Combined I2C Thermometer project
□ Sufficiently commented code for implementing the Serial Seven Segment Display via I2C project
□ A logic analyzer capture of one message or the series of messages in which the value of tempC
is being sent to the serial seven segment display.
□ Comparison of the logic analyzer capture of one message or the series of messages in which a
temperature value is being read from the temperature sensor and the figure you drew beforehand illustrating the message.
□ Comparison of the logic analyzer capture of one message or the series of messages in which
the value of tempC is being sent to the serial seven segment display and the corresponding
message or messages using the incorrect device address
□ Annotation of the logic analyzer capture of one message or the series of messages in which the
value of tempC is being sent to the serial seven segment display.
□ Sufficiently commented code for implementing the The TMP102 Temperature Sensor project
□ A logic analyzer capture that includes the messages required to read a value from the Temperature Register of the TMP102.
□ Annotation of the logic analyzer capture that includes the messages required to read a value
from the Temperature Register of the TMP102.
□ Sufficiently commented code for implementing the The Combined I2C Thermometer Project
project
□ A logic analyzer capture of the whole communication cycle.
□ Annotation and analysis of the logic analyzer capture for the whole communication cycle.
10.2 Project Foundation
This project requires that you study and understand the datasheets for the Sparkfun serial seven
segment display and the TMP102 temperature sensor. In this project we will only require that the
temperature be displayed in degrees Celsius.
Note: Part (A) of the project foundation is included in Homework 13, and the remaining parts of
the project foundation are included in Homework 14. You should complete the project foundation
before continuing in the project.
(A) If device is to be communicated with via I2C, a first step is to determine its device address.
(1) Look in the online data sheet for the Sparkfun serial-seven-segment display and determine
the default device address for the Sparkfun serial-seven-segment display. What other
addresses can the Sparkfun serial-seven-segment-display be given? How can the Sparkfun
serial-seven-segment-display be given such an address?
(2) The TMP102 temperature sensor has four possible device addresses, determined by an
input pin A0. (Input pin A0 is sampled “continuously” to determine which connection is
being made.) Looking at the data sheet determine the four possible device addresses for
the TMP102, and how the A0 pin should be connected to use each address.
(B) Looking at the TMP102 temperature sensor’s data sheet, determine a relationship between the
digital output of the TMP102 and the measured temperature.
(C) Determine the value that must be written to the Configuration Register of the TMP102
temperature sensor for the sensor to operate in Normal Mode, at a 4 Hz Conversion Rate,
using a Converter Resolution of 12 bits, with neither Shutdown Mode, nor Thermostat Mode
enabled. Since we will be using neither the “Alert” pin nor will we be checking for faults, the
Configuration Register bits associated with these features can be ignored.
(D) How many registers are present in the TMP102 temperature sensor, and what are the registers.
(E) Using the device address from Part (A.2) where the A0 pin is connected to ground, and the value
determined in Part (C), draw two figures illustrating1 the I2C message. Pay particular attention
to the section labled “WRITING/READING OPERATION” that starts near the bottom of the
left column of page 10 of the datasheet. The first figure should explicitly show the bits being
transmitted in each frame, and the second figure should use shorthand representations2 of each
frame. Add labels for each of the address and data frames, each acknowledge, the start event,
and the stop event. Add additional notation to indicate which device “does each part.”
(F) What is the sequence necessary to read a value from the Temperature Register? Draw two
figures illustrating the I2C messages required to read a value from the Temperature Register.
As with Part (E), the first figure should explicitly show the bits being transmitted in each
frame, and the second figure should use shorthand representations of each frame. Add labels
for each of the address and data frames, each acknowledge, the start event, and the stop event.
Add additional notation to indicate which device “does each part.”
1This is the same as the figures drawn with boxes in the lectures for a start event, stop event, the
acknowledge, and for the address(s) and data frame(s). The data was written in the box for the appropriate frames.
2For a given bit pattern, the hexadecimal representation of the bit pattern is the shorthand notation. For
data bits, something like D < 27 : 13 > is the shorthand notation.
(G) Draw a detailed diagram/schematic that shows the connections between the PIC24FJ128GA010,
the TMP102 temperature sensor, and the Sparkfun serial segment display. Be sure to label pin
numbers, and connection names. Do not forget to include two 4.7 kΩ pull-up resistors, one for
SCL, and the other for SDA.
10.3 Serial Seven Segment Display via I2C
(A) Because a previous owner may have assigned a different device address to the serial sevensegment display, make sure that you have completed the “Factory Reset” project in Seciont 9.4
before attempting this project.
(B) Write and implement a program that takes the value of a global variable tempC where tempC
represents temperature in tenths of degrees Celsius, and where −550 ≤ tempC ≤ 999, and
displays the temperature in degrees Celsius on the Sparkfun serial seven segment display.
Communication between the microcontroller and the serial seven segment display should be
using the I2C protocol. If the temperature in degrees Celsius is non-negative, two digits to
the left of a decimal place, and one digit to the right of a decimal place must be displayed. If
the temperature in degrees Celsius is negative, a negative sign and the two digits to the left of
the decimal place (but not the decimal place itself) should be displayed. In either case, the
units
◦C must be displayed.
(C) Obtain a logic analyzer capture of one message or the series of messages in which the value of
tempC is being sent to the serial seven segment display via I2C.
(D) Annotate the logic analyzer capture, showing each start event, each acknowledge, and each
address frame, and each data frame. How does this compare with what you were expecting?
(E) Change the device address to the wrong device address, and obtain a logic analyzer capture
of the same message or the same series of messages as part (B). Annotate this logic analyzer
capture, comparing and contrasting this logic analyzer capture with the logic analyzer capture
from part (B).
10.4 The TMP102 Temperature Sensor
(A) Write and implement a program that initializes the TMP102 temperature sensor, and one every
second reads the value from the Temperature Register, storing the value in the global variable
tempRAW . Use a timer interrupt to obtain the timing, setting a user-defined flag named onesec
when the timer interrupt is generated. In the main infinite loop, have an if statement that
checks the value of the onesec flag. If the onesec flag is set, then read the Temperature Register
and clear the onesec flag. We use this construction so that we are not calling any functions (for
example startI2C() in an interrupt service routine.
(B) Run your program, including a breakpoint when the onesec flag is cleared, checking the value
of the tempRAW variable using the watch window. Verify that this value corresponds to an
appropriate temperature.
(C) Obtain a logic analyzer capture that includes the messages required to read a value from the
Temperature Register of the TMP102. Set up your triggering so that the logic analyzer triggers
every time at the beginning of these messages. Compare the value present in the data transfer
with the value stored in tempRAW .
(D) Annotate the logic analyzer capture, showing each start event, each acknowledge, and each
address frame, and each data frame. How does this compare with what you were expecting?
10.5 The Combined I2C Thermometer Project
(A) Starting with the code from the TMP102 “Temperature Sensor” project, add the code necessary
to convert tempRAW the code generated by the TMP102 into the corresponding temperature
in tenths of degrees Celsius. Store the result in the variable tempC. This conversion should
occur within the if statement in which the Temperature Register of the TMP102 is read.
(B) Following the if statement in which the Temperature Register of the TMP102 is read, add the
code from the “Serial Seven Segment Display via I2C ” project.
(C) Using a breakpoint when the onesec flag is cleared, verify that the value stored in tempRAW
makes sense, and that the value stored in tempC is correctly being displayed on the serial seven
segment display.
(D) Obtain a logic analyzer capture that include a whole cycle of the Temperature Register being
read and the temperature value being written to the serial seven segment display. Including
all of the messages in the cycle probably makes it impossible to read the data values in each
message. Therefore, by noting the positions of convenient starting locations, obtain the logic
analyzer captures necessary to be able to analyze the whole cycle of messages.
(E) Annotate the logic analyzer capture, showing each start event, each acknowledge, and each
address frame, and each data frame. Do the messages make sense?

This is using the 100-pin PIC24

Temperature sensor: tmp102.pdf

7-segment display: https://github.com/sparkfun/Serial7SegmentDisplay/wiki/Interface-specifications