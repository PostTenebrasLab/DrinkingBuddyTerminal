#include <Arduino.h>

#include "Pins.h"

#ifndef _ENCODER_H
#define _ENCODER_H

class Encoder
{
public:
  
    void begin();

    bool rightPressed()
    {
        if (pressedButton != RIGHT) return false;
        pressedButton = NONE;
        return true;
    }

    bool leftPressed()
    {
        if (pressedButton != LEFT) return false;
        pressedButton = NONE;
        return true;
    }

    bool btnPressed()
    {
        if (pressedButton != PRESS) return false;
        pressedButton = NONE;
        return true;
    }

    void ledOn(int RGB)
    {
      if(RGB = 1) //RED
        digitalWrite(ledRED, LOW);
      if(RGB = 2) //GREEN
        digitalWrite(ledGREEN, LOW);
      if(RGB = 3) //BLUE
        digitalWrite(ledBLUE, LOW);      
    }

    void ledChange(bool RED, bool GREEN, bool BLUE)
    {
      if(RED)
        digitalWrite(ledRED, LOW);
      else
        digitalWrite(ledRED, HIGH);
      if(GREEN)
        digitalWrite(ledGREEN, LOW);
      else
        digitalWrite(ledGREEN, HIGH);
      if(BLUE)
        digitalWrite(ledBLUE, LOW);
      else
        digitalWrite(ledBLUE, HIGH);
    }

    void ledOff(int RGB)
    {
      if(RGB = 1) //RED
        digitalWrite(ledRED, HIGH);
      if(RGB = 2) //GREEN
        digitalWrite(ledGREEN, HIGH);
      if(RGB = 3) //BLUE
        digitalWrite(ledBLUE, HIGH);      
    }

    void ledAllOff()
    {      
        digitalWrite(ledRED, HIGH);
        digitalWrite(ledGREEN, HIGH);
        digitalWrite(ledBLUE, HIGH);      
    }


private:

    static bool checkBounce();
    static bool checkBounceBtn();
    static void doEncoder();
    static void doBtn();

    static const unsigned long DEBOUNCE_PERIOD = 100;
    static const byte NONE = 0;
    static const byte LEFT = 1;
    static const byte RIGHT = 2;
    static const byte PRESS = 3;

    static byte pressedButton;
    static unsigned long blindUntil;
    static unsigned long blindUntilBtn;

    static unsigned int encoder0Pos;
    static unsigned int encoder0PosOld;
    static unsigned long lastCall;

    static boolean fired;
    static boolean up;
};

#endif
