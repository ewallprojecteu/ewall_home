/*
 * AUTHOR: Loredan E. Bucur
 * LAST_EDIT: 2014
 *
 */

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h" //for Serial and other stuff
#else
	#include "WProgram.h"
#endif

#include "Debug.h"

unsigned DB::LOOP_COUNT; //global -> here happens the definition: the var is init with 0;

void DB::serialEvent(char c)
{
	//buffer is 64 Bytes long
	//wait for commands
	//can't begin() serial here
	if (c == 'm')
	{
		Serial.print(F("\nFree bytes in SRAM: "));
		Serial.println(freeRam());
	}
}

int DB::freeRam() {
  extern unsigned int __heap_start, * __brkval;
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
