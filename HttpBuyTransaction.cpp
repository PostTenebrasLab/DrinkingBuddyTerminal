/*
* "Drinks" RFID Terminal
* Buy sodas with your company badge!
*
* Benoit Blanchon 2014 - MIT License
* https://github.com/bblanchon/DrinksRfidTerminal
*/

#include <Arduino.h>
#include <ArduinoJson.h>

#include "Configuration.h"
#include "HashBuilder.h"
#include "HttpBuyTransaction.h"

bool HttpBuyTransaction::send(char* badge, char* productString, unsigned long time)
{
    //char productString[6];
    char timeString[11];
    
    //snprintf(productString, sizeof(productString), "%d", product);
    sprintf(timeString, "%lu", time);
    Serial.println(timeString);

    HashBuilder hashBuilder;
    hashBuilder.print(badge);
    hashBuilder.print(productString);
    hashBuilder.print(timeString);
    
    StaticJsonBuffer<JSON_OBJECT_SIZE(6)+20> jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["Tid"] = "4";   //////////---------------------------------------> Change HERE
    json["Badge"] = badge;
    json["Hash"] = hashBuilder.getHash();
    json["Barcode"] = productString;
    json["Time"] = (const char*)timeString;
    json.printTo(buffer, sizeof(buffer));

    return http.query("POST " API_PATH "/buy", buffer, sizeof(buffer));
}

bool HttpBuyTransaction::sendForBalance(char* badge, unsigned long time)
{
    char timeString[11];
    
    sprintf(timeString, "%lu", time);
    Serial.println(timeString);

    HashBuilder hashBuilder;
    hashBuilder.print(badge);
    hashBuilder.print(timeString);
    
    StaticJsonBuffer<JSON_OBJECT_SIZE(4)+12> jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["Tid"] = "4";   //////////---------------------------------------> Change HERE
    json["Badge"] = badge;
    json["Hash"] = hashBuilder.getHash();
    json["Time"] = (const char*)timeString;
    json.printTo(buffer, sizeof(buffer));

    return http.query("POST " API_PATH "/balance", buffer, sizeof(buffer));
}

bool HttpBuyTransaction::add(char* badge, char* barcodeString, char* itemCount, unsigned long time)
{
    //char productString[6];
    char timeString[11];
    
    //snprintf(productString, sizeof(productString), "%d", product);
    sprintf(timeString, "%lu", time);
    Serial.println(timeString);

    HashBuilder hashBuilder;
    hashBuilder.print(badge);
    hashBuilder.print(barcodeString);
    hashBuilder.print(timeString);
    
    StaticJsonBuffer<JSON_OBJECT_SIZE(6)+32> jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["Tid"] = "4";   //////////---------------------------------------> Change HERE
    json["Badge"] = badge;
    json["Hash"] = hashBuilder.getHash();
    json["Barcode"] = barcodeString;
    json["Item_count"] = itemCount;
    json["Time"] = (const char*)timeString;
    json.printTo(buffer, sizeof(buffer));

    return http.query("POST " API_PATH "/add", buffer, sizeof(buffer));
}

bool HttpBuyTransaction::parse()
{
    StaticJsonBuffer<JSON_OBJECT_SIZE(4)+JSON_ARRAY_SIZE(2)> jsonBuffer;

    JsonObject& root = jsonBuffer.parseObject(buffer);
    if (!root.success()) {Serial.println("JSON error"); error = "JSON error"; return false;}

    melody = root["Melody"];
    if (melody == NULL) {Serial.println("No melody sent"); error = "No melody sent"; return false;}

    JsonArray& messageArray = root["Message"];
    if (!messageArray.success()) return false;
    
    messages[0] = messageArray[0];
    messages[1] = messageArray[1];

    time = root["Time"];
    if (time == NULL) {Serial.println("No time sent"); error = "No time sent"; return false;}

    hash = root["Hash"];
    if (hash == NULL) {Serial.println("No hash sent"); error = "No hash sent"; return false;}

    return true;
}

bool HttpBuyTransaction::validate()
{
    HashBuilder hashBuilder;
    hashBuilder.print(melody);
    hashBuilder.print(messages[0]);
    hashBuilder.print(messages[1]);
    hashBuilder.print(time);

    if(strcasecmp(hash, hashBuilder.getHash()) == 0)
      return true;
    else
    {
      Serial.print("Hash incorrect received: ");
      Serial.print(hash); Serial.print(" expecting "); Serial.println(hashBuilder.getHash());
      Serial.print("Time: "); Serial.println(time);
      
      error = "Hash incorrect";
      return false;
    }
}
