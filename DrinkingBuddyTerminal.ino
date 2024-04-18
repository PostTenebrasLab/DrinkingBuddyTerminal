#include "Configuration.h"

#include <ESP8266WiFi.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h> // Hardware-specific library

#include <MFRC522.h>

#include <ESP8266HTTPClient.h>
#include "WiFiManager.h"          //https://github.com/tzapu/WiFiManager

#include <NTPClient.h>
#include <WiFiUdp.h>

#include "Barcode.h"
#include "Clock.h"
#include "HttpBuyTransaction.h"
#include "RfidReader.h"
#include "Sound.h"


static RfidReader rfid;
static MyClock myclock;
Barcode barcode(PIN_BARCODE_RX,PIN_BARCODE_TX,PIN_BARCODE_EN);
static Sound sound;

unsigned long previousMillis = 0;
const long interval = 20000; //20 secs

char lastBadge[10] = "";
char lastBarcode[20] = "";

WiFiClient client;
//HttpBuyTransaction dbTransaction(client);
const long utcOffsetInSeconds = 0; // UTC (use 3600 for UTC+1 and 7200 for UTC+2) but UTC is fine for EPOCH
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

TFT_eSPI tft = TFT_eSPI();       // Invoke custom library
TFT_eSprite img = TFT_eSprite(&tft);
unsigned long targetTime = 0;



// The scrolling area must be a integral multiple of TEXT_HEIGHT
#define TEXT_HEIGHT 16 // Height of text to be printed and scrolled
#define BOT_FIXED_AREA 0 // Number of lines in bottom fixed area (lines counted from bottom of screen)
#define TOP_FIXED_AREA 16 // Number of lines in top fixed area (lines counted from top of screen)
#define YMAX 320 // Bottom of screen area

// The initial y coordinate of the top of the scrolling area
uint16_t yStart = TOP_FIXED_AREA;
// yArea must be a integral multiple of TEXT_HEIGHT
uint16_t yArea = YMAX-TOP_FIXED_AREA-BOT_FIXED_AREA;
// The initial y coordinate of the top of the bottom text line
uint16_t yDraw = YMAX - BOT_FIXED_AREA - TEXT_HEIGHT;

// Keep track of the drawing x coordinate
uint16_t xPos = 0;

// We have to blank the top line each time the display is scrolled, but this takes up to 13 milliseconds
// for a full width line, meanwhile the serial buffer may be filling... and overflowing
// We can speed up scrolling of short text lines by just blanking the character we drew
int blank[19]; // We keep all the strings pixel lengths to optimise the speed of the top line blanking

void clearScreen() {
  xPos = 0;
  tft.fillScreen(TFT_BLACK);
  
  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.fillRect(0,0,240,16, TFT_BLUE);
  tft.drawCentreString(" Drinking Buddy - PTL ",120,0,2);

  // Change colour for scrolling zone text
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  // Setup scroll area
  setupScrollArea(TOP_FIXED_AREA, BOT_FIXED_AREA);

  // Zero the array
  for (byte i = 0; i<18; i++) blank[i]=0;
  
}

void iniTFT()
{
  // Setup the TFT display
  tft.init();
  tft.setRotation(0); // Must be setRotation(0) for this sketch to work correctly
  clearScreen();
  img.createSprite(160, 128);
  img.fillSprite(TFT_BLACK);

  targetTime = millis() + 1000;

}

// ##############################################################################################
// Call this function to scroll the display one text line
// ##############################################################################################
int scroll_line() {
  int yTemp = yStart; // Store the old yStart, this is where we draw the next line
  // Use the record of line lengths to optimise the rectangle size we need to erase the top line
  tft.fillRect(0,yStart,blank[(yStart-TOP_FIXED_AREA)/TEXT_HEIGHT],TEXT_HEIGHT, TFT_BLACK);

  // Change the top of the scroll area
  yStart+=TEXT_HEIGHT;
  // The value must wrap around as the screen memory is a circular buffer
  if (yStart >= YMAX - BOT_FIXED_AREA) yStart = TOP_FIXED_AREA + (yStart - YMAX + BOT_FIXED_AREA);
  // Now we can scroll the display
  scrollAddress(yStart);
  return  yTemp;
}

// ##############################################################################################
// Setup a portion of the screen for vertical scrolling
// ##############################################################################################
// We are using a hardware feature of the display, so we can only scroll in portrait orientation
void setupScrollArea(uint16_t tfa, uint16_t bfa) {
  tft.writecommand(ILI9341_VSCRDEF); // Vertical scroll definition
  tft.writedata(tfa >> 8);           // Top Fixed Area line count
  tft.writedata(tfa);
  tft.writedata((YMAX-tfa-bfa)>>8);  // Vertical Scrolling Area line count
  tft.writedata(YMAX-tfa-bfa);
  tft.writedata(bfa >> 8);           // Bottom Fixed Area line count
  tft.writedata(bfa);
}

