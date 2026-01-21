#include "MyMainprogram.h"
#include "Data_Store.h"
//#include "rtc.h"
#include "string.h"
#include "mySensor.h"
#include "adc.h"
#include "tim.h"
#include <stdio.h>
#include "pan_rf.h"
#include "LIS2DH12.h"	

//休眠 0.05ma*3.7=0.19mw 工作0.8*3.7=3mW 发送峰值功耗 40*3.7=150mW 发送时间2ms

//BKP 32位  0~4
//#define IWDG_Flag 1 //  独立看门狗开启关闭标志
/*0-控制命令 1-模块高地址 2-模块低地址 3-（76-串口校验位，543-串口波特率，210-无线波特率） 
4-(43210-通信频率) 5-(7-定点发送使能 6-IO驱动发送 543-唤醒时间 210-发送功率)  */
/*参数保存，地址0101，波特率9600，8数据位1停止位无校验，空中速率2.5K,250ms唤醒，透明传输*/

//EEPROM  8位 0~1023 参数地址 大小  默认 数据说明

#define SET_OK					0   // 1  默认0xAC  设置过0xAC，设置前0x00
#define DEVICE_NUM  		1		// 1 	默认  1		设备编号1 2 3
#define MY_CHANNER 			2		// 1  默认  0		信道0-31
#define OPTION			  	3   // 1  默认  0   0水平安装 1垂直安装  010差分 100电流
#define TIME_STOP 			4		// 2  默认150 	待机超时时间0-65535 单位秒 
#define Adc_SING 				6		// 1  默认  6 	重量发送基准0-255          
#define Angle_SING			7		// 1	默认 50
#define Bat_SING 				8		// 1 	默认 36		电压发送基准0-255

#define SPARE1					9   //备用
#define SPARE2					10  //备用

#define PAIR_CHANNEL 146 //配对时

uint16_t Data_Num[10]={0,0,0,0,0,0,0,0,0,0};

//0 休眠 4 STB3 5 TX 6RX
void set_rf_STB3()
{
	while(rf_init());//初始化 将射频模式从深度睡眠模式切换到待机3（STB3）模式
	while(rf_set_default_para((uint32_t)(408+low_power.Channel)*1000000));//设置射频默认参数
	low_power.rf_flag=4;
}

//0-低功耗标识+从机地址 1-2重量 3-4 X 5-6 Y 7-8 Z 9-10电池 11-限位开关 9,10-校验
#define My_Data_Send_Len 17
uint8_t My_Data_Send[My_Data_Send_Len];
uint16_t CRC_Data;
extern struct RxDoneMsg RxDoneParams;

void Prepare_Data(void)
{
	if(low_power.rf_flag==0)
	{
	set_rf_STB3();
	low_power.rf_flag=4;
	}
	if(low_power.rf_flag<5){
//	while(rf_enter_continous_rx());//rf进入rx连续模式接收数据包
//	rf_enter_single_rx();//rf进入rx单模接收数据包
	rf_enter_single_timeout_rx(500);
	low_power.rf_flag=6;	
	}
	
	My_Data_Send[1] = RxDoneParams.Rssi; //RSSI 接收信号强度指示
	
	My_Data_Send[2]  = my_data.adc >> 8; 
	My_Data_Send[3]  = my_data.adc & 0xff; //重量信号   14
	
	My_Data_Send[4]  = my_data.Angle[0] >> 8; 
	My_Data_Send[5]  = my_data.Angle[0] & 0xff;//X     16
	
	My_Data_Send[6]  = my_data.Angle[1] >> 8; 
	My_Data_Send[7]  = my_data.Angle[1] & 0xff;//Y    16
	
	My_Data_Send[8]  = my_data.Angle[2] >> 8; 
	My_Data_Send[9]  = my_data.Angle[2] & 0xff;//Z     16
	
	My_Data_Send[10] = (Chrg<<7|my_data.DI<<6 | low_power.Option<<3 |my_data.Bat_mv >> 8); //充电状态+开关量+电压    1+1+3+10 
	My_Data_Send[11] = my_data.Bat_mv & 0xff; 
	
	My_Data_Send[12] = my_data.OnlineTime>>16;
	
	My_Data_Send[13] = my_data.OnlineTime>>8;
	My_Data_Send[14] = my_data.OnlineTime&0xff;
	
	CRC_Data=HAL_CRC_Calculate(&hcrc,(uint32_t *)My_Data_Send,My_Data_Send_Len-2);
	My_Data_Send[My_Data_Send_Len-2] = CRC_Data&0xff;
	My_Data_Send[My_Data_Send_Len-1] = CRC_Data>>8;
	my_data.Data_Send_Flag=2;
}

