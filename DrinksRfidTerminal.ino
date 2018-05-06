/*
  "Drinks" RFID Terminal
  Buy sodas with your company badge!

  Benoit Blanchon 2014 - MIT License
  https://github.com/bblanchon/DrinksRfidTerminal
*/

#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoJson.h>
#include <LiquidCrystal.h>
#include <SipHash_2_4.h>
#include <MFRC522.h>
#include <BlynkSimpleEthernet.h>
#include <EEPROM.h>

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


#define SYNC_PERIOD    300000UL // 5 minutes
#define IDLE_PERIOD    15000UL  // 15 seconds
#define TINY_WAIT 250UL //250 ms

char auth[] = "xxxxx";
WidgetTerminal terminal(V0);

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

byte myIP[4] = IP_ADDRESS;

void setup()
{
  Serial.begin(9600);

  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, HIGH);

  Serial.println("Starting display...");
  display.begin();
  display.setBacklight(255);
  display.setBusy();

  Serial.println("Starting encoder...");
  encoder.begin();
      
  Serial.println("Getting IP...");
  getIP();  //Previously used to manually set IP, otherwise use default....now used to get IP from EEPROM

  Serial.println("Starting piezo buzzer...");
  sound.begin();

  Serial.println("Starting Ethernet...");
  http.begin(myIP);

  Serial.println("Setting server IP from EEPROM");
  char serverIP[16];
  sprintf(serverIP, "%d.%d.%d.%d", (int) EEPROM.read(0), (int) EEPROM.read(1), (int) EEPROM.read(2), (int) EEPROM.read(3));
  http.setServer(serverIP);
  
  Serial.println("Starting RFID...");
  rfid.begin();

  display.setText(0, "Blynk connect...");
  display.setText(1, "Plz wait :)");
  Blynk.config(auth);
  Blynk.connect(2000); //2000 * 3ms (blynk default) = 6 seconds
  terminal.flush();

   Serial.println("Starting SYNC...");
  lastSyncTime = millis();
  while (!sync())
  {
    display.setText(0, "Sync failed...");
    display.setText(1, "Trying again.");
    delay(3000);    
  }
  lastSyncTime = 0;
}

void loop()
{
  Blynk.run();

  unsigned long now = millis();

  showSelection();
  bool btnPressed = encoder.btnPressed(); //we assign it in a bool instead of calling the function because calling the function will reset the last known action of button press, so we cannot call it more than once

  // Button left
  if (encoder.leftPressed())
  {
    moveSelectedProduct(-1);
    delay(TINY_WAIT);
    encoder.leftPressed(); //clear last left press
  }
  //Button right
  else if (encoder.rightPressed())
  {
    moveSelectedProduct(+1);
    delay(TINY_WAIT);
    encoder.rightPressed(); //clear last right press
  }
  else if (btnPressed && (lastBadgeTime + IDLE_PERIOD) > now && lastBadge != "")
  {
    encoder.ledChange(false, false, true);
    Serial.print("Last badge before buy send: ");
    Serial.println(lastBadge);
    buy(lastBadge, selectedProduct);
    lastBadge = "";
  }
  else if ((lastBadgeTime + IDLE_PERIOD) < now)
  {
    lastBadge = "";
    encoder.ledChange(false, false, false); //turn off green led
  }
  else if ((lastBadgeTime + IDLE_PERIOD - 3000) < now && lastBadge != "") //beep the last 3 seconds
  {
    sound.play("a1");
    delay(500);
  }
  else if (btnPressed) //case button pressed but no badge presented
  {
    Serial.println("Button pressed...cash transaction");
    encoder.ledChange(true, false, false);
    display.setText(0, "Cash payment");
    display.setText(1, "Not implemented");
    sound.play("a1c1");
    delay(2500);
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
        delay(2000);
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
    
    terminal.print("Badge found:"); //print badge ID to Blynk
    terminal.println(badge); //print badge ID to Blynk
    terminal.flush();
    
    getBalance(badge);

    delay(1000);

    // ignore all waiting badge to avoid unintended double buy
    while (rfid.tryRead())
    {
      Serial.println("rfid.tryRead");
      delay(1000);
    }


    lastEventTime = millis();
    lastBadgeTime = millis();
  }

}

