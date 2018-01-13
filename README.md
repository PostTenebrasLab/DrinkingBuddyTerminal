Drinks RFID Terminal PTL version
=======
What is "DrinkingBuddy"?
====================
An RFID payment system used to make small payments in the PostTenebrasLab hackerspace.
The system is composed of an Arduino based terminal with an LCD, an RFID reader and Ethernet shield that communicates with a Python server to get the list of items that can be purchased and check user's balance.

"DrinkingBuddy" is a fork of DrinksRfidTerminal developed by Benoît Blanchon (http://blog.benoitblanchon.fr/rfid-payment-terminal/)

What are the differences between this and the fork?
====================
* We use MFRC522 RFID module
* Add a rotary encoder with RGB LED instead of buttons
* Add control for a magnetic lock (make sure the fridge door cannot open unless a transaction has been made)
* The "buy" workflow has changed in the GUI
	1- Present your RFID badge ---> User balance is displayed
	2- For several seconds (as defined in the config or as long as you're navigating the menu) the user is "logged in" and can make a buy transaction
	3- Click the button on the rotary encoder and a "buy" transaction will be made
	4- If you want to buy more things you have to put the badge again
* Using an Arduino MEGA (more memory, more pins)

DrinkingBuddy server
====================
Here is a link to the server API's implemented in Python (using Flask and SQLAlchemy)
https://github.com/PostTenebrasLab/DrinksBuddyServer

Drinks RFID Terminal
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
