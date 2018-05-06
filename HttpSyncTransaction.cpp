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
#include "HttpSyncTransaction.h"

//bool HttpSyncTransaction::send()
//{
//    buffer[0] = 0;
//
//    return http.query("GET " API_PATH "/sync", buffer, buffer_size-1);
//}

bool HttpSyncTransaction::send()
{
    //char terminalIdString[5];
    //int Tid = TERMINAL_ID;
    //snprintf(terminalIdString, sizeof(terminalIdString), "%d", Tid);
    //itoa(Tid,terminalIdString,10);
    
    StaticJsonBuffer<JSON_OBJECT_SIZE(1)> jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["Tid"] = "1";
    json.printTo(buffer, sizeof(buffer));

    return http.query("POST " API_PATH "/sync", buffer, sizeof(buffer));
  
}



bool HttpSyncTransaction::parse()
{
    //StaticJsonBuffer<JSON_OBJECT_SIZE(10)+JSON_ARRAY_SIZE(Catalog::MAX_PRODUCT_COUNT)> jsonBuffer;
    //StaticJsonBuffer<1024> jsonBuffer; //buffer size is too small if we have more items...512 seems to be enough for now
    DynamicJsonBuffer jsonBuffer(768);
    Serial.println("Sync parse: root.success");
    JsonObject& root = jsonBuffer.parseObject(buffer);
    if (!root.success()) return false;


    Serial.println("Sync parse: Header");
    header = root["Header"];
    if (header == NULL) return false;

    HashBuilder hashBuilder;
    hashBuilder.print(header);

    //Serial.println("Sync parse: DBID");

    //JsonArray& dbIDArray = root["DBID"];
    //if (!dbIDArray.success()) return false;

    Serial.println("Sync parse: Products");
    JsonArray& productsArray = root["Products"];
    if (!productsArray.success()) return false;


    Serial.println("Sync parse: For loop");
    int count = productsArray.size();
    Serial.print("Products count: "); Serial.println(count);

    for (int i = 0; i < count; i++)
    {
      products[i] = malloc(20);
    }
    products[count] = NULL;

    for (int i = 0; i < count; i++)
    {
        JsonArray& product = productsArray[i];
        snprintf(products[i], Catalog::MAX_PRODUCT_COUNT, "%s %s",product.get<const char*>(1),product.get<const char*>(2));
        //products[i] = productsArray[i][1];
        dbID[i] = product[0];
        hashBuilder.print(product[0]);
        hashBuilder.print(product[1]);
        hashBuilder.print(product[2]);
        
    }    

    Serial.println("Sync parse: Time");
    time = root["Time"].as<unsigned long>();
    if (time == NULL) return false;
    char timeStr[10];
    sprintf(timeStr, "%lu", time);

    Serial.println("Sync parse: Hash");
    hash = root["Hash"];
    if (hash == NULL) return false;

    hashBuilder.print(timeStr);

    const char* hash2 = hashBuilder.getHash();


    Serial.println("");
    Serial.print("'");
    Serial.print(hash);
    Serial.print("' == '");
    Serial.print(hash2);
    Serial.print("' -> ");
    Serial.println(!strcmp(hash, hash2));

    return !strcmp(hash, hash2);
}

bool HttpSyncTransaction::validate()
{
  /*
    HashBuilder hashBuilder;

    hashBuilder.print(header);
    DynamicJsonBuffer jsonBuffer(768);
    JsonObject& root = jsonBuffer.parseObject(buffer);
    if (!root.success()) return false;

    for (int i = 0; products[i] != NULL; i++)
    Serial.println(root.get<const char*>("Products"));
    hashBuilder.print(root.get<const char*>("Products"));

    //for (int i = 0; dbID[i] != NULL; i++)
        //hashBuilder.print(dbID[i]);

    hashBuilder.print(time);

    const char* hash2 = hashBuilder.getHash();


    Serial.println("");
    Serial.print("'");
    Serial.print(hash);
    Serial.print("' == '");
    Serial.print(hash2);
    Serial.print("' -> ");
    Serial.println(!strcmp(hash, hash2));

    return !strcmp(hash, hash2);
    */
    return true;
}

void HttpSyncTransaction::getCatalog(Catalog& catalog)
{
    catalog.setHeader(header);

    int i;
    for (i = 0; i < MAX_PRODUCTS; i++)
    {
        if (products[i] == NULL) break;
        catalog.setProduct(i, products[i]);
        catalog.setProductDBID(i, dbID[i]);
    }

    catalog.setProductCount(i);
}
