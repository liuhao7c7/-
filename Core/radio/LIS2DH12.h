#ifndef __LSM6DS3TR_D_H_
#define __LSM6DS3TR_D_H_
#include "stdint.h"

uint8_t LSM_GetID();
uint8_t LSM_Reset();
void LSM_Init();
void QCM_Init();
void Get_Angle();
uint16_t AngleChange(uint16_t AngleNew,uint16_t AngleOld);
#endif


