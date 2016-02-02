/*
  "Drinks" RFID Terminal
  Buy sodas with your company badge!

  Benoit Blanchon 2014 - MIT License
  https://github.com/bblanchon/DrinksRfidTerminal
*/

#include <Ethernet.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include <LiquidCrystal.h>
#include <SipHash_2_4.h>
#include <MFRC522.h>

#include "encoder.h"
#include "Catalog.h"
#include "Clock.h"
#include "Display.h"
#include "HttpBuyTransaction.h"
#include "HttpClient.h"
#include "HttpSyncTransaction.h"
#include "RfidReader.h"
#include "Sound.h"
#include "Configuration.h"


#define SYNC_PERIOD    600000UL // 10 minutes
#define IDLE_PERIOD    15000UL  // 15 seconds

static Encoder encoder;
static Catalog catalog;
static Clock clock;
static Display display;
static HttpClient http;
static RfidReader rfid;
static Sound sound;

int selectedProduct = 0;
unsigned long lastSyncTime = 0;
unsigned long lastEventTime = 0;
unsigned long lastBadgeTime = 0;

char* lastBadge = "";

/*
int ip0 = (int) String(IP_ADDRESS[0]);
int ip1 = (int) String(IP_ADDRESS[1]);
int ip2 = (int) String(IP_ADDRESS[2]);
int ip3 = (int) String(IP_ADDRESS[3]);
*/

//byte myDefineIP[4] = IP_ADDRESS;
byte myIP[4] = IP_ADDRESS;

void setup()
{
  Serial.begin(9600);

  display.begin();
  display.setBacklight(255);
  display.setBusy();
  encoder.begin();

  getIP();  //Manually set IP, otherwise use default

  sound.begin();
  
  http.begin(myIP);
  
  rfid.begin();

  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, LOW);

  while (!sync())
  {
      delay(5000);
  }
}

void loop()
{
  unsigned long now = millis();

  showSelection();

  // Button left
  if (encoder.leftPressed())
  {
    moveSelectedProduct(-1);
  }
  //Button right
  else if (encoder.rightPressed())
  {
    moveSelectedProduct(+1);
  }
  else if (encoder.btnPressed() && (lastBadgeTime + IDLE_PERIOD) > now && lastBadge != "")
  {    
      encoder.ledChange(false,false,true);
      Serial.print("Last badge before buy send: ");
      Serial.println(lastBadge);
      buy(lastBadge, selectedProduct);
      lastBadge = "";
  }
  else if ((lastBadgeTime + IDLE_PERIOD) < now)
  {
      lastBadge = "";
      encoder.ledChange(false,false,false); //turn off green led
  }
  if (now > lastEventTime + IDLE_PERIOD)
  {
    if (selectedProduct != 0)
    {
      selectedProduct = 0;
      showSelection();
    }

    if (now > lastSyncTime + SYNC_PERIOD)
    {
      if (!sync())
      {
        delay(5000);
      }
      return;
    }
  }
  else
  {
    unsigned long remainingTime = lastEventTime + IDLE_PERIOD - now;

    if (remainingTime < 1024)
    {
      display.setBacklight(remainingTime / 4);
    }
    else
    {
      display.setBacklight(255);
    }
  }

  char* badge = rfid.tryRead();

  if (badge)
  {
    lastBadge = badge;
    Serial.print("badge found ");
    Serial.println(badge);
    Serial.print("Last badge changed to: ");
    Serial.println(lastBadge);
    getBalance(badge);

    delay(2000);

    // ignore all waiting badge to avoid unintended double buy
    while (rfid.tryRead());

    

    lastEventTime = millis();
    lastBadgeTime = millis();
  }

}

void moveSelectedProduct(int increment)
{
  lastEventTime = millis();
  selectedProduct = (selectedProduct + increment + catalog.getProductCount()) % catalog.getProductCount();
  showSelection();
}

