#include "uartDriver.h"

/*
@Adi: Safa
@Soyadi: Kucukkomurler
@Daha once yaptigim uart driver 
*/

static volatile uartBufferT uartBuffTX;
static volatile uartBufferT uartBuffRX;

void USART2_IRQHandler(void){

	uint32_t usart2_isrflags = USART2->ISR;
	uint32_t usart2_control_reg = USART2->CR1;

	if (((usart2_isrflags & USART_ISR_RXNE) != RESET) && ((usart2_control_reg & USART_CR1_RXNEIE) != RESET))
	{
		uartBuffRX.buffer[uartBuffRX.headPointer++] = USART2->RDR;

		if(uartBuffRX.headPointer == UART_BUFFER_SIZE)
		{
			uartBuffRX.headPointer = 0;
		}

		return;
	}

	if (((usart2_isrflags & USART_ISR_TXE) != RESET) && ((usart2_control_reg & USART_CR1_TXEIE) != RESET))
	{
		if(uartBuffTX.headPointer != uartBuffTX.tailPointer)
		{
			USART2->TDR = uartBuffTX.buffer[uartBuffTX.tailPointer++];

			if(uartBuffTX.tailPointer == UART_BUFFER_SIZE)
			{
				uartBuffTX.tailPointer = 0;
			}
		}
		else
		{
			CLEAR_BIT(USART2->CR1, USART_CR1_TXEIE);
		}

		return;
	}


}

void USART_SendByte(uint8_t data){

	uartBuffTX.buffer[uartBuffTX.headPointer++] = data;

	if(uartBuffTX.headPointer == UART_BUFFER_SIZE)
	{
		uartBuffTX.headPointer = 0;
	}

	SET_BIT(USART2->CR1, USART_CR1_TXEIE);
}

uint8_t USART_IsBuffEmpty(volatile uartBufferT* buffer){

	if(buffer->headPointer == buffer->tailPointer)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int32_t USART_ReadByte(){

	int32_t receivedByte = 0;
	if(USART_IsBuffEmpty(&uartBuffRX) == 1)
	{
		receivedByte = -1;
	}
	else
	{
		receivedByte = uartBuffRX.buffer[uartBuffRX.tailPointer++];

		if(uartBuffRX.tailPointer==UART_BUFFER_SIZE)
		{
			uartBuffRX.tailPointer = 0;
		}
	}

	return receivedByte;
}

uint32_t USART_BytesToRead(){

	return (UART_BUFFER_SIZE + uartBuffRX.headPointer - uartBuffRX.tailPointer) % UART_BUFFER_SIZE;

}

void USART_SendByteArray(uint8_t* buffer, uint32_t size){

	uint32_t i;
	for(i=0; i<size; i++)
	{
		USART_SendByte(buffer[i]);
	}
}
