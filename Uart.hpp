/**
 * Uart.hpp
 * Handle UARTs (transmission/reception) for STM32F4
 *
 * @author Paul Bernier
 *
 * 28/11/2912
 * v1.0
 * To get the latest version of the lib, please go to : http://github.com/Absurdev/UART_STM32F4
 *
 *********************************************************
 * Pins :
 *
 * UART1: B6 (TX) B7 (RX)
 * UART2: A2 (TX) A3 (RX)
 * UART3: D8 (TX) D9 (RX)
 * UART4: C10 (TX) C11 (RX)
 * UART5: C12 (TX) D2 (RX)
 * UART6: C6 (TX) C7 (RX)
 *
 */

#ifndef UART_HPP
#define UART_HPP

#include "stm32f4xx.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/**
*Include this only if ltoa is not implemented in you standard library implementation
*/
#include "ltoa.c"

#define RX_BUFFER_SIZE 64

template<uint8_t USART_ID>
class Uart {
private:

	static USART_TypeDef* USARTx;

	/**
	 * Write functions : send raw strings
	 *
	 */

	template<class T>
	static inline void write(T val) {
		char buffer[10];
		ltoa(val, buffer, 10);
		write((const char *) buffer);
	}

	static inline void write(bool val) {
		(val) ? write("true") : write("false");
	}

	static inline void write(char* val) {
		for (uint16_t i = 0; i < strlen(val); i++) {
			send_char(val[i]);
		}
	}

	static inline void write(const char* val) {
		for (uint16_t i = 0; i < strlen(val); i++) {
			send_char(val[i]);
		}
	}

	static inline void write(float value, int places) {
		int digit;
		float tens = 0.1;
		int tenscount = 0;
		int i;
		float tempfloat = value;

		// make sure we round properly. this could use pow from <math.h>, but doesn't seem worth the import
		// if this rounding step isn't here, the value  54.321 prints as 54.3209
		// calculate rounding term d:   0.5/pow(10,places)
		float d = 0.5;
		if (value < 0)
			d *= -1.0;
		// divide by ten for each decimal place
		for (i = 0; i < places; i++)
			d /= 10.0;
		// this small addition, combined with truncation will round our values properly
		tempfloat += d;

		// first get value tens to be the large power of ten less than value
		// tenscount isn't necessary but it would be useful if you wanted to know after this how many chars the number will take
		if (value < 0)
			tempfloat *= -1.0;
		while ((tens * 10.0) <= tempfloat) {
			tens *= 10.0;
			tenscount += 1;
		}

		// write out the negative if needed
		if (value < 0)
			write("-");

		if (tenscount == 0)
			write(0);

		for (i = 0; i < tenscount; i++) {
			digit = (int) (tempfloat / tens);
			write(digit);
			tempfloat = tempfloat - ((float) digit * tens);
			tens /= 10.0;
		}

		// if no places after decimal, stop now and return
		if (places <= 0)
			return;

		// otherwise, write the point and continue on
		write(".");

		// now write out each decimal place by shifting digits one by one into the ones place and writing the truncated value
		for (i = 0; i < places; i++) {
			tempfloat *= 10.0;
			digit = (int) tempfloat;
			write(digit);
			// once written, subtract off that digit
			tempfloat = tempfloat - (float) digit;
		}
	}

	static inline void send_ln() {
		send_char('\r');
		send_char('\n');
	}

public:
	struct ring_buffer {

		ring_buffer() {
		}
		unsigned char buffer[RX_BUFFER_SIZE];
		int head;
		int tail;
	};
	static volatile ring_buffer rx_buffer_;

	enum {
		READ_TIMEOUT = 0, READ_SUCCESS = 1
	};

