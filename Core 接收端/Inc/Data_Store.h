#ifndef __DATA_STORE_H  //防止头文件的重复包含和编译  配套#endif
#define __DATA_STORE_H  //标识符

#include "stdint.h"

void My_EEPROM_WriteLen(uint16_t AddrWrite,uint32_t Data,uint16_t Len);//Len 要写入数据的长度1,2,3,4
uint32_t My_EEPROM_ReadLen(uint16_t ReadAdd,uint8_t Len);
void My_EEPROM_Read(uint16_t ReadAdd,uint8_t *Cache,uint8_t Len);
void My_BKP_Write(uint8_t AddrWrite,uint32_t Data);
uint32_t My_BKP_Read(uint8_t AddrRead);

#endif


