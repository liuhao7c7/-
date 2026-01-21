#include "mySensor.h"
#include "i2c.h"
#include "spi.h"
#include "my_Database.h"
#include <string.h>

#define BUFFER_SIZE 50           // 缓冲区大小
#define SPI_BUFFER_SIZE 100 
 uint8_t I2C_rx_buffer[BUFFER_SIZE];
 uint8_t spi_rx_buffer[SPI_BUFFER_SIZE];
 uint8_t spi_tx_buffer[SPI_BUFFER_SIZE];
 
/*IIC*/
uint8_t my_Page=0xff;

void delay_us(uint32_t time)
{
	while(time--)
	{
		__NOP(); // 每次调用 __NOP() 大约 250 ns  
//    __NOP(); // 2 NOP = 500 ns  
//    __NOP(); // 3 NOP = 750 ns  
//    __NOP(); // 4 NOP = 1 μs  
	}
}

//0-正常 1-错误 2-忙 3-超时
uint8_t Read_I2C_DMA(uint8_t DevAddr,uint8_t MemAddr,uint16_t Size)
{
	return	HAL_I2C_Mem_Read_DMA(&hi2c1,DevAddr|0x01,MemAddr,I2C_MEMADD_SIZE_8BIT,I2C_rx_buffer,Size);
}

uint8_t Write_I2C_DMA(uint8_t DevAddr,uint8_t MemAddr,uint8_t *pData,uint16_t Size)
{
return	HAL_I2C_Mem_Write_DMA(&hi2c1,DevAddr,MemAddr,I2C_MEMADD_SIZE_8BIT,pData,Size);
}

uint8_t Read_I2C(uint8_t DevAddr,uint8_t MemAddr,uint8_t *pData,uint16_t Size)
{
	return	HAL_I2C_Mem_Read(&hi2c1,DevAddr|0x01,MemAddr,I2C_MEMADD_SIZE_8BIT,pData,Size,100);
}

uint8_t Write_I2C(uint8_t DevAddr,uint8_t MemAddr,uint8_t *pData,uint16_t Size)
{
return	HAL_I2C_Mem_Write(&hi2c1,DevAddr,MemAddr,I2C_MEMADD_SIZE_8BIT,pData,Size,100);
}

uint8_t Read_I2C_u8(uint8_t DevAddr,uint8_t MemAddr)//从当前页的寄存器中读取一个字节
{
	static uint8_t pData=0;
	HAL_I2C_Mem_Read(&hi2c1,DevAddr|0x01,MemAddr,I2C_MEMADD_SIZE_8BIT,&pData,1,100);
	return pData;
}

// 新增的单字节 I2C 写函数  
uint8_t Write_I2C_u8(uint8_t DevAddr,uint8_t MemAddr, uint8_t pData)  
{ 
	HAL_I2C_Mem_Write(&hi2c1,DevAddr,MemAddr,I2C_MEMADD_SIZE_8BIT,&pData,1,100); // 传递单字节数据的地址
	#if SPI_WRITE_CHECK
	if(Read_I2C_u8(DevAddr,MemAddr)!=pData)return 1;
	#endif
	return 0;
}

void Modify_I2C_u8(uint8_t DevAddr,uint8_t MemAddr,uint8_t rate)
{
	uint8_t rx_spi;
	rx_spi=Read_I2C_u8(DevAddr,MemAddr);
	rx_spi |= rate;
	Write_I2C_u8(DevAddr,MemAddr, rx_spi);
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c) {  
    // 处理发送完成的逻辑  
}  

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c) {  
    // 处理接收完成的逻辑
}

//spi

uint8_t Read_SPI_u8(uint8_t Addr) 
{
	CS_L;
	spi_tx_buffer[0] = ((Addr<<1)|0x00);//Addr | 0x80;
	HAL_SPI_TransmitReceive(&hspi1,spi_tx_buffer, spi_rx_buffer, 2, 100);
	CS_H;
	return spi_rx_buffer[1];
}

uint8_t Write_SPI_u8(uint8_t Addr, uint8_t pData) {
		spi_tx_buffer[0]=((Addr<<1)|0x01);// 地址设置为写模式
		spi_tx_buffer[1]=pData;
		CS_L;// 使能片选
    HAL_SPI_Transmit(&hspi1,spi_tx_buffer, 2, 100);
		// 禁用片选
    CS_H;
	#if SPI_WRITE_CHECK
	if(Read_SPI_u8(Addr)!=pData)return 1;
	#endif
	return 0;
}

// 读取SPI数据
uint8_t Read_SPI(uint8_t Addr, uint8_t *pData,uint16_t Size) 
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
uint8_t Write_SPI(uint8_t Addr, uint8_t *pData, uint16_t Size) 
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


//写入SPI数据DMA
void Write_SPI_DMA(uint8_t Addr, uint8_t *pData, uint16_t Size) 
{
	spi_tx_buffer[0]=(Addr<<1) | 0x01;// 地址设置为写模式
	memcpy(spi_tx_buffer + 1, pData, Size);// 复制数据到发送缓冲区
	CS_L;
	HAL_SPI_Transmit_DMA(&hspi1, spi_tx_buffer, Size+1);
}


void Read_SPI_DMA(uint8_t Addr,uint16_t Size)
{
	CS_L;// 使能片选
	// 设置读取地址
  spi_tx_buffer[0] = Addr<<1;//Addr | 0x80;
	HAL_SPI_TransmitReceive_DMA(&hspi1,spi_tx_buffer,spi_rx_buffer,Size+1);
}


int16_t acc[4][3];
int16_t gyr[4][3];
uint8_t acc_Num=0;
uint8_t gyr_Num=0;
#include "my_Database.h"
// SPI传输完成回调函数
// SPI传输完成回调
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if(hspi->Instance == SPI1)
    {
        CS_H; // 在SPI确认传输完成后拉高CS
    }
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	 if (hspi->Instance == SPI1) {
      // 禁用片选
    CS_H;
	 }
}


