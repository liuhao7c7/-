/******************************/
/*  编码器选择程序、改地址程序  */
/******************************/
#include "Menu_Set.h"
#include "usart.h"
#include "oled.h"
#include "CRC.h"
#include "iwdg.h"
#include "string.h"

uint8_t Key_Data=0;
uint8_t x_oled = 30;
uint8_t y_oled = 0;
uint8_t WS_Switch;//外设选择 0-重量1,1重量2


void My_Pair_Set(MY_DATA *p,E33_433 *q,uint8_t Pair_Flag)//0-重量1 1-重量2
{
	uint8_t Option_Key=0,Option,Oled_Y=2,My_Switch=1;
	OLED_Clear();
	OLED_Show_Str_CHinese(32,2,"正在加载");
	HAL_IWDG_Refresh(&hiwdg);
	uint16_t Send_Addr;
	uint8_t Send_Channel,Send_Flag=0;
	uint8_t My_Stay_Set[17]={0x00,0x00,0x00,0xDD,0xDB,0xEA,0x00,0x00,0x00,0x00,0x96,0x06,0x24,0x00,0x00,0x15,0x16};
	uint8_t My_Pair_Data[17]={0xFF,0xFF,0x10,0xDD,0xDB,0x00,0x00,0x00,0x00,0x00,0x96,0x06,0x24,0x00,0x00,0x15,0x16};

	Send_Addr=AT24CXX_ReadLenByte(PAIR_ADDR+q->Num*2,2);//之前的低功耗地址
	Send_Channel=AT24CXX_ReadLenByte(PAIR_CHANNEL+q->Num,1);//之前的低功耗信道
	My_Stay_Set[0]=Send_Addr>>8;
	My_Stay_Set[1]=Send_Addr&0xff;
	My_Stay_Set[2]=Send_Channel&0xff;

	My_Stay_Set[6]=Modbus_20ms_flag & 0xff;
	
	My_Pair_Data[6]	=(q->Addr-1)>>8;	//地址
	My_Pair_Data[7]	=(q->Addr-1)&0xff;
	My_Pair_Data[8]	=q->Channel;			//信道
	My_Pair_Data[9]	=q->Sleep_Time>>8;//休眠时间
	My_Pair_Data[10]=q->Sleep_Time&0xff;
	My_Pair_Data[11]=q->Weight_Send;	//重量基准
	My_Pair_Data[12]=q->Batter_Send;	//电池基准
	//My_Pair_Data[13]=;//备用
	//My_Pair_Data[14]=;//
	My433_Mode_Set(q->Num,1);//唤醒模式
	HAL_Delay(100);
	Send_Data(q->Num,(uint8_t *)My_Stay_Set,17);
	HAL_Delay(700);
	HAL_IWDG_Refresh(&hiwdg);
	HAL_Delay(700);
	HAL_IWDG_Refresh(&hiwdg);
	HAL_Delay(700);
	HAL_IWDG_Refresh(&hiwdg);
	My433_Mode_Set(q->Num,0);//普通模式
	for(uint8_t i=0;i<2;i++)
	{
		HAL_Delay(700);
		HAL_IWDG_Refresh(&hiwdg);
		Send_Data(q->Num,(uint8_t *)My_Stay_Set,17);
	}
	OLED_Clear();
	while(1)
	{
		HAL_IWDG_Refresh(&hiwdg);
		Key_Data = KEY_Scan();
		if(WS_Switch){My_Switch=2;}
		else{My_Switch=1;Pair_Flag &=0x0D;}
		if(p->RXD_flag)//收到数据
		{
			p->RXD_flag=0;
			if(memcmp(My_Pair_Data+3,p->buf,14)==0 ) //接收到反馈相同数据
			{
				Send_Flag=0;
				AT24CXX_WriteLenByte(PAIR_ADDR+q->Num*2,q->Addr-1,2);
				AT24CXX_WriteLenByte(PAIR_CHANNEL+q->Num,q->Channel,1);
				if(p->buf[2]==0x01){Pair_Flag &=0xD0;}
					else if(p->buf[2]==0x02){Pair_Flag &=0x0D;}
				OLED_ShowString(64,2,"        ");
				OLED_Show_Str_CHinese(96,2,"成功");
			}
		}
		if(p->Send_flag>100)
		{
			if(Send_Flag){Send_Data(q->Num,(uint8_t *)My_Pair_Data,17);}
			else{Send_Data(q->Num,(uint8_t *)My_Stay_Set,17);}
			p->Send_flag=0;
		}
		if(Key_Data == 0x01)
			{
				Send_Flag=1;
				OLED_ShowString(64,2,"=>");
				OLED_Show_Str_CHinese(80,2,"配对中");
				Key_Data = 0;
				HAL_IWDG_Refresh(&hiwdg);
				HAL_Delay(300);
			}
		if(Key_Data == 0x02)
			{
				Send_Flag=0;
				Option_Key++;//按选择键界面显示，+1
				OLED_ShowString(64,2,"        ");
				Key_Data = 0;
				HAL_IWDG_Refresh(&hiwdg);
				HAL_Delay(300);
			}
		if(Key_Data & 0x04 && Pair_Flag==0)
			{
				Key_Data = 0;
				HAL_IWDG_Refresh(&hiwdg);
				HAL_Delay(300);
				OLED_Clear();
				break;
			}
		if(Option_Key>=My_Switch){Option_Key = 0;}
		My_Pair_Data[5]=Option_Key+1;
		RTU_Send_CRC16( My_Pair_Data+3, 14 ) ;//计算CRC16校验
		Option=Option_Key;
		OLED_Show_Str_CHinese(0,0,"重量配对");OLED_ShowString(64,0,"433-");OLED_ShowNum(96,0,q->Num,1,16);
		OLED_ShowString(0,2,"-->");
		for(uint8_t ii=0;ii<My_Switch;ii++)
		{
			switch(Option)
			{
				case 0:OLED_Show_Str_CHinese(24,Oled_Y,"重量");OLED_ShowString(56,Oled_Y,"1 ");break;
				case 1:OLED_Show_Str_CHinese(24,Oled_Y,"重量");OLED_ShowString(56,Oled_Y,"2 ");break;
				default:break;
			}
			Oled_Y+=2;Option++;
			if(Option>=My_Switch){Option=0;}
		}
		Oled_Y=2;
	}
}