	/**
	 * Initialize the UART  : set pins, enable clocks, set uart, enable interrupt
	 *
	 */
	static inline void init(uint32_t baudrate) {
		GPIO_InitTypeDef GPIO_InitStruct;
		USART_InitTypeDef USART_InitStruct;
		NVIC_InitTypeDef NVIC_InitStructure;

		//General settings of pins TX/RX
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;

		switch (USART_ID) {
		case 1:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; // Pins B6 (TX) and B7 (RX)
			GPIO_Init(GPIOB, &GPIO_InitStruct);

			GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1);
			GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);

			NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
			break;
		case 2:
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3; // Pins A2 (TX) and A3 (RX)
			GPIO_Init(GPIOA, &GPIO_InitStruct);

			GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
			GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

			NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
			break;
		case 3:
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9; // Pins D8 (TX) and D9 (RX)
			GPIO_Init(GPIOD, &GPIO_InitStruct);

			GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3);
			GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART3);

			NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
			break;
		case 4:
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11; // Pins C10 (TX) and C11 (RX)
			GPIO_Init(GPIOC, &GPIO_InitStruct);

			GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_UART4);
			GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_UART4);

			NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
			break;
		case 5:
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12; // Pin C12 (TX)
			GPIO_Init(GPIOC, &GPIO_InitStruct);
			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2; // Pin D2 (RX)
			GPIO_Init(GPIOD, &GPIO_InitStruct);

			GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_UART5);
			GPIO_PinAFConfig(GPIOD, GPIO_PinSource2, GPIO_AF_UART5);

			NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
			break;
		case 6:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; // Pins C6 (TX) and C7 (RX)
			GPIO_Init(GPIOC, &GPIO_InitStruct);

			GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6);
			GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6);

			NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;
			break;
		}

		//UART setting
		USART_InitStruct.USART_BaudRate = baudrate;
		USART_InitStruct.USART_WordLength = USART_WordLength_8b; // octet comme taille élémentaore (standard)
		USART_InitStruct.USART_StopBits = USART_StopBits_1; // bit de stop = 1 (standard)
		USART_InitStruct.USART_Parity = USART_Parity_No; // pas de bit de parité (standard)
		USART_InitStruct.USART_HardwareFlowControl =
				USART_HardwareFlowControl_None; // pas de controle de flux (standard)
		USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
		USART_Init(USARTx, &USART_InitStruct);

		USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);

		//Setting of interrupt
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);

		//Enable UART
		USART_Cmd(USARTx, ENABLE);
	}

	/**
	 * Base function to send only one byte
	 *
	 */
	static inline void send_char(unsigned char c) {
		USART_SendData(USARTx, c);
		while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET) {
		}
	}

	/**
	 * Availability of data in the buffer
	 *
	 */
	static inline bool available(void) {
		return (RX_BUFFER_SIZE + rx_buffer_.head - rx_buffer_.tail)
				% RX_BUFFER_SIZE;
	}

	/**
	 * Read one byte from the ring buffer with a timeout (~ in ms)
	 *
	 */
	static inline uint8_t read_char(unsigned char &byte, uint16_t timeout = 0) {
		uint16_t i = 0;
		uint8_t j = 0;

		// Hack for timeout
		if (timeout > 0)
			timeout *= 26;

		while (!available()) {
			if (timeout > 0) {
				if (i > timeout)
					return READ_TIMEOUT;
				if (j == 0)
					i++;
				j++;
			}
		}

		byte = rx_buffer_.buffer[rx_buffer_.tail];
		rx_buffer_.tail = (rx_buffer_.tail + 1) % RX_BUFFER_SIZE;

		return READ_SUCCESS;
	}

	/**
	 * Store one byte in the ring buffer
	 *
	 */
	static inline void store_char(unsigned char c) {
		int i = (rx_buffer_.head + 1) % RX_BUFFER_SIZE;
		if (i != rx_buffer_.tail) {
			rx_buffer_.buffer[rx_buffer_.head] = c;
			rx_buffer_.head = i;
		}
	}

	/**
	 * Print the binary expression of a variable
	 *
	 */
	template<class T>
	static inline void print_binary(T val) {
		static char buff[sizeof(T) * 8 + 1];
		buff[sizeof(T) * 8] = '\0';
		uint16_t j = sizeof(T) * 8 - 1;
		for (uint16_t i = 0; i < sizeof(T) * 8; ++i) {
			if (val & ((T) 1 << i))
				buff[j] = '1';
			else
				buff[j] = '0';
			j--;
		}
		println((const char *) buff);
	}

	static inline void print_binary(unsigned char *val, int16_t len) {
		for (int16_t i = 0; i < len; ++i) {
			print_binary(val[i]);
		}
	}

	template<class T>
	static inline void print(T val) {
		write(val);
		send_char('\r');
	}

	template<class T>
	static inline void println(T val) {
		write(val);
		send_ln();
	}

	static inline void println(float val, int places) {
		write(val, places);
		send_ln();
	}

	template<class T>
	static inline uint8_t read(T &val, uint16_t timeout = 0) {
		static char buffer[20];
		uint8_t status = read(buffer, timeout);
		val = atol(buffer);

		return status;
	}

	static inline uint8_t read(float &val, uint16_t timeout = 0) {
		static char buffer[20];
		uint8_t status = read(buffer, timeout);
		val = atof(buffer);

		return status;
	}

	static inline uint8_t read(char* string, uint16_t timeout = 0) {
		static unsigned char buffer;
		uint8_t i = 0;

		do {
			if (read_char(buffer, timeout) == READ_TIMEOUT)
				return READ_TIMEOUT;

			if (i == 0 && buffer == '\r') {
				return READ_SUCCESS;
			}

			if (i == 0 && buffer == '\n') {
				continue;
			}

			string[i] = static_cast<char>(buffer);
			i++;
		} while (string[i - 1] != '\r');

		string[i - 1] = '\0';

		return READ_SUCCESS;
	}

};

