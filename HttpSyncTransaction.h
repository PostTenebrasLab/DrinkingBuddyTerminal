/*
* "Drinks" RFID Terminal
* Buy sodas with your company badge!
*
* Benoit Blanchon 2014 - MIT License
* https://github.com/bblanchon/DrinksRfidTerminal
*/

#ifndef _HTTPSYNCTRANSACTION_H
#define _HTTPSYNCTRANSACTION_H

#include "Catalog.h"
#include "HttpClient.h"

class HttpSyncTransaction
{
public:

    HttpSyncTransaction(HttpClient& http)
        : http(http)
    {

    }

    bool perform()
    {
        //return send() && parse() && validate();
        bool res = send();
        if(res)
          Serial.println("Send OK");
        else
        {
          Serial.println("Send NOK");
          return false;
        }
    

        res = parse();
        if(res)
          Serial.println("parse OK");
        else
        {
          Serial.println("parse NOK");
          return false;
        }

        res = validate();
        if(res)
          Serial.println("validate OK");
        else
        {
          Serial.println("validate NOK");
          return false;
        }
          

        return true;
    }

    void getCatalog(Catalog&);
    unsigned long getTime() { return time/*strtoul(time, 0, 10)*/; }

private:

    bool send();
    bool parse();
    bool validate();

    HttpClient& http;
    static const int buffer_size = 768; //increased for bigger JSON
    char buffer[buffer_size];    
    unsigned long time;
    const char* header;
    const char* products[Catalog::MAX_PRODUCT_COUNT + 1];
    const char* dbID[Catalog::MAX_PRODUCT_COUNT + 1];
    const char* hash;
};

#endif
