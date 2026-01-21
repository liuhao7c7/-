#ifndef __LSM6DS3TR_D_H_
#define __LSM6DS3TR_D_H_
#include "stdint.h"

uint8_t LSM_GetID();
uint8_t LSM_Reset();
void LSM_Init();
void LSM_Get_Angle();
float atan2_approx(float y, float x);
void LSM_Set_u8(uint8_t Addr,uint8_t rate);
#endif


