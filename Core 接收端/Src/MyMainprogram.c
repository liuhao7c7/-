#include "MyMainprogram.h"
#include "Data_Store.h"
#include "string.h"
#include "mySensor.h"
#include <stdio.h>
#include "pan_rf.h"
#include "oled.h"

extern struct RxDoneMsg RxDoneParams;
//BKP 32位  0~4
//#define IWDG_Flag 1 //  独立看门狗开启关闭标志
/*0-控制命令 1-模块高地址 2-模块低地址 3-（76-串口校验位，543-串口波特率，210-无线波特率） 
4-(43210-通信频率) 5-(7-定点发送使能 6-IO驱动发送 543-唤醒时间 210-发送功率)  */
/*参数保存，地址0101，波特率9600，8数据位1停止位无校验，空中速率2.5K,250ms唤醒，透明传输*/

//EEPROM  8位 0~1023 参数地址 大小  默认 数据说明

#define SET_OK					0   // 1  默认0xAC  设置过0xAC，设置前0x00
#define DEVICE_NUM  		1		// 1 	默认  1		低功耗个数
#define MY_CHANNER 			2		// 1  默认  0		信道0-31
#define TIME_STOP 			3		// 2  默认150 	待机超时时间0-65535 单位秒 
#define Adc_SING 				5		// 1  默认  6 	重量发送基准0-255          
#define Bat_SING 				6		// 1 	默认 36		电压发送基准0-255
#define Angle_SING			7		// 1	默认 50		电压发送基准0-255
#define OPTION				  11  // 15  默认  0  0水平安装 1垂直安装 010差分 001电流 

#define SPARE2					9  //备用
#define MODBUS_ADDR			10 //1 当前modbusRTU地址

#define PAIR_CHANNEL 146 //配对时

#define VREFINT_CAL	(uint16_t)(*(__I uint16_t *)(0x1FF80078)) //3V 1.218 1663

uint8_t	Key_value=0,Key_Cache,Key_Flag=0;
uint16_t CRC_Data;
uint32_t tx_time;
uint8_t Show_Time;

#define DATA_TX_DELAY 150

uint8_t Read_EppromData(void)
{
	//读取EEPROM存储参数
	if(My_EEPROM_ReadLen(SET_OK,1)==0xAC)//设置过
	{
		my_433.err=0xfffe;//0本机1-15 低功耗
		for(uint8_t i=0;i<15;i++)
		{
		my_433.Device[i].LowOption	= My_EEPROM_ReadLen(OPTION+i,1)&0x0f;//0水平安装 1垂直安装 010差分 001电流 
		}
		my_433.Channel				=	My_EEPROM_ReadLen(MY_CHANNER ,1);//信道0-31
		my_433.Time_to_stop	  =	My_EEPROM_ReadLen(TIME_STOP	,2);//待机模式超时时间
		my_433.ADC_D_value		=	My_EEPROM_ReadLen(Adc_SING			,1);//重量发送基准0-255
		my_433.Bat_D_value		=	My_EEPROM_ReadLen(Bat_SING		,1);//电压发送基准0-255
		my_433.Angel_D_value	=	My_EEPROM_ReadLen(Angle_SING,1);
		if(my_433.Channel>142)my_433.Channel=142;
		if(my_433.Time_to_stop<=60)my_433.Time_to_stop=60;//休眠时间限制
		return 1;
	}
		my_433.err=0xffff;
		my_433.Channel=PAIR_CHANNEL;//配对频率555
		my_433.Time_to_stop=120;
		my_433.ADC_D_value=20;
		my_433.Bat_D_value=50;
		my_433.Angel_D_value=50;
	return 0;
}


