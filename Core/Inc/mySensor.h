#ifndef __MYSENSOR_H  //防止头文件的重复包含和编译  配套#endif
#define __MYSENSOR_H  //标识符

#include "main.h"
void delay_us(uint32_t time);
uint8_t Read_I2C(uint8_t DevAddr,uint8_t MemAddr,uint8_t *pData,uint16_t Size);
uint8_t Write_I2C(uint8_t DevAddr,uint8_t MemAddr,uint8_t *pData,uint16_t Size);
uint8_t Read_I2C_DMA(uint8_t DevAddr,uint8_t MemAddr,uint16_t Size);
uint8_t Write_I2C_DMA(uint8_t DevAddr,uint8_t MemAddr,uint8_t *pData,uint16_t Size);
uint8_t Read_I2C_u8(uint8_t DevAddr,uint8_t MemAddr);
uint8_t Write_I2C_u8(uint8_t DevAddr,uint8_t MemAddr, uint8_t pData);
void Modify_I2C_u8(uint8_t DevAddr,uint8_t MemAddr,uint8_t rate);

uint8_t Write_SPI(uint8_t Addr, uint8_t *pData, uint16_t Size);
void Write_SPI_DMA(uint8_t Addr, uint8_t *pData, uint16_t Size);
uint8_t Read_SPI(uint8_t Addr, uint8_t *pData,uint16_t Size);
void Read_SPI_DMA(uint8_t Addr,uint16_t Size);
uint8_t Read_SPI_u8(uint8_t Addr);
uint8_t Write_SPI_u8(uint8_t Addr, uint8_t pData);

extern uint8_t my_Page;

#define SPI_WRITE_CHECK  1 //校验

#define CS_H HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET) 
#define CS_L HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET)

#define I2C_SCL_H HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET) 
#define I2C_SCL_L HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET)
#define I2C_SDA_H HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET) 
#define I2C_SDA_L HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET)

#pragma pack(1)

#pragma pack ()



#endif