template<uint8_t ID>
volatile typename Uart<ID>::ring_buffer Uart<ID>::rx_buffer_;

/**
 * Initialisation of USART number
 *
 */

template<> USART_TypeDef* Uart<1>::USARTx = USART1;
template<> USART_TypeDef* Uart<2>::USARTx = USART2;
template<> USART_TypeDef* Uart<3>::USARTx = USART3;
template<> USART_TypeDef* Uart<4>::USARTx = UART4;
template<> USART_TypeDef* Uart<5>::USARTx = UART5;
template<> USART_TypeDef* Uart<6>::USARTx = USART6;

/**
 * Interrupt service routines definitions
 *
 */

#ifdef __cplusplus
extern "C" {
#endif
void USART1_IRQHandler(void) {
	if (USART_GetITStatus(USART1, USART_IT_RXNE)) {
		unsigned char c = USART_ReceiveData(USART1);
		Uart<1>::store_char(c);
	}
}
void USART2_IRQHandler(void) {
	if (USART_GetITStatus(USART2, USART_IT_RXNE)) {
		unsigned char c = USART_ReceiveData(USART2);
		Uart<2>::store_char(c);
	}
}
void USART3_IRQHandler(void) {
	if (USART_GetITStatus(USART3, USART_IT_RXNE)) {
		unsigned char c = USART_ReceiveData(USART3);
		Uart<3>::store_char(c);
	}
}
void UART4_IRQHandler(void) {
	if (USART_GetITStatus(UART4, USART_IT_RXNE)) {
		unsigned char c = USART_ReceiveData(UART4);
		Uart<4>::store_char(c);
	}
}
void UART5_IRQHandler(void) {
	if (USART_GetITStatus(UART5, USART_IT_RXNE)) {
		unsigned char c = USART_ReceiveData(UART5);
		Uart<5>::store_char(c);
	}
}
void USART6_IRQHandler(void) {
	if (USART_GetITStatus(USART6, USART_IT_RXNE)) {
		unsigned char c = USART_ReceiveData(USART6);
		Uart<6>::store_char(c);
	}
}
#ifdef __cplusplus
}
#endif

#endif  /* UART_HPP */

