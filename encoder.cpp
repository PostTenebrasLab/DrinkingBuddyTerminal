/* 
 * "Drinks" RFID Terminal 
 * Buy sodas with your company badge! 
 * 
 * Benoit Blanchon 2014 - MIT License 
 * https://github.com/bblanchon/DrinksRfidTerminal 
 */ 

#include <Arduino.h> 

#include "encoder.h" 
#include "Pins.h"

byte Encoder::pressedButton = Encoder::NONE; 
unsigned long Encoder::blindUntil = 0; 
unsigned long Encoder::blindUntilBtn = 0; 

unsigned int Encoder::encoder0Pos = 0; 
unsigned int Encoder::encoder0PosOld = 0; 
boolean Encoder::fired=false; 
boolean Encoder::up=false; 

//unsigned long Encoder::lastCall = 0; 

bool Encoder::checkBounce() 
{ 
    unsigned long now = millis(); 
    bool ok = now > blindUntil; 
    blindUntil = now + DEBOUNCE_PERIOD; 
    return ok; 
} 

bool Encoder::checkBounceBtn() 
{ 
    unsigned long now = millis(); 
    bool ok = now > blindUntilBtn; 
    blindUntilBtn = now + (DEBOUNCE_PERIOD*2); 
    return ok; 
} 

void Encoder::doBtn() 
{ 
  if(checkBounceBtn ) 
    pressedButton = PRESS;     
} 

void Encoder::doEncoder() 
{ 
        
  if (digitalRead (encoder0PinA)) 
    up = digitalRead (encoder0PinB); 
  else 
    up = !digitalRead (encoder0PinB); 
  fired = true;  
  if (up)
  {
  encoder0Pos++;
  pressedButton = LEFT;
  }
  else 
  {
    encoder0Pos--; 
    pressedButton = RIGHT;
  }
  
} 

  
void Encoder::begin() 
{ 
  pinMode(encoder0PinA, INPUT); 
  digitalWrite(encoder0PinA, HIGH);       // turn on pullup resistor 
  pinMode(encoder0PinB, INPUT); 
  digitalWrite(encoder0PinB, HIGH);       // turn on pullup resistor 
  pinMode(encoder0Press, INPUT); 
  //digitalWrite(encoder0Press, HIGH);     // turn on pullup resistor 
  
  pinMode(ledRED, OUTPUT); 
  digitalWrite(ledRED, HIGH); 

  pinMode(ledGREEN, OUTPUT); 
  digitalWrite(ledGREEN, HIGH); 

  pinMode(ledBLUE, OUTPUT); 
  digitalWrite(ledBLUE, HIGH); 
  
  attachInterrupt(digitalPinToInterrupt(encoder0PinA), doEncoder, CHANGE);  // UNO encoder pin on interrupt 0 - pin 2 
  attachInterrupt(digitalPinToInterrupt(encoder0Press), doBtn, FALLING );   // UNO encoder pin on interrupt 0 - pin 2 
} 

