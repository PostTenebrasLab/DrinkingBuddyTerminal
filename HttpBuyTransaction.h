/*
* "Drinks" RFID Terminal
* Buy sodas with your company badge!
*
* Benoit Blanchon 2014 - MIT License
* https://github.com/bblanchon/DrinksRfidTerminal
*/

#ifndef _HTTPBUYTRANSACTION_H
#define _HTTPBUYTRANSACTION_H

#include "HttpClient.h"

class HttpBuyTransaction
{
public:

    HttpBuyTransaction(HttpClient& http)
        :http(http)
    {
    }

    bool perform(char* badge, char* product, unsigned long time)
    {
        return send(badge, product, time) && parse() && validate();
    }

    bool getBalance(char* badge, unsigned long time)
    {
        return sendForBalance(badge, time) && parse() && validate();
    }

    bool addItems(char* badge, char* barcode, char* itemCount, unsigned long time)
    {
        return add(badge, barcode, itemCount, time) && parse() && validate();
    }

    const char* getMelody() { return melody; }
    const char* getError() { return error; }
    const char* getMessage(int i) { return messages[i]; }

private:

    bool send(char*, char*, unsigned long);
    bool add(char*, char*, char*, unsigned long);
    bool sendForBalance(char*, unsigned long);
    bool parse();
    bool validate();

    HttpClient& http;
    char buffer[150];
    const char* hash;
    const char* messages[2];
    const char* melody;
    const char* time;
    const char* error;
};

#endif
