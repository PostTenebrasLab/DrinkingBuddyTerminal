/*
* "Drinks" RFID Terminal
* Buy sodas with your company badge!
*
* Benoit Blanchon 2014 - MIT License
* https://github.com/bblanchon/DrinksRfidTerminal
*/

#include <Arduino.h>

#include "RfidReader.h"

byte RfidReader::parseHexNibble(char c)
{
    if (c >= 'A') return c - 'A' + 0xA;
    if (c >= 'a') return c - 'a' + 0xA;
    return c - '0';
}

byte RfidReader::parseHexByte(char* str)
{
    return (parseHexNibble(str[0]) << 4) | parseHexNibble(str[1]);
}

byte RfidReader::computeCheckSum(char badge[])
{
    char checkSum = 0;

    for (int i = 0; i < 5; i++)
    {
        checkSum ^= parseHexByte(badge + 2 * i);
    }

    return checkSum;
}

char* RfidReader::tryRead()
{
    int i;

        // Look for new cards
    if ( ! mfrc522.PICC_IsNewCardPresent())
        return NULL;

    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial())
        return NULL;


    // convert UID to string
    for(int i=0; i<mfrc522.uid.size; i++) {
      sprintf(&buffer[i*2], "%02X", (int)mfrc522.uid.uidByte[i]);
    }

    return buffer;
}
