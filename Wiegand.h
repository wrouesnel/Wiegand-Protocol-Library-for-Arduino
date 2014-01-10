/*
	This modified version of the Wiegand library depends on the 
	PinChangeInterrupt library, which must be availabe in your sketchbook.

	It uses this library to monitor arbitrary pins for Wiegand data to do D0/D1
	detection.
*/

#ifndef _WIEGAND_H
#define _WIEGAND_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#ifndef PinChangeInt_h
	#define LIBCALL_PINCHANGEINT
	#include "../PinChangeInt/PinChangeInt.h"
#endif

// Struct to store Wiegand decode data in.
typedef struct {
	byte 			D0Pin;
	byte 			D1Pin;
	unsigned long 	cardTempHigh;
	unsigned long 	cardTemp;
	unsigned long 	lastWiegand;
	unsigned long 	sysTick;
	int				bitCount;	
	int				wiegandType;
	unsigned long	code;
	WiegandInterface* pNext;
} WiegandInterface;

class WIEGAND {

public:
	WIEGAND();
	void begin(byte D0Pin, byte D1Pin);	// Initialize a Wiegand instance
	bool available(byte InterfaceNum);
	unsigned long getCode();
	int getWiegandType();
	
private:
	static void ReadD0();
	static void ReadD1();
	static bool DoWiegandConversion (byte InterfaceNum);
	static unsigned long GetCardId (unsigned long *codehigh, unsigned long *codelow, char bitlength);
	
	static WiegandInterface* _WiegandInterfaces;
};

#endif
