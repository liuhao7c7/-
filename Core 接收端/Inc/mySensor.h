#ifndef __MYSENSOR_H  //防止头文件的重复包含和编译  配套#endif
#define __MYSENSOR_H  //标识符

#include "main.h"
void delay_us(uint16_t time);
//uint8_t Read_I2C(uint16_t MemAddr,uint8_t *pData,uint16_t Size);
//uint8_t Write_I2C(uint16_t MemAddr,uint8_t *pData,uint16_t Size);
//uint8_t Read_I2C_DMA(uint16_t MemAddr,uint8_t *pData,uint16_t Size);
//uint8_t Write_I2C_DMA(uint16_t MemAddr,uint8_t *pData,uint16_t Size);
//uint8_t Read_I2C_u8(uint8_t MemAddr);
//uint8_t Write_I2C_u8(uint8_t MemAddr, uint8_t pData);

uint8_t Write_SPI(uint8_t Addr, uint8_t *pData, uint8_t Size);
uint8_t Write_SPI_DMA(uint8_t Addr, uint8_t *pData, uint8_t Size);
uint8_t Read_SPI(uint8_t Addr, uint8_t *pData,uint8_t Size);
uint8_t Read_SPI_DMA(uint8_t Addr,uint8_t Size);
uint8_t Read_SPI_u8(uint8_t Addr);
uint8_t Write_SPI_u8(uint8_t Addr, const uint8_t pData);

#ifdef DEBUG
void My_Printf(uint8_t* data,uint8_t len);						
						#endif


extern uint8_t my_Page;
extern uint8_t my_Mode;

#define SPI_WRITE_CHECK  1 //校验

#define CS_H HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET)
#define CS_L HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET)

#pragma pack(1)

#pragma pack ()



#endif

