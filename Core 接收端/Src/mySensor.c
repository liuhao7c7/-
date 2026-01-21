#include "mySensor.h"
//#include "i2c.h"
#include "spi.h"
#include <string.h>
#include "usart.h"

/*IIC*/
#define DEV_ADDR (0x72<<1) //PAN3029 0x72<<1

#define DATA_ADDR 0x01 //256个
uint8_t my_Page=0xff;//寄存器页
uint8_t my_Mode=0;//射频模式

void delay_us(uint16_t time)
{
	while(time--)
	{
		__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();
	}
}

////0-正常 1-错误 2-忙 3-超时
//uint8_t Read_I2C_DMA(uint16_t MemAddr,uint8_t *pData,uint16_t Size)
//{
////	while(HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY) {;}//空闲
//	return	HAL_I2C_Mem_Read_DMA(&hi2c1,DEV_ADDR,(MemAddr<<1)|1,I2C_MEMADD_SIZE_8BIT,pData,Size);
//}

//uint8_t Write_I2C_DMA(uint16_t MemAddr,uint8_t *pData,uint16_t Size)
//{
//return	HAL_I2C_Mem_Write_DMA(&hi2c1,DEV_ADDR,MemAddr<<1,I2C_MEMADD_SIZE_8BIT,pData,Size);
//}

//uint8_t Read_I2C(uint16_t MemAddr,uint8_t *pData,uint16_t Size)
//{
////	while(HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY) {;}//空闲
//	return	HAL_I2C_Mem_Read(&hi2c1,DEV_ADDR,(MemAddr<<1)|1,I2C_MEMADD_SIZE_8BIT,pData,Size,100);
//}

//uint8_t Write_I2C(uint16_t MemAddr,uint8_t *pData,uint16_t Size)
//{
//return	HAL_I2C_Mem_Write(&hi2c1,DEV_ADDR,MemAddr<<1,I2C_MEMADD_SIZE_8BIT,pData,Size,100);
//}

//uint8_t Read_I2C_u8(uint8_t MemAddr)//从当前页的寄存器中读取一个字节
//{
//	static uint8_t pData=0;
//	HAL_I2C_Mem_Read(&hi2c1,DEV_ADDR,(MemAddr<<1)|1,I2C_MEMADD_SIZE_8BIT,&pData,1,100);
//	return pData;
//}

//// 新增的单字节 I2C 写函数  
//uint8_t Write_I2C_u8(uint8_t MemAddr, uint8_t pData)  
//{ 
//	HAL_I2C_Mem_Write(&hi2c1,DEV_ADDR,MemAddr<<1,I2C_MEMADD_SIZE_8BIT,&pData,1,100); // 传递单字节数据的地址
//	#if SPI_WRITE_CHECK
//	if(Read_I2C_u8(MemAddr)!=pData)return 1;
//	#endif
//	return 0;
//}

//void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c) {  
//    // 处理发送完成的逻辑  
//}  

//void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c) {  
//    // 处理接收完成的逻辑  
//}

//spi
#define SPI_BUFFER_SIZE 200           // SPI缓冲区大小
 uint8_t spi_rx_buffer[SPI_BUFFER_SIZE];
 uint8_t spi_tx_buffer[SPI_BUFFER_SIZE];

// 读取SPI数据
uint8_t Read_SPI(uint8_t Addr, uint8_t *pData,uint8_t Size) 
{
	HAL_StatusTypeDef status;
	spi_tx_buffer[0] = Addr<<1;//Addr // 设置读取地址
	CS_L;// 使能片选
	status = HAL_SPI_TransmitReceive(&hspi1,spi_tx_buffer, spi_rx_buffer, Size + 1, 0xffff);
	CS_H;
	if (status == HAL_OK) {
		memcpy(pData, spi_rx_buffer + 1, Size);
		return 0;
	}
	return 1;
}

//// 写入SPI数据
uint8_t Write_SPI(uint8_t Addr, uint8_t *pData, uint8_t Size) 
{
  HAL_StatusTypeDef status;
	memcpy(spi_tx_buffer + 1, pData, Size);// 复制数据到发送缓冲区
	spi_tx_buffer[0]=(Addr<<1)|0x01;// 地址设置为写模式
	CS_L;// 使能片选
  status = HAL_SPI_Transmit(&hspi1,spi_tx_buffer, Size+1, 0xffff);
	CS_H;// 禁用片选
	if (status == HAL_OK) {
   return 0;
	}
    return status;
}


uint8_t Read_SPI_u8(uint8_t Addr) 
{
	spi_tx_buffer[0] = Addr<<1;//Addr | 0x80;
	spi_tx_buffer[1] =0;
	CS_L;
	HAL_SPI_TransmitReceive(&hspi1,spi_tx_buffer, spi_rx_buffer, 2, 100);
	CS_H;
	return spi_rx_buffer[1];
}

uint8_t Write_SPI_u8(uint8_t Addr, const uint8_t pData) 
{
	spi_tx_buffer[0]=((Addr<<1)|0x01);// 地址设置为写模式
	spi_tx_buffer[1]=pData;
	
	CS_L;// 使能片选
  HAL_SPI_Transmit(&hspi1,spi_tx_buffer, 2, 100);
	// 禁用片选
  CS_H;
	#if SPI_WRITE_CHECK
	if(Read_SPI_u8(Addr)!=pData){	
	return 1;
	}
	#endif
	return 0;
}

//写入SPI数据DMA
uint8_t Write_SPI_DMA(uint8_t Addr, uint8_t *pData, uint8_t Size) 
{
	spi_tx_buffer[0]=(Addr<<1) | 0x01;// 地址设置为写模式
	memcpy(spi_tx_buffer + 1, pData, Size);// 复制数据到发送缓冲区
	CS_L;
	HAL_SPI_Transmit_DMA(&hspi1, spi_tx_buffer, Size+1);
}

uint8_t Read_SPI_DMA(uint8_t Addr,uint8_t Size)
{
	CS_L;// 使能片选
	// 设置读取地址
  spi_tx_buffer[0] = Addr<<1;//Addr | 0x80;
	HAL_SPI_TransmitReceive_DMA(&hspi1,spi_tx_buffer,spi_rx_buffer,Size+1);
}

#include "my_Database.h"
// SPI传输完成回调函数
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	 if (hspi->Instance == SPI1) {
      // 禁用片选
    CS_H;
	 }
}

// SPI传输完成回调
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if(hspi->Instance == SPI1)
    {
        CS_H; // 在SPI确认传输完成后拉高CS
    }
}

#ifdef DEBUG
			void My_Printf(uint8_t* data,uint8_t len)
{
//	HAL_UART_Transmit(&huart2,data,len,0xffff);
	HAL_UART_Transmit_DMA(&huart2,data,len);
}			
						#endif


