//#include "LSM6DS3TR-C.h"
//#include "mySensor.h"
//#include "my_Database.h"
//#include "spi.h"
//#define ID 0x6A

//#define LSM6DS3TRC_WHO_AM_I		0x0F
//#define LSM6DS3TRC_CTRL3_C		0x12

//uint8_t* rx_spi;

//uint8_t LSM_GetID()
//{
//	uint8_t tryCount=0;
//	while(tryCount<5){
//		rx_spi=Read_SPI(LSM6DS3TRC_WHO_AM_I,1);
//		if(*rx_spi==ID)return SUCCESS;
//		delay_us(10);
//		LSM_Reset();//复位
//		delay_us(10);
//		tryCount++;
//	}
//	return ERROR;
//}

//uint8_t LSM_Reset()//复位
//{
//	*rx_spi=0x80;
//	Write_SPI(LSM6DS3TRC_CTRL3_C,rx_spi,1);//Boot->1
//	HAL_Delay(15);
//	rx_spi=Read_SPI(LSM6DS3TRC_CTRL3_C,1);
//	*rx_spi|=0x01;
//	Write_SPI(LSM6DS3TRC_CTRL3_C,rx_spi,1);//Reset->1
//	while(*rx_spi &0x01)
//	rx_spi=Read_SPI(LSM6DS3TRC_CTRL3_C,1);
//}

////1 数据锁定 0数据更新
////void LSM_Set_BDU(uint8_t flag)
////{
////	rx_spi=Read_SPI(LSM6DS3TRC_CTRL3_C,1);
////	if(flag)
////	{
////		*rx_spi |= 0x40;//启用BDU
////		Write_SPI(LSM6DS3TRC_CTRL3_C, rx_spi, 1);
////	}
////	else
////	{
////		*rx_spi &= 0xbf;//禁用BDU
////		Write_SPI(LSM6DS3TRC_CTRL3_C, rx_spi, 1);
////	}
////	rx_spi=Read_SPI(LSM6DS3TRC_CTRL3_C,1);
////}

//void LSM_Set_u8(uint8_t Addr,uint8_t rate)
//{
//	rx_spi=Read_SPI(Addr,1);
//	*rx_spi |= rate;
//	Write_SPI(Addr, rx_spi, 1);
//	delay_us(5);
//}

//void LSM_Init()
//{
//	uint8_t Err_Flag;
////	delay_us(100);
//	LSM_GetID();//获取失败
//	
///*加速度计控制寄存器 0x10
//	7-4	ODR_XL[3:0]	加速度计输出数据率和功耗模式选择，
//				默认：0x00-关机 0xB0-1.6HZ 0x10-12.5HZ 0x20-26HZ 0x30-52HZ 0x40-104HZ 0x50-208HZ
//	3-2	FS_XL[1:0]	加速度计量程选择，默认00：±2g，01：±16g，10：±4g，11：±8g
//	1		LPF1_BW_SEL	数字低通滤波器1带宽选择 参考CTRL8_XL的具体配置
//	0		BW0_XL	模拟模拟带宽选择，仅当ODR≥1.67kHz有效，0：1.5kHz，1：400Hz*/
//	LSM_Set_u8(0x10,0x10);
//	LSM_Set_u8(0x15,0x10);//低功耗模式
//	
///*加速度计的数字滤波器 0x17
//	7		LPF2_XL_EN	加速度计第2级数字低通滤波器LPF2选择，使能后滤波器结构改变，影响带宽和噪声特性。 0x80
//	6-5	HPCF_XL[1:0]	加速度计高通滤波器 cutoff 频率设置及LPF2配置选择，影响高通滤波器特性。0x20
//	4		HP_REF_MODE	高通滤波器参考模式使能，开启后首次输出数据需丢弃，可稳定输出。默认0关闭 0x00
//	3		INPUT_COMPOSITE	复合滤波器输入选择，0为ODR/2的低通滤波数据，1为ODR/4的低通滤波数据，影响滤波路径。0x00
//	2		HP_SLOPE_XL_EN	斜率滤波器/高通滤波器选择.0x00
//	0		LOW_PASS_ON_6D	低通滤波器是否作用于6D功能（姿态方向检测专用功能），通常使用默认即可。
//	*/
//	LSM_Set_u8(0x17,0x88);
//	
//	//陀螺仪
//	//断电 量程 ±2g
//	LSM_Set_u8(0x11,0x00);
//	LSM_Set_u8(0x13,0x40);//睡眠模式
//	LSM_Set_u8(0x16,0x80);//低功耗模式
//	
//	lsm_Angle.flag=0xff;
//	lsm_Angle.Angle_C_Num=0;
//}


