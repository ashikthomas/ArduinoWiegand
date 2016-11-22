#ifndef WIEGAND_H
#define WIEGAND_H

#include <Arduino.h>

typedef unsigned long ulng;

class Wiegand
{
public:
	Wiegand (byte id, byte D0, byte D1);

public:
	void begin ();
	bool available ();

public:
	ulng getCode ();
	byte getType ();

private:
	byte id;
	byte D0, D1;
	volatile byte type, bits;
	volatile unsigned long code, tick;
	volatile unsigned long codelo, codehi;

private:
	bool convert ();
	long calculate ();
	void readWiegandD0 ();
	void readWiegandD1 ();

private:
	static void Int0D0 ();
	static void Int0D1 ();
	static void Int1D0 ();
	static void Int1D1 ();
	static void Int2D0 ();
	static void Int2D1 ();
	static Wiegand *device [3];
};

#endif