uint8_t i_adc;
uint32_t ADC_buff[4]={0};
uint16_t ADC_data[4];
uint8_t ADC_OK;
double ADC_JZ_K=1.0000f;

#define VREFINT_CAL	(uint16_t)(*(__I uint16_t *)(0x1FF80078)) //3V 1.218 1663

uint32_t tx_time;
uint32_t rx_num = 0;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	AI_12V_OFF;
	Weight_OFF;
	low_power.GetAdc_time=0;
	ADC_OK=1;	
			//ADC0 差分 ADC1 电流 ADC9 电压
		for(i_adc =0;i_adc<ADC_N;i_adc++)
		{
			ADC_buff[0] += ADC_value_buff[4*i_adc+0];
			ADC_buff[1] += ADC_value_buff[4*i_adc+1];
			ADC_buff[2] += ADC_value_buff[4*i_adc+2];
			ADC_buff[3] += ADC_value_buff[4*i_adc+3];
		}	
		//电流输入1~4
		ADC_JZ_K=(double)ADC_buff[3]/VREFINT_CAL/4;//1573 3.3
		ADC_data[0]=(double)ADC_buff[0]/ADC_JZ_K;//差分  									14位
		ADC_data[1]=(double)ADC_buff[1]/ADC_JZ_K;//电流 20mA*150欧姆				14位
		ADC_data[2]=(double)ADC_buff[2]/ADC_JZ_K;//*0.846f-1203;///1.529f;//1029  //ADC_buff[0]*3400/ADC_buff[3];///19.2*2;//电池电压1/1
		my_data.Bat_mv=ADC_data[2]/2.515f;//-1390;//电池电压  13位
		if(my_data.Bat_mv>4200)my_data.Bat_mv=4200;
		if(my_data.Bat_mv>3200)my_data.Bat_mv-=3200;//0-1000 10位
		else my_data.Bat_mv=0;
		
		if(low_power.Option&0x02)//启用差分
		{
			my_data.adc=ADC_data[0];//差分信号
		}
		else if(low_power.Option&0x04)
		{
			my_data.adc=ADC_data[1];//电流信号
		}
		ADC_buff[0] =0;
		ADC_buff[1] =0;
		ADC_buff[2] =0;
		ADC_buff[3] =0;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)  
{  
	if(GPIO_Pin == IRQ_Pin)
	{    
		rf_irq_process();//RF IRQ服务器例程，应在IRQ引脚的ISR处调用
	}
	if(GPIO_Pin == WAKE_UP_Pin)//空中信号
	{
		if(low_power.Online==2)low_power.Online=1;//如果离线 则待初始化
	}	
} 

