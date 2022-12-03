/*
 * parser.c
 *
 *  Created on: Oct 16, 2022
 *      Author: Safa
 */

/*
@Adi: Safa
@Soyadi: Kucukkomurler
@Tarih: 16.10.2022
*/

#include "parser.h"
#include "uartDriver.h"

uint32_t receivedByteCnt;
uint32_t receivedPacketCnt, wrongPacketCnt;

//Bu fonksiyonlar main den cagirilmasina gerek yok
//O yuzden static
static parser_status_t receive_handler(void);
static parser_status_t parse_handler(packet_struct_t* receivedPacket);
static parser_status_t process_handler(packet_struct_t* receivedPacket, receive_data_t* sendData);

static uint8_t buffer[PACKET_LENGTH] = {0};

//Uart tan okuma yapilirken 15 byte in belirli kismi geldi
//okuma devam ediyor. Yada tamami okundu icin. receive_handler() icin
enum {
	NEW_READ,
	READING
};

//Parser ve uart haberlesme ana fonksiyon
parser_status_t communication_layer_handler(packet_struct_t* receivedPacket, receive_data_t* sendData){

	parser_status_t parseStatus = receive_handler();

	if(parseStatus == RECEIVE_OK)
	{
		parseStatus = parse_handler(receivedPacket);
		if(parseStatus == PARSE_OK)
		{
			parseStatus = process_handler(receivedPacket, sendData);
		}
	}

	return parseStatus;

}

//Okuma ve header finisher kontrol islemleri
static parser_status_t receive_handler(void){

	char header = 0;
	static uint8_t bufferIndex = 0;
	static uint8_t status = NEW_READ;

	if(status == NEW_READ)
	{
		//yeni okumada gecici buffer sifirlaniyor
		bufferIndex = 0;
		for(int i = 0; i < PACKET_LENGTH; i++)
		{
			buffer[i] = 0;
		}

		//Okunacak veri varmi
		if(USART_BytesToRead() > 0)
		{
			//Header bulana kadar oku ayrica alinacak veri hala varsa.
			do{
				header = USART_ReadByte();
				receivedByteCnt++;
			}while((header != 163) && (USART_BytesToRead() > 0));

			if (header == PACKET_HEADER)
			{
				//Kalan verileri oku varsa
				buffer[bufferIndex++] = header;
				while(USART_BytesToRead() > 0 && bufferIndex < PACKET_LENGTH)
				{
					receivedByteCnt++;
					buffer[bufferIndex++] = USART_ReadByte();
				}

				//Hepsi okunduysa ve finisher dogruysa basarili olarak don.
				//Daha hepsi gelmediyse bi dahaki verilerin gelip okunmasi
				//ve yazilimin bloklanmamasi icin status = READING yap ve don.
				if(bufferIndex == PACKET_LENGTH)
				{
					status = NEW_READ;
					//Finisher dogrumu?
					if (buffer[PACKET_LENGTH-1] == PACKET_FINISHER)
						return RECEIVE_OK;
					else
						return FINISHER_ERROR;
				}
				else
				{
					status = READING;
					return NOT_ENOUGH_DATA;
				}
			}
			else
				return HEADER_ERROR;
		}
		else
		{
			//Hic okunacak veri yok geri don
			status = NEW_READ;
			return NOT_ENOUGH_DATA;
		}

	}
	else
	{
		//Eski kaldigin yerden okumaya devam et.
		while(USART_BytesToRead() > 0 && bufferIndex < PACKET_LENGTH)
		{
			receivedByteCnt++;
			buffer[bufferIndex++] = USART_ReadByte();
		}

		//Hepsi okunduysa ve finisher dogruysa basarili olarak don.
		if(bufferIndex == PACKET_LENGTH)
		{
			status = NEW_READ;
			//Finisher dogrumu?
			if (buffer[PACKET_LENGTH-1] == PACKET_FINISHER)
				return RECEIVE_OK;
			else
				return FINISHER_ERROR;
		}
		else
		{
			status = READING;
			return NOT_ENOUGH_DATA;
		}
	}
}

