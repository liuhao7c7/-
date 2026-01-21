#include "LIS2DH12.h"
#include "mySensor.h"
#include "my_Database.h"
#include "i2c.h"
#include "iwdg.h"
//更改为LIS2DH12
#define LIS2DH12_ADDR 0x30 //0x18<<1
#define LIS2DH12_ID 0x33

#define QCM6309_ADDR 0x7C<<1
#define QCM6309_ID 0x90


uint8_t QCM_GetID()
{
	uint8_t retry=10;
	do{
		if(Read_I2C_u8(QCM6309_ADDR,0x00)==QCM6309_ID)return SUCCESS;
		HAL_Delay(5);
	}while(--retry>0);
	return ERROR;
}

void QCM_Init()
{
	if(QCM_GetID())my_Angle.err|=0x02;//获取失败
	else{
		my_Angle.err&=0xfd;
	Write_I2C_u8(QCM6309_ADDR,0x0B,0x18);//定义设置/重启模式，打开设置/重置，字段范围8Guass，10HZ
	Write_I2C_u8(QCM6309_ADDR,0x0A,0x61);//正常模式 OSR1=8 OSR2=8
	}
}

uint8_t LSM_GetID()
{
	uint8_t retry=10;
	do{
		if(Read_I2C_u8(LIS2DH12_ADDR,0x0f)==LIS2DH12_ID)return SUCCESS;
		HAL_Delay(5);
	}while(--retry>0);
	return ERROR;
}

void LSM_Init()
{
	if(LSM_GetID())my_Angle.err|=0x01;//获取失败
	else{
		my_Angle.err&=0xfe;
/*加速度计控制寄存器 0x20
	7-4	ODR_XL[3:0]	加速度计输出数据率和功耗模式选择，
			0x00-关机 0x10-1HZ 0x20-10HZ 0x30-25HZ 默认：0x00
	3   0高分辨率、正常模式 1低功耗模式 默认0
	2		Z启用 1启用 0禁用 默认1
	1		Y启用 1启用 0禁用 默认1
	0		X启用 1启用 0禁用 默认1
	
	0x23
	7		0持续更新 1读取MSB和LSB前不更新 默认0
	6		0低数据LSB在低地址 1高数据MSB在低地址 默认0
	5-4 FS_XL[1:0]	加速度计量程选择，
			00：±2g，01：±4g，10：±8g，11：±16g 默认00
	3   HR 操作模式选择
	2-1 自检启用 00正常模式 01自检0 10自检1 默认00
	0		SPI模式 0四线 1三线
	
	0x24
	7 Boot 0正常模式 1重启内存内容 默认0
	6 FiFO 0禁用 1启用 默认0
	3 LIR_INT1 (31H)锁存中断请求 0中断请求未锁定 1锁定 默认0
	
	1 LIR_INT2 (35H)锁存中断请求 0中断请求未锁定 1锁定 默认0
	
	0x27 3 XYZ有新数据 0没有 1有 2-Z 1-Y 0-Z  读（0x08）
	
	0x28-0x29 X加速度 0x2A-0x2B Y加速度 0x2C-0x2D Z加速度
*/
	Write_I2C_u8(LIS2DH12_ADDR,0x20,0x27);//00100111 10HZ
//	Write_I2C_u8(LIS2DH12_ADDR,0x23,0x00);//00000000
//	Write_I2C_u8(LIS2DH12_ADDR,0x24,0x00);//0000
	}
}



void LSM_Get_Status()
{
	if(Read_I2C_u8(LIS2DH12_ADDR,0x27)&0x03)
	{
		Read_I2C(LIS2DH12_ADDR,0x80|0x28,(uint8_t*)my_Angle.acc,6);
		my_Angle.flag=1;
	}
}

void QCM_Get_Status()
{
	if(Read_I2C_u8(QCM6309_ADDR,0x09))
	{
		Read_I2C(QCM6309_ADDR,0x01,(uint8_t*)my_Angle.mag,6);
	}
}

#include <math.h>
#include <stdio.h> 
#include <stdlib.h> 

uint16_t Angle_Average(uint16_t a,uint16_t b)//角度平均值计算
{
    uint16_t angle;
    if(abs(a - b)>18000){angle=(a+b)/2+18000;}
    else{angle=(uint32_t)(a+b)/2;}
    if(angle>=36000){angle-=36000;}
    return angle;
}

uint16_t Angle_Covert(float x,float y,float Angle_f)
{
	uint16_t Angle;
	if(x<0)Angle=18000-Angle_f*100.0f;//2、3象限
	else if(x>0 && Angle_f<0)Angle=36000+Angle_f*100.0f;//0-36000 +- 4象限
	else Angle=Angle_f*100.0f;//1 ++
	return Angle;
}

