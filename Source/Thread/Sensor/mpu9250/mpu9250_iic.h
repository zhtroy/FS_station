#ifndef __MPU9250_IIC_H__
#define __MPU9250_IIC_H__

#include <stdint.h>

/***** MPU9250寄存器地址 ***********/
#define MPU9250_SMPLRT_DIV      0x19    //陀螺仪采样率，典型值：0x07(125Hz)
#define MPU9250_CONFIG          0x1A    //低通滤波频率，典型值：0x06(5Hz)
#define MPU9250_GYRO_CONFIG     0x1B    //陀螺仪自检及测量范围，典型值：0x18(不自检，2000deg/s)
#define MPU9250_ACCEL_CONFIG    0x1C    //加速计自检、测量范围及高通滤波频率，典型值：0x01(不自检，2G，5Hz)
#define MPU9250_ACCEL_CONFIG_2  0x1D

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

#define MPU9250_MAG_XOUT_L      0x03
#define MPU9250_MAG_XOUT_H      0x04
#define MPU9250_MAG_YOUT_L      0x05
#define MPU9250_MAG_YOUT_H      0x06
#define MPU9250_MAG_ZOUT_L      0x07
#define MPU9250_MAG_ZOUT_H      0x08


#define MPU9250_PWR_MGMT_1      0x6B    //电源管理，典型值：0x00(正常启用)
#define MPU9250_WHO_AM_I        0x75    //IIC地址寄存器(默认数值0x68，只读)

#define IIC_EVENT_RECV_DATA     (1) /**< Data has been received */
#define IIC_EVENT_RECV_ERROR    (2) /**< When Data is receiving, transmit NACK or STOP */
#define IIC_EVENT_SEND_DATA     (3) /**< All Data has been sent */
#define IIC_EVENT_SEND_ERROR    (4) /**< When Data is sending, receive NACK or transmit STOP */


typedef struct {
    uint8_t *nextBytePtr;
    uint8_t requestedBytes;
    uint8_t remainingBytes;
} IICBuffer_t;

typedef void (*IIC_Handler)(void *callBackRef, uint32_t event,
                    uint32_t eventData);

typedef struct {
    uint32_t    baseAddr;
    uint8_t     slaveAddr;
    uint8_t     intrEventID;
    uint8_t     regAddr;
    IICBuffer_t sendBuffer;     /**< Send Buffer */
    IICBuffer_t recvBuffer;     /**< Receive Buffer */
    IIC_Handler handler;        /**< Call back handler */
    void *callBackRef;          /* Callback reference for control handler */
} IICObj_t;


typedef struct {
    uint32_t    devAddr;
    uint32_t    sysClk;
    uint32_t    internalClk;
    uint32_t    iicFreq;
    uint8_t     intrEventID;
    IIC_Handler handler;        /**< Call back handler */
    void *callBackRef;          /* Callback reference for control handler */
} IICConfig_t;


void IICCfgInit(IICObj_t *insPtr,const IICConfig_t *cfgPtr);
void IICSendBytes(IICObj_t *insPtr,uint8_t regAddr,uint8_t *bufPtr,uint8_t numBytes);
void IICRecvBytes(IICObj_t *insPtr,uint8_t *bufPtr,uint8_t numBytes);
void IICSetSlaveAddr(IICObj_t *insPtr,uint8_t addr);


#endif
