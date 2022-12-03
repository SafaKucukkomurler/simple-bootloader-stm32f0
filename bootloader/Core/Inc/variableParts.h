/*
 * variableParts.h
 *
 *  Created on: 16 Eki 2022
 *      Author: Safa
 */
 
  /*
@Adi: Safa
@Soyadi: Kucukkomurler
@Tarih: 16.10.2022
@Bazi veri yapisi tanimlamalari 
*/

#ifndef INC_VARIABLEPARTS_H_
#define INC_VARIABLEPARTS_H_

/* uint16_t 2 uint8_t-bits parcaya bolundu.
*/
typedef struct{
	uint8_t low;
	uint8_t high;
}u8u8;

typedef union{
	uint16_t all;
	u8u8 high_low;
}u16_u8u8;

/* uint32_t 4 uint8_t parcaya bolundu.
*/
typedef struct{
	uint8_t byte0;
	uint8_t byte1;
	uint8_t byte2;
	uint8_t byte3;
}u8u8u8u8;

typedef union{
	uint32_t  all;
	u8u8u8u8  bytes;
}u32_u8u8u8u8;

#endif /* INC_VARIABLEPARTS_H_ */
