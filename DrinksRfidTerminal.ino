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

void setup()
{
  Serial.begin(9600);

  display.begin();
  display.setBacklight(255);
  display.setBusy();

  sound.begin();
  http.begin();
  encoder.begin();
  rfid.begin();

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
    
    //buy(badge, selectedProduct); // TODO ---> We want to buy when we click the button, not when we put the badge. 

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
  encoder.ledChange(false,false,false);
  sound.play(buyTransaction.getMelody());

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