void LowPower_Pair(uint16_t Pair_Flag)//设备配对 修改地址+信道(0按键进入-改Num，1远程修改-Num不变)
{
	uint8_t SetOK[14]={0xDD,0xDB};
	uint8_t	SendFlag=0;
//	SetOK[4]	=	my_433.ID;//编号1 2
	SetOK[2]  = Show_Time;//随机数
//SetOK[3]=Menu_Option+1;//编号
	SetOK[4]	=	my_433.Channel;//信道
//	SetOK[5]	=	my_433.Option;//0水平安装 1垂直安装 010差分 001电流 
	SetOK[6]	=	my_433.Time_to_stop>>8;//
	SetOK[7]	=	my_433.Time_to_stop&0xff;//休眠时间
	SetOK[8]	=	my_433.ADC_D_value;//重量发送基准
	SetOK[9]	=	my_433.Angel_D_value;//角度变化参考值
	SetOK[10]	=	my_433.Bat_D_value;//电压发送基准
	SetOK[11] = 0;//备用
	
	while(rf_init());//初始化 将射频模式从深度睡眠模式切换到待机3（STB3）模式
	while(rf_set_default_para((uint32_t)(408+PAIR_CHANNEL)*1000000));//设置射频默认参数
	my_433.rf_flag=4;
	
	uint8_t Menu_Option=0,Option,Display=1,Oled_Y=2,My_Switch=my_433.Number;
	Key_value=0;
	OLED_Clear();
	while(1)
		{
		HAL_IWDG_Refresh(&hiwdg);
		if(Key_value == 0x01)//确认键
			{
				Key_value=0;
				OLED_ShowString(64,2,"=>");
				OLED_Show_Str_CHinese(80,2,"配对中");
				SetOK[3]=Menu_Option+1;//编号
				SetOK[5]=	my_433.Device[Menu_Option].LowOption;////0水平安装 1垂直安装 010差分 001电流 
				RTU_Send_CRC16(SetOK,14);
				SendFlag=1;
				Display=1;
				}//闭合复位
		else if(Key_value == 0x02)//选择
			{
				Key_value=0;
				SendFlag=0;
				OLED_ShowString(64,2,"        ");
				Menu_Option++;//按选择键界面显示，+1
				Display=1;
			}
		else if(Key_value == 0x04  && Pair_Flag==0)//返回
			{
				Key_value=0;
				OLED_Clear();
				break;
			}
		if(Display)
		{
			Display=0;	
			OLED_Show_Str_CHinese(0,0,"配对模式");
			OLED_ShowString(0,2,"-->");
			if(Menu_Option>=My_Switch){Menu_Option = 0;}
			Option=Menu_Option;
			for(uint8_t ii=0;ii<(My_Switch>3?3:My_Switch);ii++)
			{
				OLED_Show_Str_CHinese(24,Oled_Y,"低");
				OLED_ShowNum(40,Oled_Y,Option+1,2,16);
				Oled_Y+=2;
				Option++;
				if(Option>=My_Switch){Option=0;}
			}
			Oled_Y=2;
		}
		if(SendFlag)
			{
				if(my_433.Send_flag>=DATA_TX_DELAY)
				{
					if(rf_single_tx_data(SetOK, 14, &tx_time) != OK)//rf进入单tx模式并发送数据包
					{
				#ifdef DEBUG
						My_Printf("发送异常",8);//tx失败
						#endif
					}
					my_433.Send_flag=0;
				}
				if(rf_get_recv_flag() == RADIO_FLAG_RXDONE)//获取接收标志==无线电标志RX完成
				{
					my_433.rf_flag=4;
					rf_set_recv_flag(RADIO_FLAG_IDLE);//设置接收标志 无线电标志空闲
					if(	 RxDoneParams.Payload[0] == SetOK[0]
						&& RxDoneParams.Payload[1] == SetOK[1]
						&& RxDoneParams.Payload[2] == SetOK[2]
						&& RxDoneParams.Payload[3] == SetOK[3]
						&& RxDoneParams.Payload[4]==0xDB
					)//低功耗数据
					{
						SendFlag=0;
						Pair_Flag&=~0x01<<RxDoneParams.Payload[3];
						OLED_ShowString(80,2,"  ");
						OLED_Show_Str_CHinese(96,2,"成功");
					}
				}
			if(rf_get_transmit_flag() == RADIO_FLAG_TXDONE)//传输完成
			{
				#ifdef DEBUG
							My_Printf(SetOK, 14);
						#endif
				rf_set_transmit_flag(RADIO_FLAG_IDLE);
				if(rf_enter_single_rx()!=OK)//rf进入rx单模接收数据包
				{
					#ifdef DEBUG
						My_Printf((uint8_t *)"接收失败",8);
						#endif					
					my_433.rf_flag=4;
				}
				my_433.rf_flag=6;
			}
		}
	}
		while(rf_set_default_para((uint32_t)(408+my_433.Channel)*1000000));//设置射频默认参数
}