uint32_t Iput_Num(uint8_t X,uint8_t Y,uint32_t Num,uint8_t JiWei,uint32_t Num_min,uint32_t Num_Max)//X Y 数字 位数 最小值 最大值
{
	uint8_t ID[10]={0},ID_Num,ID_Seat=0;
	uint32_t ID_Data=0xFFFFFFFF;
	HAL_Delay(300);
	HAL_IWDG_Refresh(&hiwdg);
	ID_Data=Num;
	for(uint8_t i=0;i<JiWei;i++)//abc -> c b a
	{
		ID[i]=Num%10;
		Num/=10;
		OLED_ShowNum(X+8*JiWei-8*i-8,Y,ID[i],1,16);//没问题
	}
	while(1)
	{
		HAL_IWDG_Refresh(&hiwdg);
		ID_Num = Read_Switch();//读取拨码开关地址 设置地址
		if(ID_Num>9){ID_Num=9;}
		Key_Data = KEY_Scan();
		if(Key_Data==0x02)
		{
			HAL_IWDG_Refresh(&hiwdg);
			HAL_Delay(300);
			if(ID_Seat!=JiWei){ID[ID_Seat]=ID_Num;OLED_ShowNum(X+8*JiWei-8*ID_Seat-8,Y,ID[ID_Seat],1,16);}
			ID_Seat++;
			if(ID_Seat>JiWei){ID_Seat=0;}
			Key_Data=0;
		}
		if(Key_Data==0x01)
		{
			Key_Data=0;
			HAL_Delay(200);
			ID[ID_Seat]=ID_Num;
			ID_Data=ID[0] + ID[1]*10 + ID[2]*100 + ID[3]*1000 + ID[4]*10000 + ID[5]*100000 + ID[6]*1000000 + ID[7]*10000000 + ID[8]*100000000 + ID[9]*1000000000;
			ID_Seat=JiWei;
			break;
		}
		if(Key_Data & 0x04)
		{
			Key_Data=0;
			HAL_IWDG_Refresh(&hiwdg);
			HAL_Delay(200);
			break;
		}
		if(ID_Seat!=JiWei)
		{
			if(Modbus_20ms_flag==0){OLED_ShowNum(X+8*JiWei-8*ID_Seat-8,Y,ID_Num,1,16);}
			else if(Modbus_20ms_flag==20){OLED_ShowString(X+8*JiWei-8*ID_Seat-8,Y,"_");}
			else if(Modbus_20ms_flag>=40){Modbus_20ms_flag=0;}
		}
	}
	if(ID_Data<Num_min){ID_Data=Num_min;}
	if(ID_Data>Num_Max){ID_Data=Num_Max;}
	return ID_Data;
}

