#include "Wiegand.h"
#include "PinChangeInt.h"

Wiegand* Wiegand::device [] = {NULL};

Wiegand::Wiegand (byte id, byte D0, byte D1)
{
	this->id = id;
	this->D0 = D0;
	this->D1 = D1;
	this->code = 0;
	this->type = 0;
	this->bits = 0;
	this->tick = 0;
	this->codelo = 0;
	this->codehi = 0;
	Wiegand::device [id] = this;
}

void Wiegand::begin ()
{
//	INITIALIZE PINS
	pinMode (this->D0, INPUT);
	pinMode (this->D1, INPUT);

//	ENUMERATE ISR
	typedef void (*handler) (void);
	handler intd0 [] = {Int0D0, Int1D0, Int2D0};
	handler intd1 [] = {Int0D1, Int1D1, Int2D1};

//	ATTACH INTERRUPTS
	PCintPort::attachInterrupt (this->D0, intd0 [this->id], RISING);
	PCintPort::attachInterrupt (this->D1, intd1 [this->id], RISING);
}

bool Wiegand::available ()
{
    noInterrupts ();
	bool ret = this->convert ();
	interrupts ();
	return ret;
}

ulng Wiegand::getCode ()
{
	return this->code;
}

byte Wiegand::getType ()
{
	return this->type;
}

bool Wiegand::convert ()
{
//	GET CURRENT TIME
	unsigned long tick = millis ();
	char translateEnterEscapeKeyPress (char);

//	IF WITHIN 25ms
	if (tick - this->tick <= 25)
		return false;

//	IF NO ENOUGH BITS READ => NOISE
	if ((this->bits != 26) && (this->bits != 34) && (this->bits != 8) && (this->bits != 4))
	{
		this->tick = tick;
		this->bits = 0;
		this->codelo = 0;
		this->codehi = 0;
		return false;
	}
	
//	shift right 1 bit to get back the real value - interrupt done 1 left shift in advance
	this->codelo >>= 1;

//	bit count more than 32 bits, shift high bits right to make adjustment
	if (this->bits > 32)
		this->codehi >>= 1;

//	WIEGAND 26 / 34
	if ((this->bits == 26) || (this->bits == 34))
	{
		this->code = this->calculate ();
		this->type = this->bits;
		this->bits = 0;
		this->codelo = 0;
		this->codehi = 0;
		return true;
	}
	
//	8-BIT WIEGAND KEYBOARD DATA, HighNibble = !LowNibble
	if (this->bits == 8)
	{
		byte hi = (this->codelo & 0xf0) >> 4;
		byte lo = (this->codelo & 0x0f);
		this->type = 8;
		this->bits = 0;
		this->codelo = 0;
		this->codehi = 0;

	//	VALIDATE
		if (lo == ~hi & 0x0f)
		{
			this->code = (int) translateEnterEscapeKeyPress (lo);
			return true;
		}
		
	//	ERROR
		return false;
	}

//	4 BIT WIEGAND => NO INTEGRITY CHECK
	if (this->bits == 4)
	{
		this->code = (int) translateEnterEscapeKeyPress (this->codelo & 0x0000000F);
		this->type = 4;
		this->bits = 0;
		this->codelo = 0;
		this->codehi = 0;
		return true;
	}
}

long Wiegand::calculate ()
{
//	EM tag
	if (this->bits == 26)
		return (this->codelo & 0x1FFFFFE) >>1;

//	Mifare
	if (this->bits == 34)
	{
	//	only need the 2 LSB of the codehigh
		this->codehi = this->codehi & 0x03;
	//	shift 2 LSB to MSB
		this->codehi <<= 30;
		this->codelo >>= 1;
		return (this->codehi | this->codelo);
	}
	return 0;
}

void Wiegand::readWiegandD0 ()
{
	this->tick = millis ();
	if (++ this->bits > 31)
	{
		this->codehi |= ((0x80000000 & this->codelo)>>31);
		this->codehi <<= 1;
		this->codelo <<= 1;
	}
	else
	{
		this->codelo <<= 1;
	}
}

void Wiegand::readWiegandD1 ()
{
	this->tick = millis ();
	if (++ this->bits > 31)
	{
		this->codehi |= ((0x80000000 & this->codelo)>>31);
		this->codehi <<= 1;
		this->codelo |= 1;
		this->codelo <<= 1;
	}
	else
	{
		this->codelo |= 1;
		this->codelo <<= 1;
	}
}

void Wiegand::Int0D0 ()
{
	Wiegand::device [0]->readWiegandD0 ();
}

void Wiegand::Int0D1 ()
{
	Wiegand::device [0]->readWiegandD1 ();
}

void Wiegand::Int1D0 ()
{
	Wiegand::device [1]->readWiegandD0 ();
}

void Wiegand::Int1D1 ()
{
	Wiegand::device [1]->readWiegandD1 ();
}

void Wiegand::Int2D0 ()
{
	Wiegand::device [2]->readWiegandD0 ();
}

void Wiegand::Int2D1 ()
{
	Wiegand::device [2]->readWiegandD1 ();
}

char translateEnterEscapeKeyPress (char originalKeyPress)
{
	switch (originalKeyPress)
	{
	case 0x0b:			//	11 or * key
		return 0x0d;	//	13 or ASCII ENTER
	case 0x0a:			//	10 or # key
		return 0x1b;	//	27 or ASCII ESCAPE
	}
	return originalKeyPress;
}
