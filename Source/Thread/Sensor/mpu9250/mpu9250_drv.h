#ifndef __MPU9250_DRV_H__
#define __MPU9250_DRV_H__

#include "common.h"

/* 函数定义 */
void mpu9250I2CInit(void);
int32_t mpu9250WriteBytes(uint8_t slvAddr, uint8_t regAddr, uint8_t numBytes, uint8_t *dataPtr);
int32_t mpu9250ReadBytes(uint8_t slvAddr,uint8_t regAddr, uint8_t numBytes, uint8_t *dataPtr);

#endif
