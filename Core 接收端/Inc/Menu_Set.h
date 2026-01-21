#ifndef __MENU_SET_H
#define __MENU_SET_H	
#include "stdint.h"

extern uint8_t WS_Switch;
/*24KC08存储地址0-1013*/
#define My_Usart_0 0 //
#define My_Usart_1 1 //串口选择
#define My_Usart_2 2 //
#define My_Usart_3 3 //
#define My_Usart_4 4 //

#define WS_SWITCH 			110	//外设选择 0-重量1,1重量2
#define PAIR_ADDR 			111	//2位 配对地址*5
#define PAIR_CHANNEL 		121	//配对信道*5
#define PAIR_SLEEP			126	//2位 休眠时间
#define PAIR_WEIGHT			136	//重量基数
#define PAIR_BATTER			141	//电压基数
#define PAIR_Flag				146 //配对标志

void menu(void);
	
#endif