uint8_t get_num=0;
void Get_Angle()
{
	get_num=!get_num;
	if(get_num)
	LSM_Get_Status();//读取是否有数据
	else
	QCM_Get_Status();//读取磁传感器数据
	
if(my_Angle.flag)
	{
		my_Angle.flag=0; 
		if(low_power.Option&0x01)//垂直安装
		{
			my_Angle.acc_float[0]=(float)-my_Angle.acc[1]*0.061f;//0.1019f;
			my_Angle.acc_float[1]=(float)-my_Angle.acc[2]*0.061f;//0.1019f;
			my_Angle.acc_float[2]=(float) my_Angle.acc[0]*0.061f;//0.1019f;
		}
		else //水平安装
		{
			my_Angle.acc_float[0]=(float)-my_Angle.acc[1]*0.061f;//0.1019f;
			my_Angle.acc_float[1]=(float) my_Angle.acc[0]*0.061f;//0.1019f;
			my_Angle.acc_float[2]=(float) my_Angle.acc[2]*0.061f;//0.1019f;
		}
		
		my_Angle.A_float[0] = atan2f( my_Angle.acc_float[1], sqrtf(my_Angle.acc_float[0]*my_Angle.acc_float[0]+my_Angle.acc_float[2]*my_Angle.acc_float[2])) * 57.29578f;//X
		my_Angle.A_float[1] = atan2f(-my_Angle.acc_float[0], sqrtf(my_Angle.acc_float[1]*my_Angle.acc_float[1]+my_Angle.acc_float[2]*my_Angle.acc_float[2])) * 57.29578f;//Y
		
		//xxxxxxxx
		//1- 0 1+ 2+ 0-90  	0-90      x=x         y=90    0+ 1+ 0-90   y=180  
		//2- 0 1+ 2- 90-0		90-180		x=180-x							0- 1+ 90-0
		//3- 0 1- 2- 0--90	180-270		x=180-x							0- 1- 0--90
		//4- 0 1- 2+ -90-0  270-360		x=360+x								0+ 1+
		
		//yyyyyyyy
		//1- 0+ 1 2+ 0-90  	0-90      x=x					x=90   0+ 1+
		//2- 0+ 1 2- 90-0		90-180		x=180-x							0+ 1-
		//3- 0- 1 2- 0--90	180-270		x=180-x
		//4- 0- 1 2+ -90-0  270-360		x=360+x

		//XZ 0-360 Y ±90 0-90 270-360
		my_Angle.A[my_Angle.A_Num][0]=Angle_Covert(my_Angle.acc_float[2],my_Angle.acc_float[1],my_Angle.A_float[0]);
		my_Angle.A[my_Angle.A_Num][1]=my_Angle.A_float[1]>=0.0f ? my_Angle.A_float[1]*100 : 36000+my_Angle.A_float[1]*100.0f;//Angle_Covert(my_Angle.acc_float[2],my_Angle.acc_float[0],my_Angle.A_float[1]);

		if(low_power.Option&0x01)//垂直安装
		{
		my_Angle.mag_float[0]=-(float)my_Angle.mag[0]/4000.f;//9810/4000
		my_Angle.mag_float[1]=-(float)my_Angle.mag[2]/4000.f;
		my_Angle.mag_float[2]=-(float)my_Angle.mag[1]/4000.f;
		}
		else //水平安装
		{
		my_Angle.mag_float[0]=-(float)my_Angle.mag[0]/4000.f;//9810/4000
		my_Angle.mag_float[1]=-(float)my_Angle.mag[1]/4000.f;
		my_Angle.mag_float[2]=(float)	my_Angle.mag[2]/4000.f;
		}
		 // 倾斜补偿(使用加速度计算的俯仰角和横滚角)
    float pitch = my_Angle.A_float[0] * 3.14159f / 180.0f;  // 弧度
    float roll  = my_Angle.A_float[1] * 3.14159f / 180.0f;   // 弧度
		
				// 补偿加速度引起的倾斜
    float mx = my_Angle.mag_float[0];
    float my = my_Angle.mag_float[1];
    float mz = my_Angle.mag_float[2];
			
		float mx_comp, my_comp;
		//常规补偿
		mx_comp = mx * cosf(pitch) + mz * sinf(pitch);
		my_comp = mx * sinf(roll) * sinf(pitch) + my * cosf(roll) - mz * sinf(roll) * cosf(pitch);
		
    // 计算偏航角(0-360度)
    float yaw_rad = atan2f(-my_comp, mx_comp);
    my_Angle.A_float[2] = yaw_rad * 57.295779f;
    
    // 确保偏航角在0-360度范围内
    if (my_Angle.A_float[2] < 0) {
        my_Angle.A_float[2] += 360.0f;
    }
		my_Angle.A[my_Angle.A_Num][2]=my_Angle.A_float[2]*100;
		  //x+->Z+ y+-
		my_Angle.A_Num++;
		if(my_Angle.A_Num>=4)
		{
			my_Angle.A_Num=0;
		}
			my_data.Angle[0]=Angle_Average(Angle_Average(my_Angle.A[0][0],my_Angle.A[1][0]),Angle_Average(my_Angle.A[2][0],my_Angle.A[3][0]));
			my_data.Angle[1]=Angle_Average(Angle_Average(my_Angle.A[0][1],my_Angle.A[1][1]),Angle_Average(my_Angle.A[2][1],my_Angle.A[3][1]));
			my_data.Angle[2]=Angle_Average(Angle_Average(my_Angle.A[0][2],my_Angle.A[1][2]),Angle_Average(my_Angle.A[2][2],my_Angle.A[3][2]));
	}
}

uint16_t AngleChange(uint16_t AngleNew,uint16_t AngleOld)
{
	uint16_t Data=abs(AngleNew-AngleOld);
	return(Data>18000)?(36000-Data):Data;
}