void Setting(void)
{
	uint8_t Key_time;
	while(KEY)
	{
		LED_Toggle;
		HAL_Delay(100);
		Key_time++;
		HAL_IWDG_Refresh(&hiwdg);
		if(Key_time>=50)
		{
			LED_ON;
			while(rf_init());//初始化 将射频模式从深度睡眠模式切换到待机3（STB3）模式
			while(rf_set_default_para((uint32_t)(408+PAIR_CHANNEL)*1000000));//设置射频默认参数
			while(rf_enter_single_rx());//rf进入rx单模接收数据包
			uint8_t SetOK[5]={0xDD,0xDB,0x00,low_power.ID,0xDB};
			while(KEY)
			{
				HAL_IWDG_Refresh(&hiwdg);
			}
			my_data.OnlineTime=0;
			while(1)
			{
				if(my_data.OnlineTime>=300)break;
				HAL_IWDG_Refresh(&hiwdg);
				LED_Toggle;
				HAL_Delay(50);
				if(rf_get_recv_flag() == RADIO_FLAG_RXDONE)//获取接收标志==无线电标志RX完成
				{
					rf_set_recv_flag(RADIO_FLAG_IDLE);//设置接收标志 无线电标志空闲
					uint8_t CRC_2[2];
					CRC_Data=HAL_CRC_Calculate(&hcrc,(uint32_t *)RxDoneParams.Payload,RxDoneParams.Size-2);
					CRC_2[0]=CRC_Data&0xff;
					CRC_2[1]=CRC_Data>>8;
				if(CRC_2[0]==RxDoneParams.Payload[RxDoneParams.Size-2] && CRC_2[1]==RxDoneParams.Payload[RxDoneParams.Size-1])//校验数据
				{
					if(	 RxDoneParams.Payload[0] == SetOK[0]
						&& RxDoneParams.Payload[1] == SetOK[1]
						&& RxDoneParams.Payload[4] != SetOK[4]
					)//低功耗数据
					{
						Data_Num[0]++;
						SetOK[2]=RxDoneParams.Payload[2];//随机数
						if(rf_single_tx_data(SetOK,5, &tx_time)==FAIL)//rf进入单tx模式并发送数据包
						{
							//发送失败
							Data_Num[1]++;
							while(rf_enter_single_rx());//rf进入rx单模接收数据包
						}
						My_EEPROM_WriteLen(SET_OK,0xAC,1);
						My_EEPROM_WriteLen(DEVICE_NUM,RxDoneParams.Payload[3],1);//编号
						My_EEPROM_WriteLen(MY_CHANNER,RxDoneParams.Payload[4],1);//信道
						My_EEPROM_WriteLen(OPTION,RxDoneParams.Payload[5],1);//配置
						My_EEPROM_WriteLen(TIME_STOP,(RxDoneParams.Payload[6]<<8)|RxDoneParams.Payload[7],2);//休眠时间
						My_EEPROM_WriteLen(Adc_SING,RxDoneParams.Payload[8],1);//
						My_EEPROM_WriteLen(Angle_SING,RxDoneParams.Payload[9],1);//
						My_EEPROM_WriteLen(Bat_SING,RxDoneParams.Payload[10],1);//
					}
				}
				Data_Num[2]++;
				}
				if(rf_get_transmit_flag() == RADIO_FLAG_TXDONE)//传输完成
				{
					rf_set_transmit_flag(RADIO_FLAG_IDLE);
					LED_Toggle;
					HAL_Delay(100);
							LED_Toggle;
					HAL_Delay(100);
							LED_Toggle;
					HAL_Delay(100);
							LED_Toggle;
					HAL_Delay(100);
							LED_Toggle;
					HAL_Delay(100);
					Set_Standby();
				}
			}
		}
	}
	LED_OFF;
}

void My_Init(void)
{
	Angle_OFF;
//读取EEPROM存储参数
	if(My_EEPROM_ReadLen(SET_OK,1)==0xAC)//设置过
	{
		low_power.ID						=	My_EEPROM_ReadLen(DEVICE_NUM ,1);//设备编号1,2,3
		low_power.Channel				=	My_EEPROM_ReadLen(MY_CHANNER ,1);//信道0-31
		low_power.Option				=	My_EEPROM_ReadLen(OPTION,1);//0水平安装 1垂直安装  010差分 100电流
		low_power.Time_to_stop	=	My_EEPROM_ReadLen(TIME_STOP	,2);//待机模式超时时间
		low_power.ADC_D_value		=	My_EEPROM_ReadLen(Adc_SING			,1);//重量发送基准0-255
		low_power.Angel_D_value	=	My_EEPROM_ReadLen(Angle_SING,1);//角度发送基准0-255
		low_power.Bat_D_value		=	My_EEPROM_ReadLen(Bat_SING		,1);//电压发送基准0-255
		
		if(low_power.Channel>142)low_power.Channel=142;
		if(low_power.Time_to_stop<=60)low_power.Time_to_stop=60;//休眠时间限制
	}else
	{
		low_power.ID=1;
		low_power.Channel=PAIR_CHANNEL;//配对频率146 408+N
		low_power.Option=0x00;
		low_power.Time_to_stop=120;//休眠时间
		low_power.ADC_D_value=20;
		low_power.Angel_D_value=50;
		low_power.Bat_D_value=50;
	}
	My_Data_Send[0]=low_power.ID|0xD0;//从机地址 0xD1,0xD2,0XD3
	
	Setting();
	set_rf_STB3();//从休眠到STB3
//	rf_enter_single_timeout_rx(300);
//	rf_enter_continous_rx();
	while(rf_enter_single_rx());//rf进入rx单模接收数据包
	LED_ON;

	low_power.rf_flag=6;

	low_power.GetAdc_time=0;
	my_Angle.err=3;
}