uint32_t Iput_Num(uint8_t X,uint8_t Y,uint32_t Num,uint8_t JiWei,uint32_t Num_min,uint32_t Num_Max)//X Y 数字 位数 最小值 最大值
{
	uint8_t ID[10]={0},ID_Num,ID_Seat=0;
	uint32_t ID_Data;
	HAL_IWDG_Refresh(&hiwdg);
	for(uint8_t i=0;i<JiWei;i++)//abc -> c b a
	{
		ID[i]=Num%10;
		Num/=10;
		OLED_ShowNum(X+8*JiWei-8*i-8,Y,ID[i],1,16);//没问题
	}
	ID_Num=ID[0];
	Show_Time=45;
	Key_value=0;
	while(1)
	{
		HAL_IWDG_Refresh(&hiwdg);
		if(ID_Num>9){ID_Num=9;}
		if(Key_value==0x02)
		{
			Key_value=0;
			ID[ID_Seat]=ID_Num;OLED_ShowNum(X+8*JiWei-8*ID_Seat-8,Y,ID[ID_Seat],1,16);
			ID_Seat++;
			if(ID_Seat>=JiWei){ID_Seat=0;}
			ID_Num=ID[ID_Seat];
			Show_Time=45;
		}
		if(Key_value==0x01)
		{
			Key_value=0;
			ID[ID_Seat]=ID_Num;
			ID_Data=ID[0] + ID[1]*10 + ID[2]*100 + ID[3]*1000 + ID[4]*10000 + ID[5]*100000 + ID[6]*1000000 + ID[7]*10000000 + ID[8]*100000000 + ID[9]*1000000000;
			ID_Seat=JiWei;
			break;
		}
		if(Key_value & 0x04)
		{
			Key_value=0;
			ID_Num++;
			if(ID_Num>9)ID_Num=0;
		}
		if(Show_Time==0){OLED_ShowNum(X+8*JiWei-8*ID_Seat-8,Y,ID_Num,1,16);}
		else if(Show_Time==50){OLED_ShowString(X+8*JiWei-8*ID_Seat-8,Y,"_");}
		else if(Show_Time>=80){Show_Time=0;}
	}
	if(ID_Data<Num_min){ID_Data=Num_min;}
	if(ID_Data>Num_Max){ID_Data=Num_Max;}
	return ID_Data;
}


