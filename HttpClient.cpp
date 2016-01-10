/*
* "Drinks" RFID Terminal
* Buy sodas with your company badge!
*
* Benoit Blanchon 2014 - MIT License
* https://github.com/bblanchon/DrinksRfidTerminal
*/

#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Dns.h>

#include "Configuration.h"
#include "HttpClient.h"

#define xstr(s) str(s)
#define str(s) #s

#define NOT_EMPTY(s) (s[0]!=0 && s[0]!='\r' && s[0]!='\n')

void HttpClient::begin()
{
    delay(100);

    byte mac[6] = {0};
    byte ip[4] = IP_ADDRESS;
    Serial.println("DHCP...");

    // start the Ethernet connection:
    Ethernet.begin(mac, ip);

    Serial.print("Address=");
    Serial.println(Ethernet.localIP());
    
    Serial.print("Subnet=");
    Serial.println(Ethernet.subnetMask());
    
    Serial.print("DNS=");
    Serial.println(Ethernet.dnsServerIP());

    Serial.println("Resolve " SERVER_NAME "...");
    
    DNSClient dns;

    dns.begin(Ethernet.dnsServerIP());

    while (1 != dns.getHostByName(SERVER_NAME, serverIp))
    {
        Serial.println("Failed. Retry...");
        delay(10);
    }

    Serial.print("Address=");
    Serial.println(serverIp);

}

void HttpClient::readln(char* buffer, int size)
{
    int i = 0;
    bool connected = true;

    while (i < size - 1)
    {
        if (client.available()>0)
        {
          char c = client.read();
          Serial.print(c);  
          
          if (c == '\n') break;
              
          buffer[i++] = c;          
        }
        else if (!connected)
        {
            //Serial.println("interrupted");
            break;    
        }
        
        connected = client.connected();
    }

    buffer[i] = 0;
}

bool HttpClient::query(const char* request, char* content, int maxContentSize)
{
     // 1. SEND REQUEST
    Serial.println(request);    

    if (!client.connect(serverIp, SERVER_PORT))
    {
        Serial.println("Connect failed");
        return false;
    }
    
    client.print(request);
    client.println(" HTTP/1.1");
    client.println("Host: " SERVER_NAME ":" xstr(SERVER_PORT));
    client.println("Accept: application/json");
    client.println("Connection: close");

    if (content[0])
    {
        client.println("Content-Type: application/json");
        client.print("Content-Length: ");
        client.println(strlen(content));
        client.println();

        Serial.println(content);
        client.println(content);
    }
    else
    {
        client.println();
    }

    // 2. READ RESPONSE
    
    // skip HTTP headers
    while (readln(content, maxContentSize), NOT_EMPTY(content));

    // read content
    readln(content, maxContentSize);
        
    client.stop();

    return content[0] != 0;
}


