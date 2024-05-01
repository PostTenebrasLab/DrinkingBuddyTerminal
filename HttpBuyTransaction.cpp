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
    char timeString[16];
    
    sprintf(timeString, "%lu", time);
    Serial.println(timeString);

    HashBuilder hashBuilder;
    hashBuilder.print(badge);
    hashBuilder.print(productString);
    hashBuilder.print(timeString);
    
    JsonObject json = jsonBuffer.to<JsonObject>();
    json["Tid"] = TERMINAL_ID;
    json["Badge"] = badge;
    json["Hash"] = hashBuilder.getHash();
    json["Barcode"] = productString;
    json["Time"] = (const char*)timeString;
    //json.printTo(buffer, sizeof(buffer));
    serializeJson(json, buffer, sizeof(buffer));

    https.useHTTP10(true);

    char request[32];
    snprintf(request, 32, "%s%s", API_PATH, "/buy");
    https.begin(*client_, SERVER_NAME, SERVER_PORT, request, true);
    https.addHeader("Content-Type", "application/json");
    int ret = https.POST(buffer);

    if(ret != HTTP_CODE_OK) {
      Serial.print(F("/buy returned error ("));
      Serial.print(ret);
      Serial.print(F(") : "));
      Serial.println(HTTPClient::errorToString(ret));
      return false;
    }

    // Read response
    DeserializationError deserialError = deserializeJson(jsonBuffer, https.getStream());
    if (deserialError) {
        Serial.print(F("JSON error : deserializeJson() failed with code "));
        Serial.println(deserialError.c_str());
        strcpy(error_, "JSON deserialization error");
        return false;
    }
    // Disconnect
    https.end();

    return true;
}

bool HttpBuyTransaction::sendForBalance(char* badge, unsigned long time)
{
    char timeString[16];
    
    sprintf(timeString, "%lu", time);
    Serial.println(timeString);

    HashBuilder hashBuilder;
    hashBuilder.print(badge);
    hashBuilder.print(timeString);
    
    JsonObject json = jsonBuffer.to<JsonObject>();
    json["Tid"] = TERMINAL_ID;
    json["Badge"] = badge;
    json["Hash"] = hashBuilder.getHash();
    json["Time"] = (const char*)timeString;
    serializeJson(json, buffer, sizeof(buffer));

    Serial.print(F("JSON request: "));
    Serial.println(buffer);

    https.useHTTP10(true);

    char request[32];
    snprintf(request, 32, "%s%s", API_PATH, "/balance");
    https.begin(*client_, SERVER_NAME, SERVER_PORT, request, true);
    https.addHeader("Content-Type", "application/json");
    int ret = https.POST(buffer);

    if(ret != HTTP_CODE_OK) {
      Serial.print(F("/balance returned error ("));
      Serial.print(ret);
      Serial.print(F(") : "));
      Serial.println(HTTPClient::errorToString(ret));
      return false;
    }
    
    // Read response
    DeserializationError deserialError = deserializeJson(jsonBuffer, https.getStream());
    if (deserialError) {
        Serial.print(F("JSON error : deserializeJson() failed with code "));
        Serial.println(deserialError.c_str());
        strcpy(error_, "JSON deserialization error");
        return false;
    }
    Serial.print(F("/balance reply json buffer usage : "));
    Serial.println(jsonBuffer.memoryUsage());

    // Disconnect
    https.end();

    return true;
}

bool HttpBuyTransaction::add(char* badge, char* barcodeString, char* itemCount, unsigned long time)
{
    char timeString[16];
    
    //snprintf(productString, sizeof(productString), "%d", product);
    sprintf(timeString, "%lu", time);
    Serial.println(timeString);

    HashBuilder hashBuilder;
    hashBuilder.print(badge);
    hashBuilder.print(barcodeString);
    hashBuilder.print(timeString);
    
    JsonObject json = jsonBuffer.to<JsonObject>();
    json["Tid"] = TERMINAL_ID;
    json["Badge"] = badge;
    json["Hash"] = hashBuilder.getHash();
    json["Barcode"] = barcodeString;
    json["Item_count"] = itemCount;
    json["Time"] = (const char*)timeString;
    serializeJson(json, buffer, sizeof(buffer));

    Serial.print("/add json buffer usage : ");
    Serial.println(jsonBuffer.memoryUsage());

    https.useHTTP10(true); // needed to deserialize stream

    char request[32];
    snprintf(request, 32, "%s%s", API_PATH, "/add");
    https.begin(*client_, SERVER_NAME, SERVER_PORT, request, true);
    https.addHeader("Content-Type", "application/json");
    int ret = https.POST(buffer);
    
    if(ret != HTTP_CODE_OK) {
      Serial.print(F("/add returned error ("));
      Serial.print(ret);
      Serial.print(F(") : "));
      Serial.println(HTTPClient::errorToString(ret));
      return false;
    }

    // Read response
    DeserializationError deserialError = deserializeJson(jsonBuffer, https.getStream());
    if (deserialError) {
        Serial.print(F("JSON error : deserializeJson() failed with code "));
        Serial.println(deserialError.c_str());
        strcpy(error_, "JSON deserialization error");
        return false;
    }
    Serial.println(jsonBuffer.memoryUsage());
    //const String& reply = http.getString();
    //strncpy(buffer, reply.c_str(), sizeof(buffer));
    //buffer[sizeof(buffer)-1] = '\0';

    // Disconnect
    https.end();

    return true;
}

bool HttpBuyTransaction::parse()
{
    const char* melody = jsonBuffer["Melody"];
    if (melody == NULL) {Serial.println(F("No melody sent")); strcpy(error_, "No melody sent"); return false;}
    strcpy(melody_, melody);

    const char* hash = jsonBuffer["Hash"];
    if (hash == NULL) {Serial.println(F("No hash sent")); strcpy(error_, "No hash sent"); return false;}
    strcpy(hash_, hash);
    
    time_ = jsonBuffer["Time"];
    if (time_ == 0) {Serial.println(F("No time sent")); strcpy(error_, "No time sent"); return false;}

    JsonArray messageArray = jsonBuffer["Message"];
    if (!messageArray) { Serial.println(F("No message sent")); strcpy(error_, "No message sent"); return false;}
    strcpy(messages_[0], messageArray[0]);
    strcpy(messages_[1], messageArray[1]);

    if(jsonBuffer.containsKey("ItemPrice")) {
      itemPrice_ = jsonBuffer["ItemPrice"];
      snprintf(itemPriceStr_, sizeof(itemPriceStr_), "%2d.%2d", itemPrice_/100, itemPrice_-((itemPrice_/100)*100));
      itemPriceStr_[sizeof(itemPriceStr_)-1] = '\0';
    }
    else {
      itemPrice_ = 0;
      itemPriceStr_[0] = '\0';
    }
    
    return true;
}

bool HttpBuyTransaction::validate()
{
    char timeStr[16];
    HashBuilder hashBuilder;
    hashBuilder.print(melody_);
    hashBuilder.print(messages_[0]);
    hashBuilder.print(messages_[1]);
    hashBuilder.print(ltoa(time_, timeStr, 10));

    if(strcasecmp(hash_, hashBuilder.getHash()) == 0)
      return true;
    else
    {
      Serial.print(F("Hash incorrect received: "));
      Serial.print(hash_); Serial.print(F(" expecting ")); Serial.println(hashBuilder.getHash());
      Serial.print(F("Time: ")); Serial.println(time_);
      
      strcpy(error_, "Hash incorrect");
      return false;
    }
}