void Channel_Channel_Mhz(uint8_t X,uint8_t Y,uint8_t Channel)
{
uint8_t	Odd_Even;
uint16_t Channel_Mhz;
	switch(My_433Drive)
	{
		case E3n_E330:
			Odd_Even=Channel % 2;
			Channel_Mhz=Channel*0.5+425;
			if(Odd_Even==0)
			{OLED_ShowString(X,Y,"  ");OLED_ShowNum(X+16,Y,Channel_Mhz,3,16);}//433频率
			else{OLED_ShowNum(X,Y,Channel_Mhz,3,16);OLED_ShowString(X+24,Y,".5");}
		break;
		case E220_xxx:
			Channel_Mhz=Channel+410.125;
			OLED_ShowNum(X,Y,Channel_Mhz,3,16);OLED_ShowString(X+24,Y,".1");
		break;
}
}


uint8_t Write_E33_433T13D[6]={
0xC0,//HEAD  C0:所设置的参数会掉电保存	C2:所设置的参数不会掉电保存	
0x00,//ADDRH	00-FFH  default：00H
0xFF,//ADDRL	00-FFH	default：00H
0x18,//SPED	
0x10,//CHAN  4,3,2,1,0(0-31)	(425M + CHAN*0.5M)	00H-FFH,对应425~440.5MHz
0xF8
};

uint8_t Write_E220_400T22D[11]={
0xC0,//HEAD  C0:所设置的参数会掉电保存	C2:所设置的参数不会掉电保存	
0x00,
0x08,
0x00,//ADDRH	00-FFH  default：00H
0x00,//ADDRL	00-FFH	default：00H
0x60,//REG0
0xC0,//REG1
0x17,//REG2   CHAN 410.125+CHAN*1M   0-83
0xC3,//REG3  RSSI 定点 备 LBT 备 W O R周期   C3打开RSSI信号 43关闭
0x00,
0x00
};

