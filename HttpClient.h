/*
* "Drinks" RFID Terminal
* Buy sodas with your company badge!
*
* Benoit Blanchon 2014 - MIT License
* https://github.com/bblanchon/DrinksRfidTerminal
*/

#ifndef _HTTPCLIENT_H
#define _HTTPCLIENT_H

//#include <Ethernet.h>
#include "ESP8266WiFi.h"

class HttpClient
{
public:
    //void begin(const byte* ip = NULL);
    void begin(const byte* ip);

    bool query(const char* path, char* content, int maxContentSize);

    void setServer(char* ip);
    char* serverIP;

private:

    void readln(char*, int);

    IPAddress serverIp;
    WiFiClient client;
    

};

#endif
