#include "mpu9250_iic.h"
#include "mpu9250_drv.h"
#include "soc_C6748.h" 
#include "interrupt.h"
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/BIOS.h>


static void mpu9250IntrHandler(void *callBackRef, uint32_t event, uint32_t eventData);

static Semaphore_Handle sem_txData;
static Semaphore_Handle sem_rxData;
static uint8_t dataBuf[14];
IICObj_t mpu9250IICInst;

static const IICConfig_t cfg = {
    SOC_I2C_0_REGS,
    24000000,
    8000000,
    100000,
    SYS_INT_I2C0_INT,
    mpu9250IntrHandler,
    NULL
};

static void mpu9250initSem()
{
    Semaphore_Params semParams;
	Semaphore_Params_init(&semParams);
    semParams.mode = Semaphore_Mode_BINARY;
    sem_rxData = Semaphore_create(0, &semParams, NULL);
    sem_txData = Semaphore_create(0, &semParams, NULL);
}

static void mpu9250IntrHandler(void *callBackRef, uint32_t event, uint32_t eventData)
{
    //IICObj_t *insPtr = (IICObj_t *)CallBackRef;
    
    if(IIC_EVENT_SEND_DATA == event || IIC_EVENT_SEND_ERROR == event)
        Semaphore_post(sem_txData);
    else;

    if(IIC_EVENT_SEND_DATA == event || IIC_EVENT_SEND_ERROR == event)
        Semaphore_post(sem_rxData);
    else;
}


static void mpu9250WriteBytes(uint8_t slvAddr,uint8_t regAddr,uint8_t *dataPtr,uint8_t numBytes)
{
    IICSetSlaveAddr(&mpu9250IICInst, slvAddr);
    IICSendBytes(&mpu9250IICInst, regAddr, dataPtr,numBytes+1);
    Semaphore_pend(sem_txData,BIOS_WAIT_FOREVER);
}

static void mpu9250ReadBytes(uint8_t slvAddr,uint8_t regAddr,uint8_t *dataPtr,uint8_t numBytes)
{
    IICSetSlaveAddr(&mpu9250IICInst, slvAddr);
    
    /*设置读取地址*/
    IICSendBytes(&mpu9250IICInst, regAddr,NULL, 1);
    Semaphore_pend(sem_txData,BIOS_WAIT_FOREVER);
    
    IICRecvBytes(&mpu9250IICInst, dataPtr, numBytes);
    Semaphore_pend(sem_rxData,BIOS_WAIT_FOREVER);
}

static void mpu9250WriteReg(uint8_t slvAddr,uint8_t regAddr,uint8_t data)
{
    mpu9250WriteBytes(slvAddr,regAddr,&data,1);
}

static void mpu9250ReadReg(uint8_t slvAddr,uint8_t regAddr,uint8_t *dataPtr)
{
    mpu9250ReadBytes(slvAddr,regAddr,dataPtr,1);
}


uint8_t mpu9250Init()
{
    uint8_t ureg;
    uint16_t udelay;
    /*初始化IIC配置*/
    IICCfgInit(&mpu9250IICInst,&cfg);
    
    mpu9250initSem();

    #if 0
    mpu9250ReadReg(MPU9250_SLAVE_ADDR,MPU9250_WHO_AM_I,&ureg);
    
    if(ureg != MPU9250_WHO_AM_I_VALUE)
        return MPU_STATUS_ERROR;
    else;
    #endif
    
    /*Reset registers of mpu9250*/
    mpu9250WriteReg(MPU9250_SLAVE_ADDR,MPU9250_PWR_MGMT_1,0x80);

    /*Delay for Reset*/
    for(udelay = 10000;udelay>0;udelay--);
    
    mpu9250WriteReg(MPU9250_SLAVE_ADDR,MPU9250_PWR_MGMT_1,0x00);
    
    /*Set Sample Rate:125Hz*/
    mpu9250WriteReg(MPU9250_SLAVE_ADDR,MPU9250_SMPLRT_DIV,0x07);

    /*Set DLPF of gyro and temperature:184Hz*/
    mpu9250WriteReg(MPU9250_SLAVE_ADDR,MPU9250_CONFIG,0x01);

    /*Set Scale of gyro:1000dps*/
    mpu9250WriteReg(MPU9250_SLAVE_ADDR,MPU9250_GYRO_CONFIG,0x10);
    
    /*Set Scale of accel:+/-4g*/
    mpu9250WriteReg(MPU9250_SLAVE_ADDR,MPU9250_ACCEL_CONFIG,0x08);
    /*Set DLPF of accel:21.2Hz*/
    mpu9250WriteReg(MPU9250_SLAVE_ADDR,MPU9250_ACCEL_CONFIG_2,0x04);

    /*Enable IIC Bypass Mode*/
    mpu9250WriteReg(MPU9250_SLAVE_ADDR,MPU9250_INT_PIN_CFG,0x02); 

    /*Set AK8963 Work Mode :16Bits,measured period:100Hz*/
    mpu9250WriteReg(MPU9250_AK8963_ADDR,MPU9250_MAG_CNTL1,0x16);
    
    return MPU_STATUS_OK;
    
}

void mpu9250Read(mpu9250Data_t *data)
{
    

    mpu9250ReadBytes(MPU9250_SLAVE_ADDR,MPU9250_ACCEL_XOUT_H,dataBuf,14);
    data->accelX = (int16_t)((dataBuf[0] << 8) + dataBuf[1]);
    data->accelY = (int16_t)((dataBuf[2] << 8) + dataBuf[3]);
    data->accelZ = (int16_t)((dataBuf[4] << 8) + dataBuf[5]);
    data->temp = (int16_t)((dataBuf[6] << 8) + dataBuf[7]);
    data->gryoX = (int16_t)((dataBuf[8] << 8)+ dataBuf[9]);
    data->gryoY = (int16_t)((dataBuf[10] << 8) + dataBuf[11]);
    data->gryoZ = (int16_t)((dataBuf[12] << 8) + dataBuf[13]);


    //mpu9250ReadBytes(MPU9250_AK8963_ADDR,MPU9250_MAG_XOUT_L,dataBuf,6);
    //data->magX = (int16_t)((dataBuf[1] << 8) + dataBuf[0]);
    //data->magY = (int16_t)((dataBuf[3] << 8) + dataBuf[2]);
    //data->magZ = (int16_t)((dataBuf[5] << 8) + dataBuf[4]);
    
}