// ##############################################################################################
// Setup the vertical scrolling start address pointer
// ##############################################################################################
void scrollAddress(uint16_t vsp) {
  tft.writecommand(ILI9341_VSCRSADD); // Vertical scrolling pointer
  tft.writedata(vsp>>8);
  tft.writedata(vsp);
}

void printTft(const char* myStr) {
  printTftnoln(myStr);
  printTftnoln("\r");
}

void printTftnoln(const char* myStr)
{
  int i=0;
  while (myStr[i] != '\0') {
    char data = myStr[i++];
    // If it is a CR or we are near end of line then scroll one line
    if (data == '\r' || xPos>231) {
      xPos = 0;
      yDraw = scroll_line(); // It can take 13ms to scroll and blank 16 pixel lines
    }
    if (data > 31 && data < 128) {
      xPos += tft.drawChar(data,xPos,yDraw,2);
      blank[(18+(yStart-TOP_FIXED_AREA)/TEXT_HEIGHT)%19]=xPos; // Keep a record of line lengths
    }
    //change_colour = 1; // Line to indicate buffer is being emptied
  }
}
void printTft(int myInt)
{
  char buf[11];
  printTft(itoa(myInt, buf, 10));
}

byte red = 31;
byte green = 0;
byte blue = 0;
byte state = 0;
unsigned int colour = red << 11;
void rainbow() {

  if (targetTime < millis()) {
    targetTime = millis() + 100;//10000;

    // Colour changing state machine
    for (int i = 0; i < 320; i++) {
      img.drawFastVLine(i, 0, img.height(), colour);
      switch (state) {
        case 0:
          green += 2;
          if (green == 64) {
            green = 63;
            state = 1;
          }
          break;
        case 1:
          red--;
          if (red == 255) {
            red = 0;
            state = 2;
          }
          break;
        case 2:
          blue ++;
          if (blue == 32) {
            blue = 31;
            state = 3;
          }
          break;
        case 3:
          green -= 2;
          if (green == 255) {
            green = 0;
            state = 4;
          }
          break;
        case 4:
          red ++;
          if (red == 32) {
            red = 31;
            state = 5;
          }
          break;
        case 5:
          blue --;
          if (blue == 255) {
            blue = 0;
            state = 0;
          }
          break;
      }
      colour = red << 11 | green << 5 | blue;
    }

    // The standard ADAFruit font still works as before
    img.setTextColor(TFT_BLACK);
    img.setCursor (12, 5);
    img.print("Post Tenebras Lab");

    img.pushSprite(0, 0);
  }
}

void setup() {
    Serial.begin(115200);
    sound.play("a1");
    SPI.begin();           // Init SPI bus
    Serial.println(F("Starting RFID..."));
    rfid.begin();
    Serial.println(F("Starting tft and wifi..."));
    iniTFT();
    tft.setTextColor(ILI9341_WHITE);
    //tft.setTextSize(2);
    printTft("Starting wifi...");
    
    //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;
    //reset settings - for testing
    //wifiManager.resetSettings();
    wifiManager.setAPCallback(configModeCallback);
    //fetches ssid and pass and tries to connect
    //if it does not connect it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration
    if(!wifiManager.autoConnect()) {
      Serial.println(F("failed to connect and hit timeout"));
      //reset and try again, or maybe put it to deep sleep
      //ESP.reset();
      delay(1000);
    } 
    Serial.println(F("Wifi connected...yay :)"));
    printTft("Wifi connected...yay :)");

    timeClient.begin();
    timeClient.update();
    if(timeClient.isTimeSet()) {
      myclock.setUnixTime(timeClient.getEpochTime());
      String formattedTime = timeClient.getFormattedTime();
      Serial.print(F("NTP Time: "));
      Serial.println(formattedTime);
      printTftnoln("NTP Time: ");
      printTft(formattedTime.c_str());
    }
    else {
      Serial.println(F("NTP Time: Failed to get time"));
      printTft("NTP Time: Failed to get time");
    }

    client.setNoDelay(true);
    client.setSync(true);

    // rfid.begin();
    barcode.begin();

    tft.setTextColor(ILI9341_RED);
    //tft.setTextSize(2);
    printTft("Swipe your card");
    tft.setTextColor(ILI9341_YELLOW);
    sound.play("b1a1");
}

