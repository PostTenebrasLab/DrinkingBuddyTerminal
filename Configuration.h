/*
* "Drinks" RFID Terminal
* Buy sodas with your company badge!
*
* Benoit Blanchon 2014 - MIT License
* https://github.com/bblanchon/DrinksRfidTerminal
*/

#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H

//  PLEASE READ ! ! !
// 
//  You need to rename this file into "Configuration.h" in order to compile the project.
//  Then you must change the values to match you actual configuration.

#define PRIVATE_KEY {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'}

#define MAC_ADDRESS {0x01,0x23,0x45,0x67,0x89,0xAB}
//#define IP_ADDRESS { 172, 16, 222, 9 };
#define IP_ADDRESS { 192, 168, 137, 39 };

#define SERVER_NAME "192.168.137.204"

#define SERVER_PORT 5000

#define API_PATH    "/drinks/api"

#endif
