Drinks RFID Terminal PTL version
====================
The main differences between the original version and this is that we are using:
* MRFC522 for the RFID reader
* A rotary encoder for user input
* Added a magnetic lock on the fridge
* A Python [server](https://github.com/PostTenebrasLab/DrinkingBuddyServer) (original was in .NET) 
* Allow cash payments with coin acceptor (to be implemented)
* Admin and terminal through Blynk
* Storing IP settings in EEPROM

For the original version see:
An RFID client for the [drinks-server](https://github.com/drinks-wallet/drinks-server), designed to run on an Arduino Ethernet.

See http://blog.benoitblanchon.fr/rfid-payment-terminal/

Requirements
------------

* [Arduino SipHash library](http://www.forward.com.au/pfod/SipHashLibrary/)
* [Arduino JSON library](https://github.com/bblanchon/ArduinoJsonParser)
* [Blynk](http://www.blynk.cc/getting-started/)

WiKi
------------
[Drinking Buddy on PTL's website](https://www.posttenebraslab.ch/wiki/projects/ptl_payment_system)