uint8_t E33_433_Set(MY_DATA *p,E33_433 *q,uint8_t GetSet)
{
	uint8_t Set_Change=0;
	uint8_t Option_Key=1,Option,Display=1,Oled_Y=2,My_Switch=7;
	OLED_Clear();
	HAL_IWDG_Refresh(&hiwdg);
	/*433配置*/
	My433_Mode_Set(q->Num,3);//调试模式
	HAL_Delay(100);
	Send_Data(q->Num,Read_My_433,3);
	HAL_Delay(100);
	HAL_IWDG_Refresh(&hiwdg);
	My433_Parameter(p,q);//获取433参数
	
	uint16_t My_433_add,My_433_cha,My_433_Sleep,My_433_W,My_433_B;
	My_433_add=q->Addr;
	My_433_cha=q->Channel;
	My_433_Sleep=q->Sleep_Time;
	My_433_W=q->Weight_Send;
	My_433_B=q->Batter_Send;
	while(1)
	{
		HAL_IWDG_Refresh(&hiwdg);
		Key_Data = KEY_Scan();

		if(Key_Data == 0x01 && GetSet==1)
			{
			Key_Data = 0;
			Display=1;
			HAL_IWDG_Refresh(&hiwdg);
			HAL_Delay(300);
				switch(Option_Key)
				{
					case 0://
						My433_Mode_Set(q->Num,3);//调试模式
						HAL_Delay(100);
						switch(My_433Drive)
						{
							case E3n_E330:
								Write_E33_433T13D[2]=My_433_add;
								Write_E33_433T13D[4]=My_433_cha;
								Send_Data(q->Num,(uint8_t *)Write_E33_433T13D, 6);//设置参数
							break;
							case	E220_xxx:
								Write_E220_400T22D[4]=My_433_add;
								Write_E220_400T22D[7]=My_433_cha;
								Send_Data(q->Num,(uint8_t *)Write_E220_400T22D, 11);//设置参数
								break;
						}
						AT24CXX_WriteLenByte(PAIR_SLEEP+q->Num*2,My_433_Sleep,2);//2位 休眠时间
						AT24CXX_WriteLenByte(PAIR_WEIGHT+q->Num,My_433_W,1);//重量基数
						AT24CXX_WriteLenByte(PAIR_BATTER+q->Num,My_433_B,1);//电压基数
						OLED_Show_Str_CHinese(70,Oled_Y,"成功");
						HAL_Delay(500);
						HAL_IWDG_Refresh(&hiwdg);
						My433_Parameter(p,q);//获取433参数
						Set_Change=0xFF;
						break;
					case 1://地址设置
						My_433_add=Iput_Num(103,2,q->Addr,3,1,255);//X Y 数字 位数 最小值 最大值
						break;
					case 2://信道设置
						switch(My_433Drive)
						{
							case E3n_E330:My_433_cha=Iput_Num(111,2,q->Channel,2,0,31);break;//X Y 数字 位数 最小值 最大值
							case E220_xxx:My_433_cha=Iput_Num(111,2,q->Channel,2,0,83);break;//X Y 数字 位数 最小值 最大值
						}
						break;
					case 3://休眠时间
						My_433_Sleep=Iput_Num(87,2,q->Sleep_Time,5,60,65535);//X Y 数字 位数 最小值 最大值
						break;
					case 4://重量基数
						My_433_W=Iput_Num(103,2,q->Weight_Send,3,0,999);//X Y 数字 位数 最小值 最大值
						break;
					case 5://电压基数
						My_433_B=Iput_Num(103,2,q->Batter_Send,3,0,999);//X Y 数字 位数 最小值 最大值
						break;
					case 6://恢复默认设置
						My_433_Sleep=150;
						My_433_W=6;
						My_433_B=36;
						break;
					default:
				break;
				}
				HAL_IWDG_Refresh(&hiwdg);
			}//闭合复位
		if(Key_Data == 0x02)
			{
				Key_Data = 0;
				Option_Key++;//按选择键界面显示，+1
				HAL_IWDG_Refresh(&hiwdg);
				HAL_Delay(300);
				Display=1;
			}
		if(Key_Data & 0x04)
			{
				HAL_IWDG_Refresh(&hiwdg);
				HAL_Delay(300);
				OLED_Clear();
				Key_Data = 0;
				break;
			}
		if(Display)
		{
			Display=0;
			if(Option_Key>=My_Switch){Option_Key = 0;}
			Option=Option_Key;
			OLED_ShowNum(0,0,q->Num,1,16);
			OLED_ShowString(12,0,"433");OLED_Show_Str_CHinese(36,0,"设置");OLED_ShowString(120,0,"M");Channel_Channel_Mhz(80,0,q->Channel);//433频率
			OLED_ShowString(0,2,"-->");
			for(uint8_t ii=0;ii<3;ii++)
			{
				switch(Option)
				{
					case 0:
						OLED_Show_Str_CHinese(24,Oled_Y,"保存");OLED_ShowString(56,Oled_Y,"         ");
					break;
					case 1:
						OLED_Show_Str_CHinese(24,Oled_Y,"地址");OLED_ShowString(56,Oled_Y,"         ");OLED_ShowNum(71,Oled_Y,q->Addr,3,16);OLED_ShowNum(103,Oled_Y,My_433_add,3,16);//433地址
					break;
					case 2:
						OLED_Show_Str_CHinese(24,Oled_Y,"信道");OLED_ShowString(56,Oled_Y,"         ");OLED_ShowNum(79,Oled_Y,q->Channel,2,16);OLED_ShowNum(111,Oled_Y,My_433_cha,2,16);//433信道
					break;
					case 3:
						OLED_Show_Str_CHinese(24,Oled_Y,"休眠");OLED_ShowString(56,Oled_Y,"         ");OLED_ShowNum(87,Oled_Y,My_433_Sleep,5,16);//433休眠时间
					break;
					case 4:
						OLED_Show_Str_CHinese(24,Oled_Y,"重基");OLED_ShowString(56,Oled_Y,"         ");OLED_ShowNum(79,Oled_Y,q->Weight_Send,2,16);OLED_ShowNum(111,Oled_Y,My_433_W,2,16);//433重量基数
					break;
					case 5:
						OLED_Show_Str_CHinese(24,Oled_Y,"电基");OLED_ShowString(56,Oled_Y,"         ");OLED_ShowNum(79,Oled_Y,q->Batter_Send,2,16);OLED_ShowNum(111,Oled_Y,My_433_B,2,16);//433电压基数
					break;
					case 6:
						OLED_Show_Str_CHinese(24,Oled_Y,"恢复默认参数");OLED_ShowString(120,Oled_Y," ");
						break;
					default:
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

void Set_433_485(MY_DATA *p,E33_433 *q)
{
	uint8_t Menu_Option=0,Option,Display=1,Oled_Y=2,My_Switch=3,Get_Set;
	HAL_Delay(200);
	HAL_IWDG_Refresh(&hiwdg);
	OLED_Clear();
	while(1)
	{
		HAL_IWDG_Refresh(&hiwdg);
		Key_Data = KEY_Scan();
		if(Read_Switch()==0x0f){Get_Set=1;}
		else{Get_Set=0;}
		if(Key_Data == 0x01)//确认键
		{
			Key_Data = 0;
			Display=1;
			HAL_Delay(300);
			switch(Menu_Option)
			{
				case 0: My_Pair_Set(p,q,0);Display=1; break;//重量配对
				case 1: if(E33_433_Set(p,q,Get_Set)){My_Pair_Set(p,q,0xDD);}Display=1; break;//433设置，配对
				case 2:	if(Get_Set)
								{ if(WS_Switch){AT24CXX_WriteLenByte(WS_SWITCH,0,1);HAL_Delay(20);}
									else 				 {AT24CXX_WriteLenByte(WS_SWITCH,1,1);HAL_Delay(20);}
								}
								WS_Switch = AT24CXX_ReadLenByte(WS_SWITCH,1);
				break;
			}
		}//闭合复位
		else if(Key_Data == 0x02)//选择
		{
			Key_Data = 0;
			Display=1;
			Menu_Option++;//按选择键界面显示，+1
			HAL_Delay(300);
		}
		else if(Key_Data == 0x04)//返回
		{
			OLED_Clear();
			Key_Data = 0;
			HAL_Delay(300);
			break;
		}
		if(Menu_Option>=My_Switch){Menu_Option = 0;}
		Option=Menu_Option;
		if(Display)
		{
			Display=0;
			OLED_Show_Str_CHinese(0,0,"参数设置");
			OLED_ShowString(0,2,"-->");
			for(uint8_t ii=0;ii<3;ii++)
			{
				switch(Option)
				{
					case 0:
					OLED_Show_Str_CHinese(24,Oled_Y,"重量配对");OLED_ShowString(103,Oled_Y,"   ");
					break;
					case 1:
					OLED_ShowString(24,Oled_Y,"433 ");OLED_Show_Str_CHinese(56,Oled_Y,"设置");
					break;
					case 2:
					OLED_Show_Str_CHinese(24,Oled_Y,"重量个数");OLED_ShowNum(103,Oled_Y,WS_Switch+1,1,16);OLED_Show_Str_CHinese(111,Oled_Y,"个");
					break;
					default:
				break;
				}
				Oled_Y+=2;
				Option++;
				if(Option>=My_Switch){Option=0;}
				if(ii>=My_Switch){break;}
			}
		 Oled_Y=2;
	}
}
}
	