//void LSM_Get_Status()
//{
//	Read_SPI_DMA(0x1E,1);
//	lsm_Angle.flag=1;//读取是否有加速度数据
//}

//#include <math.h>
//#include <stdio.h> 

//void LSM_Get_Angle()
//{
//	LSM_Get_Status();//读取是否有数据
//if(lsm_Angle.flag==10)
//	{
//		lsm_Angle.flag=0;
//		lsm_Angle.acc_float[0]=(float)lsm_Angle.acc[0]*0.061f;
//		lsm_Angle.acc_float[1]=(float)lsm_Angle.acc[1]*0.061f;
//		lsm_Angle.acc_float[2]=(float)lsm_Angle.acc[2]*0.061f;
//		
////		lsm_Angle.gry_float[0]=(float)lsm_Angle.gry[0]*8.750f;
////		lsm_Angle.gry_float[1]=(float)lsm_Angle.gry[1]*8.750f;
////		lsm_Angle.gry_float[2]=(float)lsm_Angle.gry[2]*8.750f;
//		
//		lsm_Angle.Angle_C[lsm_Angle.Angle_C_Num][0] = atan2_approx(lsm_Angle.acc_float[0], sqrtf(lsm_Angle.acc_float[1]*lsm_Angle.acc_float[1]+lsm_Angle.acc_float[2]*lsm_Angle.acc_float[2])) * 57.295779f;
//		lsm_Angle.Angle_C[lsm_Angle.Angle_C_Num][1] = atan2_approx(lsm_Angle.acc_float[1], sqrtf(lsm_Angle.acc_float[0]*lsm_Angle.acc_float[0]+lsm_Angle.acc_float[2]*lsm_Angle.acc_float[2])) * 57.295779f;
//		lsm_Angle.Angle_C[lsm_Angle.Angle_C_Num][2] = -atan2_approx(lsm_Angle.acc_float[0],lsm_Angle.acc_float[1])*57.295779f;
//		lsm_Angle.Angle_C_Num++;
//		if(lsm_Angle.Angle_C_Num>=2)
//		{
//			lsm_Angle.Angle_C_Num=0;
//			my_data.Angle[0]=(lsm_Angle.Angle_C[0][0]+lsm_Angle.Angle_C[1][0]+lsm_Angle.Angle_C[2][0])/3*100;
//			my_data.Angle[1]=(lsm_Angle.Angle_C[0][1]+lsm_Angle.Angle_C[1][1]+lsm_Angle.Angle_C[2][1])/3*100;
//			my_data.Angle[2]=(lsm_Angle.Angle_C[0][2]+lsm_Angle.Angle_C[1][2]+lsm_Angle.Angle_C[2][2])/3*100;
//		}
//	}
//}

//// 一个简单的 atan2 近似算法
//float atan2_approx(float y, float x) {
//    const float PI = 3.14159265358979323846f;
//    float abs_y = fabsf(y);
//    float angle;
//    float z;
//    // 处理 x 为 0 的特殊情况
//    if (x == 0.0f) {
//        if (y > 0.0f) {
//            return PI / 2.0f;
//        } else if (y < 0.0f) {
//            return -PI / 2.0f;
//        } else {
//            return 0.0f;
//        }
//    }
//    z = abs_y / fabsf(x);
//    // 使用更高阶多项式近似计算角度
//    if (z < 1.0f) {
//        angle = z - (z * z * z) / 3.0f + (z * z * z * z * z) / 5.0f - (z * z * z * z * z * z * z) / 7.0f + 
//                (z * z * z * z * z * z * z * z * z) / 9.0f - (z * z * z * z * z * z * z * z * z * z * z) / 11.0f;
//    } else {
//        angle = PI / 2.0f - (1.0f / z) + (1.0f / (3.0f * z * z * z)) - (1.0f / (5.0f * z * z * z * z * z)) + 
//                (1.0f / (7.0f * z * z * z * z * z * z * z)) - (1.0f / (9.0f * z * z * z * z * z * z * z * z * z)) + 
//                (1.0f / (11.0f * z * z * z * z * z * z * z * z * z * z * z));
//    }
//    // 根据 x 和 y 的正负调整角度
//    if (x < 0.0f) {
//        angle = (y < 0.0f) ? -(PI - angle) : (PI - angle);
//    } else {
//        angle = (y < 0.0f) ? -angle : angle;
//    }
//    return angle;
//}  