void Get_adc_Data()
{
	if(low_power.Option&0x04)
	{
		AI_12V_ON;
	}
	if(low_power.Option>>1)//启用差分
	{
		Weight_ON;//K1
	}
	low_power.GetAdc_time=0;
	HAL_ADC_Start_DMA(&hadc,(uint32_t *)ADC_value_buff,ADC_N*4);
	ADC_OK=0;
}

void Set_Standby()
{
	HAL_IWDG_Refresh(&hiwdg);
	while(rf_deepsleep());//初始化从 待机3（STB3）更改为深度睡眠
	low_power.rf_flag=0;//状态为休眠
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);//
	HAL_SuspendTick();	//暂停滴答时钟，防止通过滴答时钟中断唤醒
	HAL_PWR_EnterSTANDBYMode();//进入待机模式
}

#include "gpio.h"
void Clear_Offline(void)
{
	if(low_power.Online==1)//之前离线
	{
		Angle_ON;//角度通电
		LED_OFF;
		MX_ADC_Init();

		LSM_Init();//角度传感器初始化
		QCM_Init();
		
		my_data.OnlineTime=0;
		my_Angle.A_Num=0;
		low_power.GetAngle_time=0;
		low_power.Online=0;//在线标志 0在线 1离线
		my_data.Data_Send_Flag=1;
	}
}