uint8_t Parameter_Set(My_433 *q,uint8_t GetSet)
{
	uint8_t Set_Change=0;
	uint8_t Option_Key=2,Option,Display=1,Oled_Y=2,My_Switch=8+my_433.Number;
	OLED_Clear();
	HAL_IWDG_Refresh(&hiwdg);

	uint16_t My_433_Sleep;
	uint8_t My_433_cha,My_433_W,My_433_A,My_433_B,My_433_LowOption[15];
	
	My_433_cha=q->Channel;
	My_433_Sleep=q->Time_to_stop;
	My_433_W=q->ADC_D_value;
	My_433_A=q->Angel_D_value;
	My_433_B=q->Bat_D_value;
	
	for(uint8_t i=0;i<15;i++)
	{
		My_433_LowOption[i]=my_433.Device[i].LowOption;
	}
	while(1)
	{
		HAL_IWDG_Refresh(&hiwdg);
		if(Key_value == 0x01 && GetSet==1)
			{
			Key_value = 0;
			Display=1;
			HAL_IWDG_Refresh(&hiwdg);
			HAL_Delay(200);
				switch(Option_Key)
				{
					case 0://恢复默认设置
						My_433_Sleep=120;
						My_433_W=20;
						My_433_A=50;
						My_433_B=50;
						break;
					case 1://
						My_EEPROM_WriteLen(SET_OK,0xAC,1);//配置过
						My_EEPROM_WriteLen(MY_CHANNER,My_433_cha,1);//配置过
						My_EEPROM_WriteLen(TIME_STOP,My_433_Sleep,2);//2位 休眠时间
						My_EEPROM_WriteLen(Adc_SING,My_433_W,1);//重量基数
						My_EEPROM_WriteLen(Angle_SING,My_433_A,1);//
						My_EEPROM_WriteLen(Bat_SING,My_433_B,1);//电压基数
					
						for(uint8_t i=0;i<15;i++)
						{
							My_EEPROM_WriteLen(OPTION+i,My_433_LowOption[i],1);
						}
						if(Read_EppromData()){
						OLED_Show_Str_CHinese(70,Oled_Y,"成功");
						}
						HAL_Delay(500);
						HAL_IWDG_Refresh(&hiwdg);
						Set_Change=0xFF;
						break;
					case 2://信道设置
						My_433_cha   = Iput_Num(104,2,q->Channel,3,0,142);
						break;
					case 3://休眠时间
						My_433_Sleep = Iput_Num(88,2,q->Time_to_stop,5,60,65535);//X Y 数字 位数 最小值 最大值
						break;
					case 4://重量基数
						My_433_W     = Iput_Num(104,2,q->ADC_D_value,3,0,255);//X Y 数字 位数 最小值 最大值
						break;
					case 5://角度基数
						My_433_A     = Iput_Num(104,2,q->Angel_D_value,3,0,255);//X Y 数字 位数 最小值 最大值
						break;
					case 6://电压基数
						My_433_B     = Iput_Num(104,2,q->Bat_D_value,3,0,255);//X Y 数字 位数 最小值 最大值
						break;
					case 7://一次性配15个
						My_433_LowOption[0]=Iput_Num(120,2,my_433.Device[0].LowOption,1,0,5);
						memset(My_433_LowOption,My_433_LowOption[0],15);
						break;
					default://0水平安装 1垂直安装 010差分 001电流 
						My_433_LowOption[Option_Key-8]=Iput_Num(120,2,my_433.Device[Option_Key-8].LowOption,1,0,5);
						break;
				}
				HAL_IWDG_Refresh(&hiwdg);
			}//闭合复位
		if(Key_value == 0x02)
			{
				Key_value = 0;
				Option_Key++;//按选择键界面显示，+1
				HAL_IWDG_Refresh(&hiwdg);
				HAL_Delay(200);
				Display=1;
			}
		if(Key_value & 0x04)
			{
				Key_value = 0;
				HAL_IWDG_Refresh(&hiwdg);
				HAL_Delay(200);
				OLED_Clear();
				break;
			}
		if(Display)
		{
			Display=0;
			if(Option_Key>=My_Switch){Option_Key = 0;}
			Option=Option_Key;
			OLED_Show_Str_CHinese(0,0,"参数设置");OLED_ShowString(120,0,"M");OLED_ShowNum(80,0,q->Channel+408,3,16);//433频率
			OLED_ShowString(0,2,"-->");
			for(uint8_t ii=0;ii<3;ii++)
			{
				switch(Option)
				{
					case 0:
						OLED_Show_Str_CHinese(24,Oled_Y,"恢复默认参数");OLED_ShowString(120,Oled_Y," ");
					break;
					case 1:
						OLED_Show_Str_CHinese(24,Oled_Y,"保存");OLED_ShowString(56,Oled_Y,"         ");
					break;
					case 2:
						OLED_Show_Str_CHinese(24,Oled_Y,"信道");OLED_ShowNum(72,Oled_Y,q->Channel,3,16);OLED_ShowNum(104,Oled_Y,My_433_cha,3,16);//433信道
					break;
					case 3:
						OLED_Show_Str_CHinese(24,Oled_Y,"休眠");OLED_ShowString(56,Oled_Y,"         ");OLED_ShowNum(88,Oled_Y,My_433_Sleep,5,16);//433休眠时间
					break;
					case 4:
						OLED_Show_Str_CHinese(24,Oled_Y,"重基");OLED_ShowNum(72,Oled_Y,q->ADC_D_value,3,16);OLED_ShowNum(104,Oled_Y,My_433_W,3,16);//433重量基数
					break;
					case 5:
						OLED_Show_Str_CHinese(24,Oled_Y,"角基");OLED_ShowNum(72,Oled_Y,q->Angel_D_value,3,16);OLED_ShowNum(104,Oled_Y,My_433_A,3,16);//433电压基数
					break;
					case 6:
						OLED_Show_Str_CHinese(24,Oled_Y,"电基");OLED_ShowNum(72,Oled_Y,q->Bat_D_value,3,16);OLED_ShowNum(104,Oled_Y,My_433_B,3,16);//433电压基数
					break;
					case 7:
						OLED_Show_Str_CHinese(24,Oled_Y,"总配");
						OLED_ShowString(56,Oled_Y,"         ");
						OLED_ShowNum(88,Oled_Y,my_433.Device[0].LowOption,1,16);
						OLED_ShowNum(120,Oled_Y,My_433_LowOption[0],1,16);
						break;	
					default:
						OLED_Show_Str_CHinese(24,Oled_Y,"单配");OLED_ShowNum(56,Oled_Y,Option-7,2,16);
						OLED_ShowString(72,Oled_Y,"       ");
						OLED_ShowNum(88,Oled_Y,my_433.Device[Option-8].LowOption,1,16);
						OLED_ShowNum(120,Oled_Y,My_433_LowOption[Option-8],1,16);
					break;
				}
				Oled_Y+=2;Option++;
				if(Option>=My_Switch){Option=0;}
			}
			Oled_Y=2;
		}
	}
	return Set_Change;
}


