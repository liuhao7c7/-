#ifndef __MYMAINPROGRAM_H  //防止头文件的重复包含和编译  配套#endif
#define __MYMAINPROGRAM_H  //标识符

#include "main.h"
#include "my_Database.h"
#include "stdlib.h"
#include "iwdg.h"

#define DI1 HAL_GPIO_ReadPin(DI1_GPIO_Port,DI1_Pin) //读DI引脚
#define DI2 HAL_GPIO_ReadPin(DI2_GPIO_Port,DI2_Pin) //读DI引脚

//#define Chrg HAL_GPIO_ReadPin(CHRG_GPIO_Port,CHRG_Pin)

#define LED_ON HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET) //打开LED
#define LED_OFF HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET)	//关闭LED
#define LED_Toggle	HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin)	//(两种状态之间)切换

#define Weight_OFF HAL_GPIO_WritePin(K1_GPIO_Port,K1_Pin,GPIO_PIN_SET)//断电 差分及运放
#define Weight_ON  HAL_GPIO_WritePin(K1_GPIO_Port,K1_Pin,GPIO_PIN_RESET)//通电 差分及运放

#define Angle_OFF HAL_GPIO_WritePin(K0_GPIO_Port,K0_Pin,GPIO_PIN_SET)//断电 差分及运放
#define Angle_ON  HAL_GPIO_WritePin(K0_GPIO_Port,K0_Pin,GPIO_PIN_RESET)//通电 差分及运放

#define AI_12V_OFF HAL_GPIO_WritePin(K2_GPIO_Port,K2_Pin,GPIO_PIN_RESET)//断电 12V
#define AI_12V_ON	 HAL_GPIO_WritePin(K2_GPIO_Port,K2_Pin,GPIO_PIN_SET)//通电 12V

#define KEY HAL_GPIO_ReadPin(KEY_GPIO_Port,KEY_Pin) //

#define NoReset HAL_GPIO_WritePin(NRST_GPIO_Port, NRST_Pin, GPIO_PIN_SET) //打开LED
#define Reset HAL_GPIO_WritePin(NRST_GPIO_Port, NRST_Pin, GPIO_PIN_RESET)

//-----------------OLED端口定义初始化中,全部设为输出----------------  
#define OLED_SCLK_0 HAL_GPIO_WritePin(OLED_SCLK_GPIO_Port, OLED_SCLK_Pin, GPIO_PIN_RESET)
#define OLED_SCLK_1 HAL_GPIO_WritePin(OLED_SCLK_GPIO_Port, OLED_SCLK_Pin, GPIO_PIN_SET)

#define OLED_SDIN_0 HAL_GPIO_WritePin(OLED_SDIN_GPIO_Port, OLED_SDIN_Pin, GPIO_PIN_RESET)
#define OLED_SDIN_1 HAL_GPIO_WritePin(OLED_SDIN_GPIO_Port, OLED_SDIN_Pin, GPIO_PIN_SET)

#define OLED_RES_0 HAL_GPIO_WritePin(OLED_RES_GPIO_Port, OLED_RES_Pin, GPIO_PIN_RESET)
#define OLED_RES_1 HAL_GPIO_WritePin(OLED_RES_GPIO_Port, OLED_RES_Pin, GPIO_PIN_SET)

#define OLED_DC_0 HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_RESET)
#define OLED_DC_1 HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET)

//------------------按键定义------全部为输入--------
#define KEY1  HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin)//读取按键0
#define KEY2  HAL_GPIO_ReadPin(KEY2_GPIO_Port,KEY2_Pin)//读取按键0
#define KEY3  HAL_GPIO_ReadPin(KEY3_GPIO_Port,KEY3_Pin)//读取按键0


//__DATE__转YYYYMMDD
#define DATE_YYYYMMDD \
    (const char[]){ \
        __DATE__[9], __DATE__[10], \
        ((__DATE__[0]=='O' || __DATE__[0]=='N' || __DATE__[0]=='D')?'1':'0'), \
        ((__DATE__[0]=='J'&&__DATE__[1]=='a')?'1': \
         __DATE__[0]=='F'?'2': \
         (__DATE__[0]=='M'&&__DATE__[2]=='r')?'3': \
         (__DATE__[0]=='A'&&__DATE__[1]=='p')?'4': \
         __DATE__[0]=='M'?'5': \
         (__DATE__[0]=='J'&&__DATE__[2]=='n')?'6': \
         (__DATE__[0]=='J'&&__DATE__[2]=='l')?'7': \
         (__DATE__[0]=='A'&&__DATE__[1]=='u')?'8': \
         __DATE__[0]=='S'?'9': \
         __DATE__[0]=='O'?'0': \
         __DATE__[0]=='N'?'1': \
         '2'), \
        (__DATE__[4]==' '?'0':__DATE__[4]), \
        (__DATE__[4]==' '?__DATE__[5]:__DATE__[5]), \
				__TIME__[0], __TIME__[1], \
        __TIME__[3], __TIME__[4], \
			'\0'\
    }

//void E433_Set(uint8_t My_Set);
void My_Init(void);
void My_Program(void);
void Clear_Offline(void);
void Oled_Device(void);
void Oled_Data(uint8_t Dev_Num);
#endif