void loop() {
  //barcode.disable(); //something is not right with the class, doesnt work properly 
  //digitalWrite(PIN_BARCODE_EN, LOW); //something is not right with the class, doesnt work properly 
  char* badge = rfid.tryRead();
  if (badge)
  {
    clearScreen();
    
    strcpy(lastBadge,badge);
    Serial.print(F("badge found ")); Serial.println(badge);
    Serial.print(F("Last badge changed to: ")); Serial.println(lastBadge);
    printTft("Found badge");

    barcode.flush(); //empty the buffer before we start the buy cycle, otherwise things will be bought by mistake
    
    if(getUser(badge))
    {
      barcode.enable();//something is not right with the class, doesnt work properly 
      //digitalWrite(PIN_BARCODE_EN, HIGH); //something is not right with the class, doesnt work properly 
      Serial.println(F("Activating barcode reader"));
      previousMillis = millis();
      //printTft("Fridge is open for 5 sec"); 
      printTft("Please scan barcode");      
      while(millis() - previousMillis < interval) //keep scanning codes until it timesout
      {
        if(!waitForBarcode())
        {
          logOut();
          return;
        }
        int myRes = barcode.get(lastBarcode,20);
        if(myRes == -1)
          Serial.println(F("Error barcode"));          
        else
        {
          Serial.print(F("Barcode read: "));Serial.print(lastBarcode);Serial.print(F(" - size: "));Serial.println(myRes);
          if(checkAdminBarcode(lastBarcode))
          {
            int myTotal = 0;
            char bufTotal[4];
            previousMillis = millis();
            printTft("Scan item count");
            while(millis() - previousMillis < interval*2) //keep scanning codes until it timesout, we double our interval
            { 
              if(waitForBarcode())
              {
                resetBarcode();
                previousMillis = millis(); //reset timer
                barcode.get(lastBarcode,20);
                if(!doneBarcode(lastBarcode))
                {
                  myTotal += barcodeToInt(lastBarcode);
                  Serial.println(myTotal);
                  printTftnoln("Item count: ");printTft(myTotal);
                }
                else
                {
                  Serial.print(F("Total to be added: ")); Serial.println(myTotal);
                  
                  printTft("Scan item to add");
                  sprintf(bufTotal, "%i", myTotal);
                  resetBarcode();
                  if(waitForBarcode())  //wait for actual item to be scanned
                  {
                    barcode.get(lastBarcode,20);
                    if(!addBarcode(badge, lastBarcode, bufTotal))
                      printTft("Add barcode in DB first");
                    else
                      printTft("Add ok");
                    previousMillis = millis(); //reset timer
                    printTft("Scan item count");
                  }
                  myTotal = 0;
                  sprintf(bufTotal, "%i", myTotal);
                } 
              }
            }
          }
          else if(checkAddcashBarcode(lastBarcode))
          {
            //do stuff
          }
          else
          { 
            if(buyBarcode(badge, lastBarcode))
            {             
              getUser(badge); //to update the new balance on screen
              
              previousMillis = millis(); // reset the timer to see if the user wants to buy something else
            }
            else
            {
              Serial.println(F("buy NOK"));
              previousMillis = millis();// test only...need to move to the OK section
            }          
          }
        }
      }
      
      logOut();
      return;        
    }
  }
  //rainbow();
  

}

bool getUser(char* badge)
{
  HttpBuyTransaction dbTransaction(client);

  Serial.println(F("Starting get user..."));
  if (!dbTransaction.getBalance(badge, myclock.getUnixTime()))
  {
    Serial.println(F("Error get user..."));
    printTft("Error getting user info");    
    return false;
  }
  
  if (strcmp(dbTransaction.getMessage(0), "ERROR") == 0)
  {
    //lastBadge = "";
    //strcpy(lastBadge,"0");
    resetBadge();
    Serial.print(F(" Unknown badge ")); 
    printTft("Unknown badge");
    
    Serial.println(dbTransaction.getMessage(0));  
    sound.play(dbTransaction.getMelody());
    delay(1500);
    return false;
  }
  else
  {
    Serial.println(F("OK user"));
    Serial.println(dbTransaction.getMessage(0)); 
    Serial.println(dbTransaction.getMessage(1)); 
    printTftnoln(dbTransaction.getMessage(0)); printTftnoln(" - "); printTft(dbTransaction.getMessage(1));
    sound.play(dbTransaction.getMelody());
    return true;
  }
}

