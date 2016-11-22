#include "Wiegand.h"

Wiegand wg1 (0, 2,3);
Wiegand wg2 (1, 4, 5);
Wiegand wg3 (1, 6, 7);

void setup ()
{
	Serial.begin (9600);
	wg1.begin ();
	wg2.begin ();
	wg3.begin ();
}

void loop ()
{
	if (wg1.available ())
	{
		Serial.print ("Wiegand 1, Type W");
		Serial.print (wg1.getType ());
		Serial.print (", Code ");
		Serial.print (wg1.getCode (), DEC);
		Serial.println (" ");
	}
	if (wg2.available ())
	{
		Serial.print ("Wiegand 2, Type W");
		Serial.print (wg2.getType ());
		Serial.print (", Code ");
		Serial.print (wg2.getCode (), DEC);
		Serial.println (" ");
	}
	if (wg3.available ())
	{
		Serial.print ("Wiegand 3, Type W");
		Serial.print (wg3.getType ());
		Serial.print (", Code ");
		Serial.print (wg3.getCode (), DEC);
		Serial.println (" ");
	}
}