//E33_433进入设置模式
void menu(void)
{
uint8_t Menu_Option=0,Option,Display=1,Oled_Y=2,My_Switch=6;
	Key_value=0;
	OLED_Clear();
	while(1)
		{
			HAL_IWDG_Refresh(&hiwdg);
			if(Key_value == 0x01)//确认键
				{
					Key_value = 0;
					switch(Menu_Option)
						{
							case 0: break;
							case 1:
								my_433.ModbusADDR=Iput_Num(103,2,my_433.ModbusADDR,3,1,254);
								My_EEPROM_WriteLen(MODBUS_ADDR,my_433.ModbusADDR,1);
								my_433.ModbusADDR=My_EEPROM_ReadLen(MODBUS_ADDR,1);
							break;
							case 2: 
								my_433.Number=Iput_Num(111,2,my_433.Number,2,1,15);
								My_EEPROM_WriteLen(DEVICE_NUM,my_433.Number,1);
								my_433.Number	=	My_EEPROM_ReadLen(DEVICE_NUM ,1);
							break;
							case 3: 
								LowPower_Pair(0);//配对
							break;
							case 4:
								Parameter_Set(&my_433,0);
								break;
							case 5:
								if(Parameter_Set(&my_433,1))LowPower_Pair(0xffff>>(16-my_433.Number)<<1);
								break;
						}
					Display=1;
					}//闭合复位
			else if(Key_value == 0x02)//选择
				{
					Key_value = 0;
					Menu_Option++;//按选择键界面显示，+1
					Display=1;
				}
			else if(Key_value == 0x04)//返回
				{
					OLED_Clear();
					Key_value = 0;
					break;
				}
				
			if(Display)
			{
				Display=0;	
				OLED_Show_Str_CHinese(0,0,"设置"); OLED_ShowString(48,0,(uint8_t *)DATE_YYYYMMDD);
				OLED_ShowString(0,2,"-->");
				if(Menu_Option>=My_Switch){Menu_Option = 0;}
				Option=Menu_Option;
				for(uint8_t ii=0;ii<3;ii++)
				{
					switch(Option)
					{
						case 0:
						OLED_Show_Str_CHinese(24,Oled_Y,"版本");OLED_ShowString(56,Oled_Y,"V1.0");
						break;
						case 1:
						OLED_Show_Str_CHinese(24,Oled_Y,"本机地址");OLED_ShowNum(103,Oled_Y,my_433.ModbusADDR ,3,16);
						break;
						case 2:
						OLED_Show_Str_CHinese(24,Oled_Y,"配对个数");OLED_ShowString(103,Oled_Y," ");OLED_ShowNum(111,Oled_Y,my_433.Number,2,16);
						break;
						case 3:
						OLED_Show_Str_CHinese(24,Oled_Y,"配对模式");OLED_ShowString(103,Oled_Y,"   ");
						break;
						case 4:
						OLED_Show_Str_CHinese(24,Oled_Y,"参数查看");
						break;
						case 5:
						OLED_Show_Str_CHinese(24,Oled_Y,"参数设置");
						break;
						default:
					break;
					}
					Oled_Y+=2;
					Option++;
					if(Option>=My_Switch){Option=0;}
				}
				Oled_Y=2;
			}
		}
}


//0-低功耗标识+从机地址 1-2重量 3-4 X 5-6 Y 7-8 Z 9-10电池 11-限位开关 9,10-校验
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)  
{  
	if(GPIO_Pin == IRQ_Pin)
	{   		
		rf_irq_process();//RF IRQ服务器例程，应在IRQ引脚的ISR处调用
	}else if(GPIO_Pin == KEY1_Pin)
	{
		if(KEY1)//松开
		{
			if(Key_Flag>=5)Key_value=Key_Cache;
			Key_Flag=0;
		}else 
		{
			Key_Flag=1;
			Key_Cache=0x01;
		}
	}
	else if(GPIO_Pin == KEY2_Pin)
	{
		if(KEY2)//松开
		{
			if(Key_Flag>=5)Key_value=Key_Cache;
			Key_Flag=0;
		}else 
		{
			Key_Flag=1;
			Key_Cache=0x02;
		}
	}
	else if(GPIO_Pin == KEY3_Pin)
	{
		if(KEY3)//松开
		{
			if(Key_Flag>=5)Key_value=Key_Cache;
			Key_Flag=0;
		}else 
		{
			Key_Flag=1;
			Key_Cache=0x04;
		}
	}
} 



