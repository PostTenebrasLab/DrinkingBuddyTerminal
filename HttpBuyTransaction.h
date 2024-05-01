/*
* "Drinks" RFID Terminal
* Buy sodas with your company badge!
*
* Benoit Blanchon 2014 - MIT License
* https://github.com/bblanchon/DrinksRfidTerminal
*/

#ifndef _HTTPBUYTRANSACTION_H
#define _HTTPBUYTRANSACTION_H

#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

class HttpBuyTransaction
{
public:

    HttpBuyTransaction(WiFiClientSecure* client)
        :client_(client)
    {
       buffer[0] = '\0';
       hash_[0] = '\0';
       messages_[0][0] = '\0';
       messages_[1][0] = '\0';
       melody_[0] = '\0';
       error_[0] = '\0';
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

    const char* getMelody() { return melody_; }
    const char* getError() { return error_; }
    const char* getMessage(int i) { return messages_[i]; }
    long getItemPrice() { return itemPrice_; }
    const char* getItemPriceStr() { return itemPriceStr_; }

private:

    bool send(char*, char*, unsigned long);
    bool add(char*, char*, char*, unsigned long);
    bool sendForBalance(char*, unsigned long);
    bool parse();
    bool validate();

    WiFiClientSecure* client_;
    StaticJsonDocument<256> jsonBuffer;
    HTTPClient https;
    char buffer[256];
    char hash_[20];
    char messages_[2][32];
    char melody_[32];
    long time_;
    char error_[48];
    char itemPriceStr_[16];
    long itemPrice_;
};

#endif
