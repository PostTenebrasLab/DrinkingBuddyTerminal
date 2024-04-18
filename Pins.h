/*
* "Drinks" RFID Terminal
* Buy sodas with your company badge!
*
* Benoit Blanchon 2014 - MIT License
* https://github.com/bblanchon/DrinksRfidTerminal
*/

#ifndef _PINS_H
#define _PINS_H



#define PIN_RFID_RESET -1
#define PIN_RFID_SS    D4

//#define PIN_RELAY       9

#define PIN_BUZZER       D1


#define PIN_BARCODE_EN       -1 //Enable and disable the GND pin of the reader....should be D3, but RFID doesnt work when d3 is chosen
#define PIN_BARCODE_RX       D2
#define PIN_BARCODE_TX       -1

//These are defined in the user_setup.h in the TFT_eSPI library...here for reference only
//#define ILI9341_DRIVER
//#define TFT_CS   D0  // Chip select control pin D8
//#define TFT_DC   D8  // Data Command control pin
//#define TFT_RST  -1 


#endif