void My_Program(void)
{
	Setting();
	Clear_Offline();
	//////////////////////////////////////////////////////////////////////////////
		//射频相关开始
	if(rf_get_recv_flag())
		{
			if(rf_get_recv_flag() == RADIO_FLAG_RXDONE)//获取接收标志==无线电标志RX完成
			{
				rf_set_recv_flag(RADIO_FLAG_IDLE);//设置接收标志 无线电标志空闲
				low_power.rf_flag=4;
				//RxDoneParams.Payload 接收的数组 RxDoneParams.Size 接收的长度
				if(RxDoneParams.Payload[0]==0xDD)//帧头正确
				{
					if(RxDoneParams.Payload[1]>>4==low_power.ID)//询问我了
					{
						if(my_data.Data_Send_Flag==2)//数据准备好
						{
							if(rf_single_tx_data(My_Data_Send,My_Data_Send_Len, &tx_time)==OK)//rf进入单tx模式并发送数据包
							{
								low_power.rf_flag=5;
								low_power.TX_OK=1;
								my_data.Data_Send_Flag=0;
							}else	my_data.Data_Send_Flag=1;//发送失败
							low_power.Offline_time=0;//离线延时时间清零
						}
					}
					else //没有轮询到我，接着接收
					{
						rf_enter_single_timeout_rx(500);
						low_power.rf_flag=6;
					}
					if((RxDoneParams.Payload[1]&0x0f)==low_power.ID)//收到了
					{
						low_power.TX_OK=0;
					}
				}
			}
			else if((rf_get_recv_flag() == RADIO_FLAG_RXTIMEOUT) || (rf_get_recv_flag() == RADIO_FLAG_RXERR))//rx超时或rx-err标志
			{
				rf_set_recv_flag(RADIO_FLAG_IDLE);//设置接收标志
				low_power.rf_flag=4;
				if(my_data.Data_Send_Flag==2){
					
				rf_enter_single_timeout_rx(500);
				low_power.rf_flag=6;
				}
			}
	}
	if(rf_get_transmit_flag() == RADIO_FLAG_TXDONE)//传输完成
	{
		rf_set_transmit_flag(RADIO_FLAG_IDLE);
//		rf_enter_single_rx();//rf进入rx单模接收数据包
//		rf_enter_continous_rx();//rf进入rx连续模式接收数据包
		rf_enter_single_timeout_rx(500);
		low_power.rf_flag=6;

	}
	////////////////////////////////////////////////////////////////////////
	//433在线标志 OK
	if(low_power.Online==2)//接收器离线
	{ 
		if(low_power.GetAdc_time>2)//100ms*3
		{
			Set_Standby();
		}
	}
		//读取重量角度数据
	else//在线
	{
	//获取角度数据，如果角度大于参考值则读取重量
		if(AngleChange(my_data.Angle[0],my_data.Angle_Old[0])>=low_power.Angel_D_value 
		|| AngleChange(my_data.Angle[1],my_data.Angle_Old[1])>=low_power.Angel_D_value
//		|| AngleChange(my_data.Angle[2],my_data.Angle_Old[2])>=low_power.Angel_D_value
		)
	{
		my_data.Angle_Old[0]=my_data.Angle[0];
		my_data.Angle_Old[1]=my_data.Angle[1];
		my_data.Angle_Old[2]=my_data.Angle[2];
		Get_adc_Data();//获取ADC数据
		my_data.Data_Send_Flag=1;//可以准备数据
	}
	//信号值变化、开关量变化
	if((abs(my_data.adc - my_data.adc_Old)>= low_power.ADC_D_value) 
		|| (my_data.DI != my_data.DI_Old)
		)
		{
			my_data.adc_Old					=my_data.adc;
			my_data.DI_Old					=my_data.DI;
			my_data.Data_Send_Flag	=1;//可以准备数据
			low_power.RestTime			=0;//获取数据省电模式计时清零
		}
	else if((low_power.TX_OK>=5)||(abs(my_data.Bat_mv_Old	- my_data.Bat_mv)>=low_power.Bat_D_value))//没有得到反馈或者数据接收失败
		{
			my_data.Bat_mv_Old	= my_data.Bat_mv;
			low_power.TX_OK=0;
			my_data.Data_Send_Flag=1;
		}
	//判断在线离线
	if(low_power.Offline_time>low_power.Time_to_stop+2)//大于离线时间，rf进入定时接收模式
	{
		my_data.Data_Send_Flag=0;
		low_power.Online=2;//离线了
		//进入待机模式
	}
	else if(low_power.Offline_time==low_power.Time_to_stop-4)//倒数10s 赶紧发数据，保活
	{
		my_data.Data_Send_Flag=1;//数据准备
		low_power.Offline_time++;
		low_power.rf_flag=0;//状态为休眠
	}
	else if(low_power.Offline_time==low_power.Time_to_stop-10)//倒数10s 赶紧发数据，保活
	{
		low_power.Offline_time++;
		my_data.Data_Send_Flag=1;//数据准备
	}
	else if((low_power.Offline_time>=3)&&(my_data.Data_Send_Flag==0)&&low_power.rf_flag)//大于3s，且无数据发送，rf进入休眠模式
	{
		while(rf_deepsleep());//初始化从 待机3（STB3）更改为深度睡眠
		low_power.rf_flag=0;//状态为休眠
	}
	if(my_data.Data_Send_Flag==1)
	{
		Prepare_Data();//准备数据
	}
	else if(ADC_OK==1 && low_power.rf_flag==0)//ADC获取成功 而且rf进入休眠模式
	{
		HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);//睡眠
	}
	if( low_power.RestTime<=10)//10s内,正常获取数据 100ms
		{
			if(low_power.GetAdc_time>8)//0.8s
			{
				Get_adc_Data();
			}
		}
	else if(low_power.RestTime<=300)//5分
		{
		if(low_power.GetAdc_time>100)//10s
			{
				Get_adc_Data();
			}
		}
	else if(low_power.RestTime>=421)
		{
			Get_adc_Data();
			low_power.RestTime=301;//2分钟1次
		}
		
	if(low_power.GetAngle_time>=250)
	{
		low_power.GetAngle_time=0;
		Get_Angle();//获得角度
	}else low_power.GetAngle_time++;
	my_data.DI=ReadDI;//获取开关量
	
	if(low_power.Led>=32)
	{
		LED_OFF;
		low_power.Led=0;
	}else if(low_power.Led>=30)
	{
		LED_ON;
	}
}
}

/*定时器回调函数*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == htim21.Instance)//1S
  {
		my_data.OnlineTime++;
		low_power.Offline_time++;
		low_power.RestTime++;
  }
  if (htim->Instance == htim22.Instance)//100ms
  {
		low_power.GetAdc_time++;//读取ADC时间
		if(low_power.TX_OK)low_power.TX_OK++;
		low_power.Led++;
  } 
}