void My_Init(void)
{
	OLED_Init();
	
	my_433.ModbusADDR = My_EEPROM_ReadLen(MODBUS_ADDR,1);
	my_433.Number			=	My_EEPROM_ReadLen(DEVICE_NUM ,1);//设备个数
	if(my_433.Number>16 ||	my_433.Number==0)my_433.Number=1;
	
	Read_EppromData();
	
	LED_ON;
	my_433.rf_flag=0;
	while(rf_init());//初始化 将射频模式从深度睡眠模式切换到待机3（STB3）模式
	while(rf_set_default_para((uint32_t)(408+my_433.Channel)*1000000));//设置射频默认参数
	my_433.rf_flag=4;
	
	LED_OFF;
	my_433.Ask_Rxc[0]=0xDD;
}


uint8_t Oled_Num=0;

int16_t UintAngleToInt(uint16_t Angle)
{
	int16_t intAngle = Angle>18000 ? intAngle=Angle-36000 : Angle ;
	return intAngle;
}

uint8_t panTX_err=0,pan_err=0;
void My_Program(void)
{
	modbus_ack_handle(&my_modbus);
	
	if(rf_get_recv_flag() == RADIO_FLAG_RXDONE)//获取接收标志==无线电标志RX完成
	{
		my_433.rf_flag=4;
		rf_set_recv_flag(RADIO_FLAG_IDLE);//设置接收标志 无线电标志空闲
		LED_Toggle;//LED
		uint8_t CRC_2[2];
		if((RxDoneParams.Payload[0]>>4) == 0x0D)//低功耗数据
		{
			if(RTU_Get_CRC16(RxDoneParams.Payload,RxDoneParams.Size))//校验数据
			{
				//XZ 0-360 Y ±90 0-90 270-360
				my_433.Ask_Rxc_OK=RxDoneParams.Payload[0]&0x0F;//接收到几的数据
				uint8_t Dev_Num=my_433.Ask_Rxc_OK-1;
				my_433.Device[Dev_Num].myrssi			=RxDoneParams.Rssi;//我接收信号强度
				my_433.Device[Dev_Num].rssi				=RxDoneParams.Payload[1];//低功耗接收信号强度
				my_433.Device[Dev_Num].Weight			=Data_u16(RxDoneParams.Payload+2);//旁压
				my_433.Device[Dev_Num].AngleX			=Data_u16(RxDoneParams.Payload+4);//X
				my_433.Device[Dev_Num].AngleX_int16=UintAngleToInt(my_433.Device[Dev_Num].AngleX);
				my_433.Device[Dev_Num].AngleY			=Data_u16(RxDoneParams.Payload+6);//Y
				my_433.Device[Dev_Num].AngleY_int16=UintAngleToInt(my_433.Device[Dev_Num].AngleY);
				my_433.Device[Dev_Num].AngleZ			=Data_u16(RxDoneParams.Payload+8);//Z
				my_433.Device[Dev_Num].AngleZ_int16=UintAngleToInt(my_433.Device[Dev_Num].AngleZ);
				my_433.Device[Dev_Num].Chrg				=(RxDoneParams.Payload[10]>>7)&0x01;//充电状态 1 未充电
				my_433.Device[Dev_Num].DI					=(RxDoneParams.Payload[10]>>6)&0x01;//开关量 1 断开
				my_433.Device[Dev_Num].LowOption	=(RxDoneParams.Payload[10]>>3)&0x07;//0水平安装 1垂直安装 010差分 100电流 
				my_433.Device[Dev_Num].Battery		=(Data_u16(RxDoneParams.Payload+10)&0x03ff)+3200;//电压 10
				my_433.Device[Dev_Num].OnlineTime	=Data_u24(RxDoneParams.Payload+12);//在线时间
				my_433.err&=~(1<<my_433.Ask);//通信正常
				my_433.Device[Dev_Num].Offline_time=0;//离线时间清零
				
				my_433.Device[Dev_Num].GetNum++;
				
				#ifdef DEBUG
							static char data_text[255];
							uint8_t data_text_len;
							data_text_len=sprintf(data_text,"低1=N:%8d:X%5d:Y%5d:Z%5d:W%5d:Time%8d:C%4d  低2=N:%8d:X%5d:Y%5d:Z%5d:W%5d:Time%8d:C%4d"
								,my_433.Device[0].GetNum,my_433.Device[0].AngleX,my_433.Device[0].AngleY,my_433.Device[0].AngleZ,my_433.Device[0].Weight,my_433.Device[0].OnlineTime,my_433.Device[0].OnlineTime-my_433.Device[0].OnlineTimeOld
								,my_433.Device[1].GetNum,my_433.Device[1].AngleX,my_433.Device[1].AngleY,my_433.Device[1].AngleZ,my_433.Device[1].Weight,my_433.Device[1].OnlineTime,my_433.Device[1].OnlineTime-my_433.Device[1].OnlineTimeOld
							);
							My_Printf((uint8_t*)data_text,data_text_len);
						#endif
			
				
				
				my_433.Device[Dev_Num].OnlineTimeOld=my_433.Device[Dev_Num].OnlineTime;
			}
		}
	}
	 //rx超时或rx-err标志
	if((rf_get_recv_flag() == RADIO_FLAG_RXTIMEOUT) || (rf_get_recv_flag() == RADIO_FLAG_RXERR))
	{
		rf_set_recv_flag(RADIO_FLAG_IDLE);
		my_433.rf_flag=4;
		#ifdef DEBUG
			My_Printf((uint8_t *)"NORX",4);			
						#endif
		
	}
	
	//发送数据
	if(my_433.Send_flag>=DATA_TX_DELAY)//数据发送标志n*20ms >260ms 几乎不掉数据 14
	{
		my_433.Ask++;
		if(my_433.Ask>my_433.Number)my_433.Ask=1;//1,2,3,4
		
		my_433.Ask_Rxc_OK+=my_433.Ask<<4;
		my_433.Ask_Rxc[1]=my_433.Ask_Rxc_OK; 
		if(rf_single_tx_data(my_433.Ask_Rxc, 2, &tx_time) != OK)//rf进入单tx模式并发送数据包
    {
			#ifdef DEBUG
					My_Printf("TXNO",4);//tx失败	
						#endif
			my_433.rf_flag=4;
    }
		#ifdef DEBUG
			else My_Printf("TXOK",4);			
						#endif
		
		panTX_err++;
		my_433.rf_flag=5;
		my_433.Send_flag=0;
		my_433.Ask_Rxc_OK=0x00;//清楚接收标志
		
		if(my_433.Device[my_433.Ask-1].Offline_time>my_433.Time_to_stop)//超过130s没有接收到D1数据
		{
			my_433.err|=1<<my_433.Ask;
		}
		if(Oled_Num)
		{
			Oled_Data(Oled_Num-1);
		}else
		{
		Oled_Device();
		}
	}
	if(rf_get_transmit_flag() == RADIO_FLAG_TXDONE)//传输完成
	{
		panTX_err=0;
		my_433.Send_flag=DATA_TX_DELAY-25;
		rf_set_transmit_flag(RADIO_FLAG_IDLE);
		if(rf_enter_single_rx()!=OK)//rf进入rx单模接收数据包
//		if(rf_enter_single_timeout_rx(230)!=OK)
		{
			#ifdef DEBUG
						My_Printf((uint8_t *)"RXNO",4);
						#endif		
			my_433.rf_flag=4;
		}//超时模式接收数据包
		#ifdef DEBUG
			else My_Printf("RXOK",4);			
						#endif
	
		my_433.rf_flag=6;
	}
	
	if(panTX_err>3)
	{
		panTX_err=0;
		pan_err++;
		HAL_Delay(2000);
		while(rf_init());//初始化 将射频模式从深度睡眠模式切换到待机3（STB3）模式
		while(rf_set_default_para((uint32_t)(408+my_433.Channel)*1000000));//设置射频默认参数
		HAL_Delay(100);
	}
	
	if(Key_value)
	{
	if(Key_value==1)
	{
		menu();//设置模式
	}
	else if(Key_value==2)
	{
		OLED_Clear();
		Oled_Num++;
		if(Oled_Num>my_433.Number)Oled_Num=0;
	}
		Key_value=0;
	}
}

