/*
 * bootloader.h
 *
 *  Created on: Oct 17, 2022
 *      Author: Safa
 */

/*
@Adi: Safa
@Soyadi: Kucukkomurler
@Tarih: 16.10.2022
*/

#ifndef INC_BOOTLOADER_H_
#define INC_BOOTLOADER_H_

#include "main.h"
#include "spy.h"

#define FLASH_PAGE20_BASE_ADDRESS 0x0800A000

void jump_user_app(void);
void flash_erase_handler(uint32_t pageAddress);
void flash_write_handler(uint32_t data, uint32_t memAddress);

#endif /* INC_BOOTLOADER_H_ */
