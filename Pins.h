/*
* "Drinks" RFID Terminal
* Buy sodas with your company badge!
*
* Benoit Blanchon 2014 - MIT License
* https://github.com/bblanchon/DrinksRfidTerminal
*/

#ifndef _PINS_H
#define _PINS_H

// RGB Led
#define ledRED   17
#define ledGREEN 16
#define ledBLUE  15

// Rotary encoder
#define encoder0PinA  21   //pinA
#define encoder0PinB  19   //pinB
#define encoder0Press 18   //Button

#define PIN_LCD_LIGHT  23
#define PIN_LCD_RS     35
#define PIN_LCD_EN     33
#define PIN_LCD_D4     31
#define PIN_LCD_D5     29
#define PIN_LCD_D6     27
#define PIN_LCD_D7     25

#define PIN_RFID_RESET 42
#define PIN_RFID_SS    44
#define PIN_RFID_MISO  50
#define PIN_RFID_MOSI  51
#define PIN_RFID_CLK   52

#define PIN_BUZZER     8

#define PIN_RELAY       9

#endif