bool buyBarcode(char* badge, char* lastBarcode)
{
  HttpBuyTransaction dbTransaction(client);

  if (!dbTransaction.perform(badge, lastBarcode, myclock.getUnixTime()))
  {
    Serial.println(F("Error buy barcode"));
    printTft("Error buy barcode");
    printTft(lastBarcode);
    
    resetBarcode();
    return false;
  }
  
  if (strcmp(dbTransaction.getMessage(0), "ERROR") == 0)
  {
    //lastBadge = "";
    
    Serial.print(F(" Too poor ")); 
    printTft("Not enough credit, too poor");
    
    Serial.println(dbTransaction.getMessage(0)); 
    sound.play(dbTransaction.getMelody()); 
    delay(1500);
    resetBarcode();
    return false;
  }
  else
  {
    Serial.println(F("OK buy"));
    Serial.println(dbTransaction.getMessage(0)); 
    Serial.println(dbTransaction.getMessage(1)); 
    Serial.print(F("Price of last item :"));
    Serial.println(dbTransaction.getItemPriceStr());
    Serial.print(F("============================"));

    printTft(dbTransaction.getMessage(0));
    printTft(dbTransaction.getMessage(1));
    printTftnoln("Price of last item :");
    printTft(dbTransaction.getItemPriceStr());
    printTft("============================");
    sound.play(dbTransaction.getMelody());
    resetBarcode();

    return true;
  }
}

bool addBarcode(char* badge, char* lastBarcode, char* itemCount)
{
  HttpBuyTransaction dbTransaction(client);

  if (!dbTransaction.addItems(badge, lastBarcode, itemCount, myclock.getUnixTime()))
  {
    Serial.println(F("Error add barcode"));
    printTft("Error add barcode");
    printTft(lastBarcode);
    
    resetBarcode();
    return false;
  }
  
  if (strcmp(dbTransaction.getMessage(0), "ERROR") == 0)
  {
    //lastBadge = "";
    
    Serial.print(F(" Barcode does not exist ")); 
    printTft("Barcode does not exist");
    printTft(lastBarcode);
    printTft(dbTransaction.getMessage(1));
    
    Serial.println(dbTransaction.getMessage(1)); 
    sound.play(dbTransaction.getMelody()); 
    delay(1500);
    resetBarcode();
    return false;
  }
  else
  {
    Serial.println(F("OK add"));
    Serial.println(dbTransaction.getMessage(0)); 
    Serial.println(dbTransaction.getMessage(1)); 
    printTft(dbTransaction.getMessage(0));
    printTft(dbTransaction.getMessage(1));
    sound.play(dbTransaction.getMelody());
    resetBarcode();
    return true;
  }
}

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println(F("Entered config mode"));
  Serial.println(WiFi.softAPIP());
  printTft("Wifi in config mode");
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  printTft(myWiFiManager->getConfigPortalSSID().c_str());
}

void resetBadge()
{
  memset(lastBadge,0,sizeof(lastBadge));
}
void resetBarcode()
{
  memset(lastBarcode,0,sizeof(lastBarcode));
}

bool checkAdminBarcode(const char* myBarcode)
{
  if(!strncmp("ptl_admin", myBarcode, strlen("ptl_admin")))
  {
    printTft("Admin mode activated");
    Serial.println(F("Admin mode started"));
    return true;
  }
  
  return false;
}

bool checkAddcashBarcode(const char* myBarcode)
{
  if(!strncmp("ptl_add_money", myBarcode, strlen("ptl_add_money")))
  {
    printTft("Adding cash");
    Serial.println(F("Add cash mode started"));
    return true;
  }

  return false;
}

bool doneBarcode(const char* myBarcode)
{
  Serial.println(myBarcode);
  if(!strncmp("done", myBarcode, strlen("done")))
  {
    printTft("Done...");
    Serial.println(F("End special mode"));
    return true;
  }

  return false;
}

int barcodeToInt(const char* myBarcode)
{
  if(!strncmp("one", myBarcode, strlen("one")))
  {
    return 1;
  }
  else if(!strncmp("five", myBarcode, strlen("five")))
  {
    return 5;
  }
  else if(!strncmp("ten", myBarcode, strlen("ten")))
  {
    return 10;
  }
  return 0;
}

bool waitForBarcode()
{
  //Serial.println("Waiting for barcode read");
  while(!barcode.available())
  {
   yield(); 
   if (millis() - previousMillis >= interval)
    return false;
  }
  return true;
}

void logOut()
{
  resetBadge();
  clearScreen();
  sound.play("d3");
  //barcode.disable(); //something is not right with the class, doesnt work properly
  digitalWrite(PIN_BARCODE_EN, LOW); //something is not right with the class, doesnt work properly
  Serial.println(F("Deactivating barcode reader"));
  Serial.println(F("Timeout...exiting"));
  printTft("Timeout...logout");
  tft.setTextColor(ILI9341_RED);
  printTft("Swipe your card");
  tft.setTextColor(ILI9341_YELLOW);
}
