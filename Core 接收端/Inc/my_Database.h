#ifndef __MY_DATABASE_H  //防止头文件的重复包含和编译  配套#endif
#define __MY_DATABASE_H  //标识符
#include "stdint.h"

#define DataCRC_u16(PTR) ((((uint8_t *)(PTR))[1]<<8)|((uint8_t *)(PTR))[0]) 
#define Data_u16(PTR) ((((uint8_t *)(PTR))[0]<<8)|((uint8_t *)(PTR))[1])  //从缓冲区取16位数据
#define Data_u24(PTR) (((((uint8_t *)(PTR))[0])) * 65536+ ((((uint8_t *)(PTR))[1]<<8)|((uint8_t *)(PTR))[2]))  //从缓冲区取24位数据
#define Data_u32(PTR) (((((uint8_t *)(PTR))[0]<<8)|(((uint8_t *)(PTR))[1])) * 65536+ ((((uint8_t *)(PTR))[2]<<8)|((uint8_t *)(PTR))[3]))  //从缓冲区取32位数据


typedef struct
{
	uint8_t LowOption;////0水平安装 1垂直安装 010差分 001电流 
	uint16_t Weight;//重量数据
	uint16_t Battery;//电池电压数据
	uint16_t AngleX,AngleY,AngleZ;//角度
	int16_t AngleX_int16,AngleY_int16,AngleZ_int16;
	uint8_t DI;//
	uint8_t rssi;//信号强度
	uint8_t myrssi;//我收到对方数据信号强度
	uint8_t Chrg;//充电状态
	uint32_t Offline_time;//没有收到433信号时间 超过多长时间433休眠 重量断电 角度休眠
	uint32_t GetNum;//收到的数量
	uint32_t OnlineTime;//在线时间
	uint32_t OnlineTimeOld;
}DEVICE;

typedef struct/*各参数结构体，需要增加参数只需在此添加*/
{
//	uint16_t Addr;//地址433
	uint8_t  ModbusADDR;//地址
	uint8_t  Number;//数量 1. 2. 3
	uint8_t  Channel;//设备信道
	uint16_t Time_to_stop;//待机模式超时时间150s
	uint8_t  ADC_D_value;//ADC变化参考值
	uint8_t  Bat_D_value;//电池电压变化参考值
	uint8_t  Angel_D_value;//角度变化参考值
	uint8_t  Ask_Rxc[2];//0x11,0x22请求返回数据1并接收完成返回2标志
	uint8_t  Ask_Rxc_OK;//收到几的数据
	DEVICE   Device[15];//15个设备
	uint16_t OnlineFlag;//在线标志
	uint8_t  Ask;//请求数据 询问几
	uint8_t  rf_flag;//射频状态
	uint8_t  Send_flag;//询问时间
	uint16_t err;//0自己 1-15低功耗
}My_433;
extern My_433 my_433;

#define BUFF_SIZE 30 //DMA数据缓存大小
typedef struct
{
	uint8_t Function;//功能码
	uint8_t temp_buf[BUFF_SIZE];//接收数据缓存  buf[20];
	uint8_t buf[BUFF_SIZE];//接收数据  RX_buf[20];
	uint8_t RX_len;//接收数据长度  datLen;//接收数据计数
	uint8_t Len;//长度+1  rcvLen;//接收数据计数
	uint16_t TX_addr;		//回传起始地址
	uint16_t TX_length;	//回传数据长度
}my_modbus_typedef;//modbus
extern my_modbus_typedef my_modbus;

void RTU_Send_CRC16(uint8_t* data,uint8_t Len);
uint8_t RTU_Get_CRC16(uint8_t* data,uint8_t Len);
void modbus_ack_handle(my_modbus_typedef* p);

#endif

