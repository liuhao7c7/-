#ifndef __MYMAINPROGRAM_H  //防止头文件的重复包含和编译  配套#endif
#define __MYMAINPROGRAM_H  //标识符

#include "main.h"
#include "my_Database.h"
#include "stdlib.h"
#include "crc.h"
#include "iwdg.h"

void CS(uint8_t num);

#define ReadDI HAL_GPIO_ReadPin(DI_GPIO_Port,DI_Pin) //读DI引脚
//#define DI2 HAL_GPIO_ReadPin(DI2_GPIO_Port,DI2_Pin) //读DI引脚

#define Chrg HAL_GPIO_ReadPin(CHRG_GPIO_Port,CHRG_Pin)

#define PAN_NRST HAL_GPIO_WritePin(NRST_433_GPIO_Port, NRST_433_Pin, GPIO_PIN_SET) //打开LED
#define PAN_RST HAL_GPIO_WritePin(NRST_433_GPIO_Port, NRST_433_Pin, GPIO_PIN_RESET)	//关闭LED

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

#define SPI_MOSI_UP HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET) //打开LED
#define SPI_MOSI_DOWN HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET)

void E433_Set(uint8_t My_Set);
void My_Init(void);
void My_Program(void);
void Clear_Offline(void);
void Set_Standby();
#endif


