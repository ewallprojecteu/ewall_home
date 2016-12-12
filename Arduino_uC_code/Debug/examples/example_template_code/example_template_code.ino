/*
 * AUTHOR: Loredan E. Bucur
 * LAST_EDIT: 2014
 *
 */

//DEFINE
#define DEBUG 1

//INCLUDE
#include <Debug.h>

//CONSTANTS
const unsigned long int_task1 = 1000;

struct
{//members are static because we don't want them to be instance-specific
	static const uint8_t led = 13;
} Pins;

//GLOBALS
unsigned long last_task1;

void setup()
{
	DB_begin(115200);
	DB_printsln("\nSETUP");

	//declare output/input_pullup pins if any
	pinMode(Pins.led, OUTPUT);
}

void loop()
{
	unsigned long current_time;

	//TASK 1
	current_time = millis();
	if (current_time - last_task1 > int_task1)
	{
		
		last_task1 = current_time;
	}
}

void serialEvent()
{
	char c = Serial.read();
	DB::serialEvent(c);
}