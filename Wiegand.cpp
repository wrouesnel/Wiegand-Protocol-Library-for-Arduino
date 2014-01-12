#include "Wiegand.h"

uint8_t		WIEGAND::_InterfaceCount=0;
WiegandInterface* WIEGAND::_WiegandInterfaces = NULL;

WIEGAND::WIEGAND()
{
}

unsigned long WIEGAND::getCode(byte InterfaceNum)
{
	// Interface num should be the nth linked-list item of Wiegand data.
    WiegandInterface* cur = _WiegandInterfaces;
    while (InterfaceNum > 0)
    {
        cur = cur->pNext;
        InterfaceNum--;
    }
    
	return cur->code;
}

int WIEGAND::getWiegandType(byte InterfaceNum)
{
	// Interface num should be the nth linked-list item of Wiegand data.
    WiegandInterface* cur = _WiegandInterfaces;
    while (InterfaceNum > 0)
    {
        cur = cur->pNext;
        InterfaceNum--;
    }
    
	return cur->wiegandType;
}

bool WIEGAND::available(byte InterfaceNum)
{
	return DoWiegandConversion(InterfaceNum);
}

void WIEGAND::begin(byte D0Pin, byte D1Pin)
{
    // Allocate a new interface struct
    WiegandInterface* newWiegand = (WiegandInterface*)malloc(sizeof(WiegandInterface));
    WiegandInterface* lastWiegand = NULL;

    // Initialize
    newWiegand->D0Pin = D0Pin;
    newWiegand->D1Pin = D1Pin;
    newWiegand->lastWiegand = 0;
	newWiegand->cardTempHigh = 0;
	newWiegand->cardTemp = 0;
	newWiegand->code = 0;
	newWiegand->wiegandType = 0;
	newWiegand->bitCount = 0;
	newWiegand->sysTick=millis();
    newWiegand->pNext = NULL;

    // Store a reference to the interface
    if (_WiegandInterfaces == NULL)
    {
        _WiegandInterfaces = newWiegand;
    }
    else
    {
        lastWiegand = _WiegandInterfaces;
        while (lastWiegand->pNext != NULL)
        {
            lastWiegand = lastWiegand->pNext;
        }
        lastWiegand->pNext = newWiegand;
    }

    // Configure new pins
	pinMode(D0Pin, INPUT);					// Set D0 pin as input
	digitalWrite(D0Pin, HIGH);				// Enable interal pull-up
	pinMode(D1Pin, INPUT);					// Set D1 pin as input
	digitalWrite(D1Pin, HIGH);				// Enable internal pull-up

	// Attach PinChangeInterrupt handlers for new pings.
	PCintPort::attachInterrupt(D0Pin, &WIEGAND::ReadD0, FALLING); // PC interrupt - high to low
	PCintPort::attachInterrupt(D1Pin, &WIEGAND::ReadD1, FALLING); // PC interrupt - high to low
	
	// Increment interface count
	_InterfaceCount++;
}

void WIEGAND::ReadD0 ()
{
    // Invoked by PinChangeInt - get the D0 pin which invoked us.
    WiegandInterface* curInterface = _WiegandInterfaces;
    while (curInterface != NULL)
    {
        if (curInterface->D0Pin == PCintPort::arduinoPin)
        {
            break;
        }
        curInterface = curInterface->pNext;
    }

    // Somehow we were invoked for a pin we don't watch.
    if (curInterface == NULL)
    {
        return;
    }

	curInterface->bitCount++;				// Increament bit count for Interrupt connected to D0
	if (curInterface->bitCount>31)			// If bit count more than 31, process high bits
	{
		curInterface->cardTempHigh |= ((0x80000000 & curInterface->cardTemp)>>31);	//	shift value to high bits
		curInterface->cardTempHigh <<= 1;
		curInterface->cardTemp <<=1;
	}
	else
	{
		curInterface->cardTemp <<= 1;		// D0 represent binary 0, so just left shift card data
	}
	curInterface->lastWiegand = curInterface->sysTick;	// Keep track of last wiegand bit received
}

