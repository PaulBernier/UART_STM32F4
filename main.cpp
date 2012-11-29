/**
* Example of code using UART library
*/

#include "stm32f4xx.h"
#include "Uart.hpp"

int main(void) {

	Delay_Init();
	
	Uart<1> serial;
	serial.init(9600);

	uint32_t test=0;
	
	while (1) {
	
		//Get data
		if(serial.available()){
			serial.read(test);
			Delay(100);
		}
		
		//Print data
		serial.println("Hello world");
		serial.print("Hi (no return)");
	}

	return 0;
}


