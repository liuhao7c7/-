#include "Data_Store.h"
#include "MyMainprogram.h"
//#include "rtc.h"

//EEPROM读写      1KB   0~1023
#define BaseAddr 0x08080000  //DATA_EEPROM_BASE   EEPROM起始地址      DATA_EEPROM_END  EEPROM结束地址

void My_EEPROM_WriteLen(uint16_t AddrWrite,uint32_t Data,uint16_t Len)//Len 要写入数据的长度1,2,3,4
{
	HAL_DATA_EEPROMEx_Unlock();//解锁EEPROM   HAL_FLASHEx_DATAEEPROM_Unlock
	for(uint8_t t=0;t<Len;t++)
	{
		HAL_DATA_EEPROMEx_Program(TYPEPROGRAMDATA_BYTE,AddrWrite+t+BaseAddr,(Data>>(8*t))&0xff);//将数据写入地址   HAL_FLASHEx_DATAEEPROM_Program
	}
	HAL_DATA_EEPROMEx_Lock();//上锁保护EEPROM   HAL_FLASHEx_DATAEEPROM_Lock
}

uint32_t My_EEPROM_ReadLen(uint16_t ReadAdd,uint8_t Len)
{
	uint32_t Temp =0;
	for(uint8_t t=0;t<Len;t++)
	{
		Temp<<=8;
		Temp+= *(__IO uint8_t*)(BaseAddr+ReadAdd+Len-t-1);
	}
	return Temp;
}

void My_EEPROM_Read(uint16_t ReadAdd,uint8_t *Cache,uint8_t Len)
{
		*Cache= *(__IO uint8_t*)(BaseAddr+ReadAdd);
}

//BKP 备份寄存器  32位数据
//void My_BKP_Write(uint8_t AddrWrite,uint32_t Data)
//{
//	switch(AddrWrite)
//	{
//		case 0:HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR0,Data);break;
//		case 1:HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR1,Data);break;
//		case 2:HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR2,Data);break;
//		case 3:HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR3,Data);break;
//		case 4:HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR4,Data);break;
//		default:break;
//	}
//}

//uint32_t My_BKP_Read(uint8_t AddrRead)
//{
//	uint32_t Data=0;;
//	switch(AddrRead)
//	{
//		case 0:Data=HAL_RTCEx_BKUPRead(&hrtc,RTC_BKP_DR0);break;//IWDG开启标志
//		case 1:Data=HAL_RTCEx_BKUPRead(&hrtc,RTC_BKP_DR1);break;
//		case 2:Data=HAL_RTCEx_BKUPRead(&hrtc,RTC_BKP_DR2);break;
//		case 3:Data=HAL_RTCEx_BKUPRead(&hrtc,RTC_BKP_DR3);break;
//		case 4:Data=HAL_RTCEx_BKUPRead(&hrtc,RTC_BKP_DR4);break;
//		default:break;
//	}
//	return Data;
//}