void moveSelectedProduct(int increment)
{
  lastEventTime = millis();
  if (lastBadge != "") lastBadgeTime = millis(); //reset timeout if the user keeps moving
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

  Serial.print("Buying product with DBID: ");Serial.println(catalog.getProductDBID(product));
  const char* myStr = catalog.getProductDBID(product);
  product = atoi(myStr);
  Serial.println(product);
  Serial.println(atoi(myStr));
  Serial.println(myStr);

  if (!buyTransaction.perform(badge, product, clock.getUnixTime()))
  {
    display.setError(buyTransaction.getError());
    encoder.ledChange(true, false, false);
    delay(IDLE_PERIOD / 5);
    return false;
  }

  display.setText(0, buyTransaction.getMessage(0));
  display.setText(1, buyTransaction.getMessage(1));

  if (strcmp(buyTransaction.getMessage(0), "ERROR") == 0)
    encoder.ledChange(true, false, false);
  else
  {
    encoder.ledChange(false, true, false);
    digitalWrite(PIN_RELAY, LOW); //OPEN RELAY
  }
  sound.play(buyTransaction.getMelody());

  if (digitalRead(PIN_RELAY) == LOW) //if relay is open(buy success), wait a little bit to allow the fridge to be opened
    delay(IDLE_PERIOD / 3);

  digitalWrite(PIN_RELAY, HIGH);

  encoder.ledChange(false, false, false); //turn off green led

  Serial.println("BEFORE END");
  return true;
}

bool getBalance(char* badge)
{
  display.setBacklight(255);
  display.setBusy();

  HttpBuyTransaction buyTransaction(http);

  Serial.println("Starting get balance...");
  if (!buyTransaction.getBalance(badge, clock.getUnixTime()))
  {
    display.setError(buyTransaction.getError());
    encoder.ledChange(true, false, false);
    delay(IDLE_PERIOD / 5);
    return false;
  }

  display.setText(0, buyTransaction.getMessage(0));
  display.setText(1, buyTransaction.getMessage(1));

  Serial.println("End get balance...");
  sound.play(buyTransaction.getMelody());
  if (strcmp(buyTransaction.getMessage(0), "ERROR") == 0)
  {
    lastBadge = "";
    Serial.print("Unknown badge");
    
    terminal.println("Unknown badge");//print to Blynk
    terminal.flush();
    
    encoder.ledChange(true, false, false);
    delay(1000);
    encoder.ledChange(false, false, false);
    return false;
  }
  else
  {
    encoder.ledChange(false, true, false);
    return true;
  }
}