void WIEGAND::ReadD1()
{
    // Invoked by PinChangeInt - get the D1 pin which invoked us.
    WiegandInterface* curInterface = _WiegandInterfaces;
    while (curInterface != NULL)
    {
        if (curInterface->D1Pin == PCintPort::arduinoPin)
        {
            break;
        }
        curInterface = curInterface->pNext;
    }

    // Somehow we were invoked for a pin we don't watch.
    if (curInterface == NULL)
    {
        return;
    }

	curInterface->bitCount ++;				// Increment bit count for Interrupt connected to D1
	if (curInterface->bitCount>31)			// If bit count more than 31, process high bits
	{
		curInterface->cardTempHigh |= ((0x80000000 & curInterface->cardTemp)>>31);	// shift value to high bits
		curInterface->cardTempHigh <<= 1;
		curInterface->cardTemp |= 1;
		curInterface->cardTemp <<=1;
	}
	else
	{
		curInterface->cardTemp |= 1;			// D1 represent binary 1, so OR card data with 1 then
		curInterface->cardTemp <<= 1;		// left shift card data
	}
	curInterface->lastWiegand = curInterface->sysTick;	// Keep track of last wiegand bit received
}

unsigned long WIEGAND::GetCardId (unsigned long *codehigh, unsigned long *codelow, char bitlength)
{
	unsigned long cardID=0;

	if (bitlength==26)								// EM tag
		cardID = (*codelow & 0x1FFFFFE) >>1;

	if (bitlength==34)								// Mifare
	{
		*codehigh = *codehigh & 0x03;				// only need the 2 LSB of the codehigh
		*codehigh <<= 30;							// shift 2 LSB to MSB
		*codelow >>=1;
		cardID = *codehigh | *codelow;
	}
	return cardID;
}

bool WIEGAND::DoWiegandConversion (byte InterfaceNum)
{
	// Interface count is 0-aligned
	if (InterfaceNum >= _InterfaceCount)
	{
		return false;
	}
	
	// Find the right interface struct
    WiegandInterface* cur = _WiegandInterfaces;
    while (InterfaceNum > 0)
    {
        cur = cur->pNext;
        InterfaceNum--;
    }

	unsigned long cardID;

	cur->sysTick=millis();
	if ((cur->sysTick - cur->lastWiegand) > 25)								// if no more signal coming through after 25ms
	{
		if ((cur->bitCount==26) || (cur->bitCount==34) || (cur->bitCount==8)) 	// bitCount for keypress=8, Wiegand 26=26, Wiegand 34=34
		{
			cur->cardTemp >>= 1;			// shift right 1 bit to get back the real value - interrupt done 1 left shift in advance
			if (cur->bitCount>32)			// bit count more than 32 bits, shift high bits right to make adjustment
				cur->cardTempHigh >>= 1;

			if((cur->bitCount==26) || (cur->bitCount==34))		// wiegand 26 or wiegand 34
			{
				cardID = GetCardId (&cur->cardTempHigh, &cur->cardTemp, cur->bitCount);
				cur->wiegandType=cur->bitCount;
				cur->bitCount=0;
				cur->cardTemp=0;
				cur->cardTempHigh=0;
				cur->code=cardID;
				return true;
			}
			else if (cur->bitCount==8)		// keypress wiegand
			{
				// 8-bit Wiegand keyboard data, high nibble is the "NOT" of low nibble
				// eg if key 1 pressed, data=E1 in binary 11100001 , high nibble=1110 , low nibble = 0001
				char highNibble = (cur->cardTemp & 0xf0) >>4;
				char lowNibble = (cur->cardTemp & 0x0f);
				cur->wiegandType=cur->bitCount;
				cur->bitCount=0;
				cur->cardTemp=0;
				cur->cardTempHigh=0;

				if (lowNibble == (~highNibble & 0x0f))		// check if low nibble matches the "NOT" of high nibble.
				{
					if (lowNibble==0x0b)					// ENT pressed
					{
						cur->code=0x0d;
					}
					else if (lowNibble==0x0a)				// ESC pressed
					{
						cur->code=0x1b;
					}
					else
					{
						cur->code=(int)lowNibble;				// 0 - 9 keys
					}
					return true;
				}
			}
		}
		else
		{
			// well time over 25 ms and bitCount !=8 , !=26, !=34 , must be noise or nothing then.
			cur->lastWiegand=cur->sysTick;
			cur->bitCount=0;
			cur->cardTemp=0;
			cur->cardTempHigh=0;
			return false;
		}
	}
	else
		return false;
}
