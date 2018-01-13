/*
* "Drinks" RFID Terminal
* Buy sodas with your company badge!
*
* Benoit Blanchon 2014 - MIT Licensfe
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

void HttpClient::begin(const byte* ip)
{

    //pinMode(4, OUTPUT);
    //pinMode(10, OUTPUT);
    //digitalWrite(4, HIGH);
    //digitalWrite(10, LOW);
    delay(100);
    Serial.begin(9600);

    byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
    
    
    byte myip[4]; //for some reason just passing ip doesnt work, so I copied it again....needs to be rewritten properly
    for(int i=0;i<4;i++)
      myip[i] = ip[i];
    

    // start the Ethernet connection:
    if(ip)
      {
     Ethernet.begin(mac, myip);
     //Ethernet.begin(mac);
     Serial.print("Begin ethernet with IP... ");//Serial.println(myip[3]);
     }
    else
      {Ethernet.begin(mac);Serial.println("DHCP...");}

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

void HttpClient::setServer(char* ip)
{
  serverIP = ip;
  Serial.print("Setting server IP to: ");
  Serial.println(serverIP);
}


bool HttpClient::query(const char* request, char* content, int maxContentSize)
{

     
     // 1. SEND REQUEST
    Serial.println(request);    

    //IPAddress server(10,42,65,29);
    if (!client.connect(SERVER_IP, SERVER_PORT))
    //if (!client.connect(serverIP, SERVER_PORT))
    {
        Serial.print("Connect failed...serverIP: ");
        Serial.println(serverIP);
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


