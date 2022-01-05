#include <SoftwareSerial.h>

#ifndef _BARCODE_H
#define _BARCODE_H

class Barcode
{
public:

    Barcode(int rx,int tx, int en);
    int get(char* barcode, int size);
    int available();
    void begin();
    void flush();
    void enable();
    void disable();

private:

      SoftwareSerial mySerial;
      int enable_pin;
      
};

#endif
