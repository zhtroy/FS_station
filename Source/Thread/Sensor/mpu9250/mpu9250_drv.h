#ifndef __MPU9250_DRV_H__
#define __MPU9250_DRV_H__

#include <stdint.h>

/***** MPU9250寄存器地址 ***********/
#define MPU9250_SMPLRT_DIV      0x19    //陀螺仪采样率，典型值：0x07(125Hz)
#define MPU9250_CONFIG          0x1A    //低通滤波频率，典型值：
#define MPU9250_GYRO_CONFIG     0x1B    //陀螺仪自检及测量范围，典型值：0x18(不自检，2000deg/s)
#define MPU9250_ACCEL_CONFIG    0x1C    //加速计自检、测量范围及高通滤波频率，典型值：0x01(不自检，2G，5Hz)
#define MPU9250_ACCEL_CONFIG_2  0x1D

#define MPU9250_INT_PIN_CFG     0x37
#define MPU9250_INT_STATUS      0x3A
#define MPU9250_ACCEL_XOUT_H    0x3B
#define MPU9250_ACCEL_XOUT_L    0x3C
#define MPU9250_ACCEL_YOUT_H    0x3D
#define MPU9250_ACCEL_YOUT_L    0x3E
#define MPU9250_ACCEL_ZOUT_H    0x3F
#define MPU9250_ACCEL_ZOUT_L    0x40

#define MPU9250_TEMP_OUT_H      0x41
#define MPU9250_TEMP_OUT_L      0x42

#define MPU9250_GYRO_XOUT_H     0x43
#define MPU9250_GYRO_XOUT_L     0x44    
#define MPU9250_GYRO_YOUT_H     0x45
#define MPU9250_GYRO_YOUT_L     0x46
#define MPU9250_GYRO_ZOUT_H     0x47
#define MPU9250_GYRO_ZOUT_L     0x48
#define MPU9250_PWR_MGMT_1      0x6B    //电源管理，典型值：0x00(正常启用)
#define MPU9250_WHO_AM_I        0x75  //IIC地址寄存器(默认数值0x68，只读)


#define MPU9250_MAG_XOUT_L      0x03
#define MPU9250_MAG_XOUT_H      0x04
#define MPU9250_MAG_YOUT_L      0x05
#define MPU9250_MAG_YOUT_H      0x06
#define MPU9250_MAG_ZOUT_L      0x07
#define MPU9250_MAG_ZOUT_H      0x08
#define MPU9250_MAG_CNTL1       0x0A
#define MPU9250_MAG_CNTL2       0x0B
#define MPU9250_MAG_ASAX        0x10
#define MPU9250_MAG_ASAY        0x11
#define MPU9250_MAG_ASAZ        0x12


#define MPU9250_SLAVE_ADDR      0x68
#define MPU9250_AK8963_ADDR     0x0C

#define MPU9250_BUFFER_MAX_SIZE 16

#define MPU9250_WHO_AM_I_VALUE 0x71

#define MPU_STATUS_ERROR (1)
#define MPU_STATUS_OK (0)

typedef struct{
    int16_t accelX;
    int16_t accelY;
    int16_t accelZ;
    int16_t temp;
    int16_t gryoX;
    int16_t gryoY;
    int16_t gryoZ;
    int16_t magX;
    int16_t magY;
    int16_t magZ;
}mpu9250Data_t;

typedef struct{
    uint8_t regAddr;
    uint8_t data[MPU9250_BUFFER_MAX_SIZE];
}mpu9250Buf_t;


uint8_t mpu9250Init();
void mpu9250Read(mpu9250Data_t *data);
#endif
