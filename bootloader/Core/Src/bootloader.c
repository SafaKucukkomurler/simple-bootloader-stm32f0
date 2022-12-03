/*
 * bootloader.c
 *
 *  Created on: Oct 17, 2022
 *      Author: Safa
 */

/*
@Adi: Safa
@Soyadi: Kucukkomurler
@Tarih: 16.10.2022
*/

#include "bootloader.h"

void jump_user_app(void){

   //Uygulamaya gecmek icin fonksiyon isaretcisi
	MY_PRINTF("Jumping to application: Address: %#x\r\n",FLASH_PAGE20_BASE_ADDRESS + 4);
	HAL_Delay(500);
    void (*jumpApplication)(void);

    //Gecilecek uygulamanin stack i ayarlaniyor
    uint32_t msp_value = *(volatile uint32_t *)FLASH_PAGE20_BASE_ADDRESS;

    //SCB->VTOR = FLASH_PAGE20_BASE_ADDRESS;

    //Base adresten 4 fazlasi uygulamanin reset handler i elde ediliyor.
    //Orada startup kodu ve ardindan main e gecis var.
    uint32_t resetHandlerAddress = *(volatile uint32_t *) (FLASH_PAGE20_BASE_ADDRESS + 4);

    //++resetHandlerAddress;

    jumpApplication = (void*) resetHandlerAddress;

    //Interrupt kapali
    __disable_irq();

    //Gecilecek uygulamanin stack i ayarlaniyor
    __set_MSP(msp_value);

    //Uygulamaya geciliyor
    jumpApplication();

}

void flash_erase_handler(uint32_t pageAddress){

	FLASH_EraseInitTypeDef flashHandle;
	uint32_t pageError;
	HAL_StatusTypeDef status;

	if(pageAddress == (uint8_t) 0xffffffff)
	{
		flashHandle.TypeErase = FLASH_TYPEERASE_MASSERASE;
	}
	else
	{

		//Silinmeye baslanacak page numarasi bulunuyor. Her page 2KB
		uint8_t pageNumber = (uint8_t)((pageAddress - 0x8000000UL)/2048);

		uint8_t pageCount = 64 - pageNumber; //Silinecek page sayisinin bulunmasi (STM32F070RB de 64 adet page var.)
		flashHandle.TypeErase = FLASH_TYPEERASE_PAGES;
		flashHandle.PageAddress = pageAddress; 	// Silmeye baslanacak page
		flashHandle.NbPages = pageCount;	// Silinecek page sayisi
	}

	//Flash register erisim
	HAL_FLASH_Unlock();
	status = HAL_FLASHEx_Erase(&flashHandle, &pageError);
	HAL_FLASH_Lock();

	MY_PRINTF("Flash Erase Status: %d\r\n",status);
}

void flash_write_handler(uint32_t data, uint32_t memAddress){

	HAL_StatusTypeDef status;
	HAL_StatusTypeDef status2;

	status2 = HAL_FLASH_Unlock();
	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
	status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, memAddress, data);
	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
	MY_PRINTF("Flash Write: Address:%#x Data:%#x Status Unlock: %d Status: %d\r\n",memAddress,data,status2,status);
	HAL_FLASH_Lock();
}

