/*
* "Drinks" RFID Terminal
* Buy sodas with your company badge!
*
* Benoit Blanchon 2014 - MIT License
* https://github.com/bblanchon/DrinksRfidTerminal
*/

#include "Configuration.h"

#ifndef _CATALOG_H
#define _CATALOG_H

class Catalog
{
public:

    Catalog()
    {
        productCount = 0;
    }

    void setProductCount(int count)
    {
        productCount = count;
    }

    int getProductCount()
    {
        return productCount;
    }

    void setProductDBID(int id, const char* s)
    {
        //dbid[id] = myDBid;
        strncpy(dbid[id], s, DBID_SIZE);
    }

    char* getProductDBID(int id)
    {
        return dbid[id];
    }

    char* getProduct(int id)
    {
        return products[id];
    }

    void setProduct(int id, const char* s)
    {
        strncpy(products[id], s, PRODUCT_NAME_SIZE);
    }

    char* getHeader()
    {
        return header;
    }

    void setHeader(const char* s)
    {
        strncpy(header, s, CATALOG_HEADER_SIZE);
    }

    static const int MAX_PRODUCT_COUNT = MAX_PRODUCTS;
    static const int PRODUCT_NAME_SIZE = 19;
    static const int DBID_SIZE = 3;
    static const int CATALOG_HEADER_SIZE = 21;

private:

    int productCount;
    char products[MAX_PRODUCT_COUNT][PRODUCT_NAME_SIZE];
    char header[CATALOG_HEADER_SIZE];
    char dbid[MAX_PRODUCT_COUNT][DBID_SIZE];
};

#endif
