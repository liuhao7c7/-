#ifndef __MY_DATABASE_H  //防止头文件的重复包含和编译  配套#endif
#define __MY_DATABASE_H  //标识符
#include "stdint.h"

typedef struct
{
	uint8_t 	ID;//设备ID 1,2,3
	uint8_t 	Channel;//设备信道
	uint8_t 	rf_flag;//射频状态
	uint8_t 	Online;//在线标志 1在线 0离线
	uint16_t 	Offline_time;//没有收到433信号时间 超过多长时间433休眠 重量断电 角度休眠
	uint16_t 	RestTime;//静止时间
	uint8_t   TX_OK;//发送确保完成标志  1
	uint8_t 	GetAdc_time;//获得ADC时间
	uint8_t   GetAngle_time;//获得角度时间
	uint8_t 	ADC_D_value;//ADC变化参考值
	uint8_t 	Angel_D_value;//角度变化参考值
	uint8_t 	Bat_D_value;//电池电压变化参考值
	uint16_t 	Time_to_stop;//待机模式超时时间150s
	uint8_t   Option;//0水平安装 1垂直安装 010差分 001电流 
	uint16_t Led;
}LOW_POWER;
extern LOW_POWER low_power;

typedef struct
{
	uint8_t flag;
	int16_t acc[3];
	int16_t mag[3];
	float acc_float[3];
	float mag_float[3];
	float A_float[3];
	int8_t fh[3];
	uint16_t A[4][3];
	uint8_t A_Num;
	uint8_t err;//0正常1 LIS2DH12异常 2 QCM6309异常
}MY_Angle;
extern MY_Angle my_Angle;

typedef struct
{
	uint8_t Data_Send_Flag;//1数据准备 2数据可发送 
	uint8_t DI;//开关量
	uint8_t DI_Old;
	uint16_t adc;//信号值
	uint16_t adc_Old;//旧信号值
	uint16_t Angle[3];//角度
	uint16_t Angle_Old[3];
	uint16_t Angle_XYZ[3];
	uint16_t Bat_mv;//电池电压毫伏
	uint16_t Bat_mv_Old;//电池电量1-100
	uint16_t Temperature;//温度
	uint32_t OnlineTime;//在线时间
}MY_DATA;
extern MY_DATA my_data;


#endif

