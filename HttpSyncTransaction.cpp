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

bool HttpSyncTransaction::send()
{
    buffer[0] = 0;

    return http.query("GET " API_PATH "/sync", buffer, buffer_size-1);
}

bool HttpSyncTransaction::parse()
{
    //StaticJsonBuffer<JSON_OBJECT_SIZE(10)+JSON_ARRAY_SIZE(Catalog::MAX_PRODUCT_COUNT)> jsonBuffer;
    StaticJsonBuffer<512> jsonBuffer; //buffer size is too small if we have more items...512 seems to be enough for now
    Serial.println("Sync parse: root.success");
    JsonObject& root = jsonBuffer.parseObject(buffer);
    if (!root.success()) return false;


    Serial.println("Sync parse: Header");
    header = root["Header"];
    if (header == NULL) return false;

    Serial.println("Sync parse: DBID");

    JsonArray& dbIDArray = root["DBID"];
    if (!dbIDArray.success()) return false;

    Serial.println("Sync parse: Products");
    JsonArray& productsArray = root["Products"];
    if (!productsArray.success()) return false;


    Serial.println("Sync parse: For loop");
    int count = productsArray.size();
    for (int i = 0; i < count; i++)
    {
        products[i] = productsArray[i];
        dbID[i] = dbIDArray[i];
    }    
    products[count] = NULL;
    Serial.print("Products count: "); Serial.println(productsArray.size());

    Serial.println("Sync parse: Time");
    time = root["Time"];
    if (time == NULL) return false;


    Serial.println("Sync parse: Hash");
    hash = root["Hash"];
    if (hash == NULL) return false;

    return true;
}

bool HttpSyncTransaction::validate()
{
    HashBuilder hashBuilder;

    hashBuilder.print(header);

    for (int i = 0; products[i] != NULL; i++)
        hashBuilder.print(products[i]);

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