static parser_status_t parse_handler(packet_struct_t* receivedPacket){

	//Slave numara bu cihaz icin mi? Muhatap bu mu?
	if(buffer[SLAVE_NUM_INDEX] == MY_SLAVE_NUM)
	{
		//Read-Write komutu dogru mu?
		if(buffer[RW_INDEX] == READ || buffer[RW_INDEX] == WRITE)
		{
			//Data tipi verilen araliktami?
			if(buffer[DATA_TYPE_INDEX] > 0 && buffer[DATA_TYPE_INDEX] < DATA_TYPE_LIM)
			{
				receivedPacket->header = buffer[HEADER_INDEX];
				receivedPacket->finisher = buffer[FINISHER_INDEX];
				receivedPacket->slaveNum = buffer[SLAVE_NUM_INDEX];
				receivedPacket->rw = buffer[RW_INDEX];
				receivedPacket->dataType = buffer[DATA_TYPE_INDEX];

				//MSB byte birinci yada LSB byte birinci. Burada LSB byte birinci.
				//MSB birinci ise asagida byte siralamasi degistirilmeli
				receivedPacket->commandNumber.high_low.low = buffer[COMM_NUM_L_INDEX];
				receivedPacket->commandNumber.high_low.high = buffer[COMM_NUM_H_INDEX];
				receivedPacket->commandAddress.bytes.byte0 = buffer[COMM_ADDR_BYTE_0_INDEX];
				receivedPacket->commandAddress.bytes.byte1 = buffer[COMM_ADDR_BYTE_1_INDEX];
				receivedPacket->commandAddress.bytes.byte2 = buffer[COMM_ADDR_BYTE_2_INDEX];
				receivedPacket->commandAddress.bytes.byte3 = buffer[COMM_ADDR_BYTE_3_INDEX];
				receivedPacket->data.bytes.byte0 = buffer[DATA_BYTE_0_INDEX];
				receivedPacket->data.bytes.byte1 = buffer[DATA_BYTE_1_INDEX];
				receivedPacket->data.bytes.byte2 = buffer[DATA_BYTE_2_INDEX];
				receivedPacket->data.bytes.byte3 = buffer[DATA_BYTE_3_INDEX];

				return PARSE_OK;
			}
			else
			{
				wrongPacketCnt++;
				return DATA_TYPE_ERROR;
			}
		}
		else
		{
			wrongPacketCnt++;
			return RW_BYTE_ERROR;
		}
	}
	else
	{
		wrongPacketCnt++;
		return SLAVE_NUM_FALSE;
	}

}

static parser_status_t process_handler(packet_struct_t* receivedPacket, receive_data_t* sendData){

	//sendData->data_u32 = 0;

	//Emir numarasi belirli aralikta olmali. Emir numaralari ozelse her seferinde artmiyorsa.
	if(receivedPacket->commandNumber.all > 0 && receivedPacket->commandNumber.all < COMM_TYPE_LIM)
	{
		//Cevap gelen data ile birlikte aynisi olarak veriliyor.
		sendData->data_u32 = receivedPacket->data.data_u32;

		if(receivedPacket->commandNumber.all == PERIPH_COMM)
		{
			//Cevre birimi (Ornegin Timer1) konfigurasyon komutu geldiyse veri tipi unsigned int olmali. (Soru gereksinimi)
			//Projeye gore diger kontroller de tanimlanabilir.
			if(receivedPacket->dataType != UINT32_DATA)
			{
				wrongPacketCnt++;
				return DATA_TYPE_FALSE;
			}
			//Ayrica gelen emir adresi cevre birimi adres araliginda olmali.
			else if(receivedPacket->commandAddress.all < MY_MCU_PERIPH_START_ADDR ||
					receivedPacket->commandAddress.all > MY_MCU_PERIPH_END_ADDR)
			{
				wrongPacketCnt++;
				return ADDR_RANGE_ERROR;
			}
			else
			{
				//Bu asamaya gelindiyse alinan paket hatasizdir, dogrudur.
				//Alinan paket sayisini artir.
				//Yazma yada okuma islemini yap.
				if(receivedPacket->rw == WRITE)
				{
					//Bellek bolgesine yaz
					*((uint32_t*)(receivedPacket->commandAddress.all)) = receivedPacket->data.data_u32;
				}
				else
				{
					//Bellek bolgesini oku ve gonder
					sendData->data_u32 = *((uint32_t*)(receivedPacket->commandAddress.all));
				}
				receivedPacketCnt++;
				return PROCESS_OK;
			}
		}
		else
		{
			receivedPacketCnt++;
			return PROCESS_OK;
		}
	}
	else
	{
		wrongPacketCnt++;
		return COMM_NUM_ERROR;
	}


}

//Mesaj dogru ve parse basarili ise cevap gonderme hatali ise NACK gonderecek sekildede kodlanabilir.
parser_status_t answer_handler(packet_struct_t* receivedPacket,  receive_data_t* sendData){

	USART_SendByte(receivedPacket->header);
	USART_SendByteArray(((uint8_t*)&receivedPacket->commandNumber.all), 2);
	USART_SendByte(receivedPacket->slaveNum);
	USART_SendByteArray(((uint8_t*)&receivedPacket->commandAddress.all), 4);
	USART_SendByte(receivedPacket->rw);
	USART_SendByte(receivedPacket->dataType);
	USART_SendByteArray(((uint8_t*)&sendData->data_u32), 4);
	USART_SendByte(receivedPacket->finisher);

	return ANSWER_OK;
}

