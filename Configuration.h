/*
* "Drinks" RFID Terminal
* Buy sodas with your company badge!
*
* Benoit Blanchon 2014 - MIT License
* https://github.com/bblanchon/DrinksRfidTerminal
*/

#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H

#define PRIVATE_KEY {'1','1','1','1','1','1','1','1','1','1','1','1','1','3','3','7'}

#define MAC_ADDRESS {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}

#define IP_ADDRESS { 10, 42, 65, 64 };

#define SERVER_IP  "10.42.129.11"

#define SERVER_NAME "10.42.129.11" //"10.42.65.29" //"10.42.129.11"//

#define SERVER_PORT 80 //5000

#define MAX_PRODUCTS 20

#define API_PATH    "" //"/DrinkingBuddy"

#define TERMINAL_ID    1 //0 for the old terminal on the fridge
 
#endif