char Oled_Text[11];

void Oled_Device(void)
{
	OLED_Show_Str_CHinese(0,0,"频率");
	OLED_ShowNum(60,0,408+my_433.Channel,3,16);
	OLED_ShowString(100,0,"Mhz");
	
	OLED_ShowString(0,2,"Ad");OLED_ShowNum(16,2,my_433.ModbusADDR,3,16);
	OLED_ShowString(0,4,"Num");OLED_ShowNum(24,4,my_433.Number,2,16);
	
	OLED_ShowNum(0,6,panTX_err,3,16); OLED_ShowNum(24,6,pan_err,2,16);
	
	for(uint8_t i=2,j=1;i<=6;i+=2)
	{
		OLED_ShowNum(56,i,(my_433.err>>j++)&1,1,16);
		OLED_ShowNum(72,i,(my_433.err>>j++)&1,1,16);
		OLED_ShowNum(88,i,(my_433.err>>j++)&1,1,16);
		OLED_ShowNum(104,i,(my_433.err>>j++)&1,1,16);
		OLED_ShowNum(120,i,(my_433.err>>j++)&1,1,16);
	}
}

void Oled_Data(uint8_t Dev_Num)
{
	OLED_ShowString(0,0,"D");
	OLED_ShowNum(8,0,Dev_Num+1,2,16);
	OLED_ShowString(32,0,"W");
	OLED_ShowNum(40,0,my_433.Device[Dev_Num].Weight,5,16);//重量ADC
	OLED_ShowNum(88,0,my_433.Device[Dev_Num].DI,1,16);//开关量
	
	if(my_433.Device[Dev_Num].LowOption&0x01)OLED_Show_Str_CHinese(96,0,"垂");
	else OLED_Show_Str_CHinese(96,0,"水");
		
	if(my_433.Device[Dev_Num].LowOption>>1){
	if(my_433.Device[Dev_Num].LowOption&0x04)OLED_Show_Str_CHinese(112,0,"电");
	else OLED_Show_Str_CHinese(112,0,"差");
	}
	
	OLED_ShowString(72,2,"B");
	OLED_ShowNum(80,2,my_433.Device[Dev_Num].Battery,4,16);//电池电压
  OLED_ShowNum(120,2,my_433.Device[Dev_Num].Chrg,1,16);//充电
	
	OLED_ShowString(0,2,"X");
	OLED_ShowNum(8,2,my_433.Device[Dev_Num].AngleX,5,16);//X

	OLED_ShowString(0,4,"Y");
	OLED_ShowNum(8,4,my_433.Device[Dev_Num].AngleY,5,16);

	OLED_ShowString(0,6,"Z");
	OLED_ShowNum(8,6,my_433.Device[Dev_Num].AngleZ,5,16);
	
	
	sprintf(Oled_Text,"%4d:%02d:%02d",my_433.Device[Dev_Num].OnlineTime/3600,my_433.Device[Dev_Num].OnlineTime/60%60,my_433.Device[Dev_Num].OnlineTime%60);
	OLED_ShowString(48,4,Oled_Text);
//	OLED_ShowNum(72,4,my_433.Device[Dev_Num].OnlineTime,5,16);
	
//	OLED_ShowNum(72,6,my_433.Device[Dev_Num].myrssi,3,16);
//	OLED_ShowNum(104,6,my_433.Device[Dev_Num].rssi,3,16);
//	
	sprintf(Oled_Text,"%4d%4d",(int8_t)my_433.Device[Dev_Num].myrssi,(int8_t)my_433.Device[Dev_Num].rssi);
	OLED_ShowString(64,6,Oled_Text);
	
//	OLED_ShowNum(72,6,my_433.Device[Dev_Num].myrssi,3,16);
//	OLED_ShowNum(104,6,my_433.Device[Dev_Num].rssi,3,16);
}

#include "tim.h"
/*定时器回调函数*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == htim21.Instance)//1S
  {
		for(uint8_t i=0;i<my_433.Number;i++)
		{
		my_433.Device[i].Offline_time++;
		}
  }
  if (htim->Instance == htim22.Instance)//10ms
  {
		my_433.Send_flag++;
		Show_Time++;
		if(Key_Flag)Key_Flag++;
  } 
}


