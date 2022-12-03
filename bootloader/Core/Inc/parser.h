/*
 * parser.h
 *
 *  Created on: Oct 16, 2022
 *      Author: Safa
 */

/*
@Adi: Safa
@Soyadi: Kucukkomurler
@Tarih: 16.10.2022
*/

#ifndef INC_PARSER_H_
#define INC_PARSER_H_

#include <stdint.h>
#include "variableParts.h"
#include "stm32f070xb.h"

#define PACKET_LENGTH 15U
#define MY_SLAVE_NUM 0x45

#define PACKET_HEADER 163U 	// '£'
#define PACKET_FINISHER 37U	// '%'

#define MY_MCU_PERIPH_START_ADDR PERIPH_BASE 	//Cevre birimi konfigyrayonunda emir adresi aralik kontrolu
#define MY_MCU_PERIPH_END_ADDR 0x480017FFUL		//Cevre birimi konfigyrayonunda emir adresi aralik kontrolu

//Gecici 15 byte lik diziye konan yapi ayiklanmadan
//onceki dizideki konumlari
enum{
	HEADER_INDEX,
	COMM_NUM_L_INDEX,
	COMM_NUM_H_INDEX,
	SLAVE_NUM_INDEX,
	COMM_ADDR_BYTE_0_INDEX,
	COMM_ADDR_BYTE_1_INDEX,
	COMM_ADDR_BYTE_2_INDEX,
	COMM_ADDR_BYTE_3_INDEX,
	RW_INDEX,
	DATA_TYPE_INDEX,
	DATA_BYTE_0_INDEX,
	DATA_BYTE_1_INDEX,
	DATA_BYTE_2_INDEX,
	DATA_BYTE_3_INDEX,
	FINISHER_INDEX,
};

typedef enum command_typeTAG{
	PERIPH_COMM = 1,	//Peripheral konfigurasyon komutu ornegin Timer1
	BL_FLASH_WRITE,		//Bootloader flash yazma icin
	BL_JMP_APP,			//Bootloader dan uygulamaya gecme icin
	BL_FLASH_ERASE,		//Bootloader flash silme icin
	COMM_TYPE_LIM		//Bu ifadenin ustundeki tanimlamalarin sayisini tutar if ile kontrol yaparken kolaylik saglamasi icin
}comm_type_t;

typedef enum parser_statusTAG{
	NOT_ENOUGH_DATA, 	//Header alindi 15 byte in tamami alinmadi daha veri alimi suruyor.
	RECEIVE_OK, 		//Veri alimi tamamlandi. Header ve finisher dogru
	PARSE_OK, 			//Parse tamamlandi. Data tipi ve read-write ve slave numara dogru.
	PROCESS_OK,			//Yazma okuma islemleri tamamlandi. Data tipi ve adres araligi uyumlu.
	ANSWER_OK,			//Cevap gonderildi
	HEADER_ERROR,		//Header hatali
	FINISHER_ERROR,		//Finisher hatali
	RW_BYTE_ERROR,		//Read-Write komutu hatali (1 veya 2 degil)
	DATA_TYPE_ERROR,	//Data tipi hatali (0 ile 7 arasinda degil)
	ADDR_RANGE_ERROR,	//Cevre birimi ile ilgili islem yapilacaksa adres araligi dogru olmali
	DATA_TYPE_FALSE,	//Veri tipi islem ile uyumsuz. (Ornegin Timer1 konfigurasyonu icin unsigned int olmali)
	COMM_NUM_ERROR,		//Emir numarasi belirli aralikta olmali. Emir numaralari ozel numaralar ise ve her seferinde artmiyorsa.
	SLAVE_NUM_FALSE		//Mesajin muhatabi bu cihaz degil
}parser_status_t;

typedef enum rwTAG{
	READ = 1,
	WRITE
}rw_t;

typedef enum data_typeTAG{
	CHAR_DATA = 1,
	FLOAT_DATA,
	INT16_DATA,
	INT32_DATA,
	UINT16_DATA,
	UINT32_DATA,
	DATA_TYPE_LIM //7 numara //Bu ifadenin ustundeki tanimlamalarin sayisini tutar if ile kontrol yaparken kolaylik saglamasi icin
}data_type_t;

//Data kullanilirken kolay erisim ve otomatik tip donusumu icin
typedef union{
	uint32_t data_u32;
	int32_t data_s32;

	uint16_t data_u16;
	int16_t data_s16;

	char data_c;

	float data_f;

	u8u8u8u8 bytes;
}receive_data_t;

//Alinan paket yapisi
typedef struct packet_structTAG{

	char header;					//Baslangic
	u16_u8u8 commandNumber;			//Emir Numarasi
	uint8_t slaveNum;				//Slave numarasi
	u32_u8u8u8u8 commandAddress;	//Emir adresi
	uint8_t rw;						//Read-Write
	uint8_t dataType;				//Data tipi
	receive_data_t data;			//Data blogu
	char finisher;					//Bitis

}packet_struct_t;

extern uint32_t receivedByteCnt;			//Alinan byte sayisi
extern uint32_t receivedPacketCnt, wrongPacketCnt;	//Alinan paket sayisi, Hatali paket sayisi

//Parser ve uart haberlesme ana fonksiyon
parser_status_t communication_layer_handler(packet_struct_t* receivedPacket, receive_data_t* sendData);

//Mesaj dogru ve parse basarili ise cevap gonderme hatali ise NACK gonderecek sekildede kodlanabilir.
parser_status_t answer_handler(packet_struct_t* receivedPacket, receive_data_t* sendData);

#endif /* INC_PARSER_H_ */
