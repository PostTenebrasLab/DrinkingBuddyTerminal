#include "Barcode.h"
#include <Arduino.h>
#include <SoftwareSerial.h>


Barcode::Barcode(int rx, int tx, int en) : mySerial(rx, tx)
{  
  enable_pin = en;
  pinMode(enable_pin, OUTPUT);
  digitalWrite(enable_pin, LOW);
}
void Barcode::begin()
{
  mySerial.begin(9600);
  mySerial.flush();
}
void Barcode::enable()
{
  digitalWrite(enable_pin, HIGH);
  delay(250);
  mySerial.flush();
}
void Barcode::disable()
{
  digitalWrite(enable_pin, LOW);  
}

void Barcode::flush()
{
  while(mySerial.available())
    mySerial.read();
}

int Barcode::available()
{
  //int temp = mySerial.available();
  //Serial.println(temp);
  return mySerial.available();
}

int Barcode::get(char* barcode, int size)
{
  
  if(!mySerial.available())
      return -1;

    int index = 0; 
    char inChar=-1; 
    while (mySerial.available() > 0)
    {
        if(index < size-1) 
        {
            inChar = mySerial.read(); // Read a character
            barcode[index++] = inChar; // Store it
            //barcode[index] = '\0'; // Null terminate the string
        }
        else {
          flush();
          break;
        }
    }
    delay(500); //sometimes the text is sent over 2 lines because of a delay in serial connection
    while (mySerial.available() > 0)
    {
        if(index < size-1) 
        {
            inChar = mySerial.read(); // Read a character
            barcode[index++] = inChar; // Store it
            //barcode[index] = '\0'; // Null terminate the string
        }
        else {
          flush();
          break;
        }
    }
    return index;
}