void showSelection()
{
  display.setText(0, catalog.getHeader());  
  display.setSelection(1, catalog.getProduct(selectedProduct));
}

bool buy(char* badge, int product)
{
  display.setBacklight(255);
  display.setBusy();

  HttpBuyTransaction buyTransaction(http);

  if (!buyTransaction.perform(badge, product, clock.getUnixTime()))
  {
    display.setError();
    encoder.ledChange(true,false,false);
    return false;
  }

  display.setText(0, buyTransaction.getMessage(0));
  display.setText(1, buyTransaction.getMessage(1));
  if(buyTransaction.getMessage(0) == "ERROR")
    encoder.ledChange(true,false,false);
  else
  {
    encoder.ledChange(false,true,false);  
    digitalWrite(PIN_RELAY, LOW); //OPEN RELAY
  }  
  sound.play(buyTransaction.getMelody());

  if(digitalRead(PIN_RELAY)) //if relay is open, wait a little bit to allow the fridge to be opened
    delay(IDLE_PERIOD/3);

  digitalWrite(PIN_RELAY, LOW);
  

  return true;
}

bool getBalance(char* badge)
{
  display.setBacklight(255);
  display.setBusy();

  HttpBuyTransaction buyTransaction(http);

  if (!buyTransaction.getBalance(badge, clock.getUnixTime()))
  {
    display.setError();
    encoder.ledChange(true,false,false);
    return false;
  }

  display.setText(0, buyTransaction.getMessage(0));
  display.setText(1, buyTransaction.getMessage(1));
  encoder.ledChange(false,true,false);
  sound.play(buyTransaction.getMelody());

  return true;
}

bool sync()
{
  display.setBusy();

  HttpSyncTransaction syncTransaction(http);

  if (!syncTransaction.perform())
  {
    display.setError();
    encoder.ledChange(true,false,false);
    return false;
  }

  syncTransaction.getCatalog(catalog);
  clock.setUnixTime(syncTransaction.getTime());

  lastSyncTime = millis();

  return true;
}


void displayIP()
{
  
  char ip[16];
  /*
  String myText = "";
  myText.concat(myIP[0]);
  myText.concat(".");
  myText.concat(myIP[1]);
  myText.concat(".");
  myText.concat(myIP[2]);
  myText.concat(".");
  myText.concat(myIP[3]);
  myText.toCharArray(ip, sizeof(ip));*/
  sprintf(ip, "%03d.%03d.%03d.%03d",(int) myIP[0],(int) myIP[1],(int) myIP[2],(int) myIP[3]);
  display.setText(1,ip);
}

void getIP()
{ 
  //memcpy(myIP, myDefineIP, sizeof(myDefineIP)); //copy array from predefined
  display.setBacklight(255);
  display.setText(0,"Set IP?");
  displayIP();
  
  unsigned long myStart = millis();
  bool myState = true;
  while (IDLE_PERIOD/5 > millis() - myStart)
  {
    encoder.ledChange(false,false,myState); //flash LED
    if(encoder.btnPressed())
    {
      encoder.ledChange(true,true,true); //flash LED
      readOctet(0);
      readOctet(1);
      readOctet(2);
      readOctet(3);
      
      break;
    }   
    myState = ! myState;
    delay(500);  //half a sec wait    
  } 
}

int checkOctet(int myOct)
{
  if(myOct > 254)
    return 0;
  else if(myOct <0)
    return 254;
  else
    return myOct;   
}


void readOctet(int myOctet)
{
  while(!encoder.btnPressed())
  { 
    displayIP();
    if (encoder.leftPressed())
    {
      //myIP[0]--;
      myIP[myOctet] = checkOctet(myIP[myOctet] - 1);
      displayIP();
    }
    //Button right
    else if (encoder.rightPressed())
    {
      //myIP[0]++;
      myIP[myOctet] = checkOctet(myIP[myOctet] + 1);
      displayIP();
    }
    else if (encoder.btnPressed())
    {
      break;
    }     
    delay(200);
  }
}