bool sync()
{
  display.setBusy();

  HttpSyncTransaction syncTransaction(http);

  if (!syncTransaction.perform())
  {
    Serial.println("Error in sync transactions perform");
    display.setError();
    encoder.ledChange(true, false, false);
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
  sprintf(ip, "%03d.%03d.%03d.%03d", (int) myIP[0], (int) myIP[1], (int) myIP[2], (int) myIP[3]);
  display.setText(1, ip);
}

void getIP()
{
  for(int i=0;i<4;i++)
  {
    myIP[i] = EEPROM.read(i+4);
    Serial.print(myIP[i]);
  }
  Serial.println();

  return;

  /////////////////////////// TESTING TO SAVE IP IN EEPROM WITH BLYNK---> IGNORING MANUAL SET  ////////////////////////////
  
  //memcpy(myIP, myDefineIP, sizeof(myDefineIP)); //copy array from predefined
  display.setBacklight(255);
  display.setText(0, "Set IP?");
  displayIP();

  unsigned long myStart = millis();
  bool myState = true;
  while (IDLE_PERIOD / 5 > millis() - myStart)
  {
    encoder.ledChange(false, false, myState); //flash LED
    if (encoder.btnPressed())
    {
      encoder.ledChange(true, true, true); //flash LED
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
  if (myOct > 254)
    return 0;
  else if (myOct < 0)
    return 254;
  else
    return myOct;
}


void readOctet(int myOctet)
{
  while (!encoder.btnPressed())
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

void writeEEPROM(int pos, byte value)
{
  EEPROM.write(pos, value);
  //EEPROM.commit();
  Serial.print("Saving to EEPROM: ");
  Serial.print(value);
  Serial.print(" in position ");
  Serial.println(pos);
}

BLYNK_WRITE(V0)
{
  String myStr = param.asStr();
  int myOct[4];
  
  if (String("PTL") == myStr) {
    terminal.println("Post Tenebras Lab");

  }
  else if (myStr.indexOf("setserver") == 0)
  {
    terminal.println("Setting server IP");
    //sscanf(&(myStr.c_str()[10]), "%d.%d.%d.%d", &oct1, &oct2, &oct3, &oct4);
    //sscanf(myStr.c_str(), "setserver=%d.%d.%d.%d", &oct1, &oct2, &oct3, &oct4);
    sscanf(myStr.c_str(), "setserver=%d.%d.%d.%d", &myOct[0], &myOct[1], &myOct[2], &myOct[3]);
    char temp[16];
    //sprintf(temp, "%d.%d.%d.%d", oct1, oct2, oct3, oct4);
    sprintf(temp, "%d.%d.%d.%d", myOct[0], myOct[1], myOct[2], myOct[3]);
    terminal.print("Saving to EEPROM: ");
    terminal.println(temp);
    Serial.print("Saving to EEPROM: ");
    Serial.println(temp);

    for(int i=0;i<4;i++)
      writeEEPROM(i, myOct[i]);
      
  }
  else if (myStr.indexOf("setip") == 0)
  {
    terminal.println("Setting terminal IP");
    //sscanf(&(myStr.c_str()[10]), "%d.%d.%d.%d", &oct1, &oct2, &oct3, &oct4);
    //sscanf(myStr.c_str(), "setip=%d.%d.%d.%d", &oct1, &oct2, &oct3, &oct4);
    sscanf(myStr.c_str(), "setip=%d.%d.%d.%d", &myOct[0], &myOct[1], &myOct[2], &myOct[3]);
    char temp[16];
    //sprintf(temp, "%d.%d.%d.%d", oct1, oct2, oct3, oct4);
    sprintf(temp, "%d.%d.%d.%d", myOct[0], myOct[1], myOct[2], myOct[3]);
    terminal.print("Saving to EEPROM: ");
    terminal.println(temp);
    Serial.print("Saving to EEPROM: ");
    Serial.println(temp);

    for(int i=0;i<4;i++) 
      writeEEPROM(i+4, myOct[i]); //+4 because we want to change the pos in EEPROM
      
  }
  else if (String("enablesound") == myStr)
    terminal.println("Enabling sound");
  else if (String("disablesound") == myStr)
    terminal.println("Diabling sound");
  else {
  
    terminal.print("Command not understood:");
    terminal.write(param.getBuffer(), param.getLength());
    terminal.println();
    terminal.println("Help: setip=x.x.x.x");
    terminal.println("setserver=x.x.x.x");
    terminal.println("setdelay=XX");
    terminal.println("enablesound  disablesound");
  }

  // Ensure everything is sent
  terminal.flush();
}

BLYNK_WRITE(V1)
{
  BLYNK_LOG("Open fridge: %s", param.asStr());
  // You can also use:
  // int i = param.asInt() or
  // double d = param.asDouble()
  if(param.asInt())
  {
    digitalWrite(PIN_RELAY, HIGH);
    delay(IDLE_PERIOD /3);
    digitalWrite(PIN_RELAY, LOW);
  }

  
}


