# Personal Development Fork
This is a fork of the library for Wiegand readers found [here] (https://github.com/monkeyboard/Wiegand-Protocol-Library-for-Arduino)

I have used this library for a personal security project, and needed to add some ease-of-use functionality to it - those commits to the original project are found here.

The most important feature added is support for using multiple pins with interrupt masking to allow reading from multiple readers (which need to be identified somehow so we know which devices to activate).

# Wiegand 26 and Wiegand 34 library for Arduino

The Wiegand interface is a de facto standard commonly used to connect a card reader or keypad to an electronic entry system. Wiegand interface has the ability to transmit signal over long distance with a simple 3 wires connection. This library uses interrupt pins from Arduino to read the pulses from Wiegand interface and return the code and type of the Wiegand.

## Requirements

The following are needed 

* [Arduino] (http://www.arduino.cc) Any ATMEGA328 compatible board should work.
* [Wiegand RFID Reader] (http://www.monkeyboard.org/products/85-developmentboard/84-rfid-wiegand-protocol-development-kit) The code was written for this reader however customers reported working with [HID] (http://www.hidglobal.com/products/cards-and-credentials) compatible readers.
	* I used very cheap $10 eBay readers, and they worked flawlessly.

## Installation 

Create a folder named Wiegand in Arduino's libraries folder.  You will have the following folder structure:

	cd arduino/libraries
	mkdir Wiegand
	cd Wiegand
	git clone https://github.com/monkeyboard/Wiegand-Protocol-Library-for-Arduino.git .

Note: in forking this project I instead pulled the library into my ~/src folder, and symlinked it to the libraries directory as "Wiegand" under my sketchbook folder.

## Arduino Sketch

![alt text](http://www.monkeyboard.org/images/tutorials/wiegand/wiegand_arduino.png "RFID Reader to Arduino connection diagram")


Execute Arduino IDE, select Example-->Wiegand-->WiegandTest

### Example
<pre><code>
#include &lt;Wiegand.h&gt;

WIEGAND wg;

void setup() {
	Serial.begin(9600);  
	wg.begin();
}

void loop() {
	if(wg.available())
	{
		Serial.print("Wiegand HEX = ");
		Serial.print(wg.getCode(),HEX);
		Serial.print(", DECIMAL = ");
		Serial.print(wg.getCode());
		Serial.print(", Type W");
		Serial.println(wg.getWiegandType());    
	}
}
</code></pre>

Updated by Will Rouesnel
Original by JP Liew

Project home: http://www.monkeyboard.org/tutorials/82-protocol/24-wiegand-converter

*This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.*

*This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.*
