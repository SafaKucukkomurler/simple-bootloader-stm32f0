#ifndef INC_UARTDRIVER_H_
#define INC_UARTDRIVER_H_

/*
@Adi: Safa
@Soyadi: Kucukkomurler
@Daha once yaptigim uart driver 
*/

#include "stm32f0xx_hal.h"

#define UART_BUFFER_SIZE 60

typedef struct uartBufferType
{
	uint32_t buffer[UART_BUFFER_SIZE];
	uint32_t headPointer;
	uint32_t tailPointer;
} uartBufferT;

void USART_SendByte(uint8_t data);
uint8_t USART_IsBuffEmpty(volatile uartBufferT* buffer);
int32_t USART_ReadByte();
uint32_t USART_BytesToRead();
void USART_SendByteArray(uint8_t* buffer, uint32_t size);

#endif
