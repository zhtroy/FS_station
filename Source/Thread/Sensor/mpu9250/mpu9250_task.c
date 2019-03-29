#include "mpu9250_drv.h"
/*SYSBIOS includes*/
#include <ti/sysbios/knl/Task.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "mpu.h"
#include "soc_C6748.h"
#include "uart.h"
#include <mlmath.h>

/* Starting sampling rate. */
#define DEFAULT_MPU_HZ  (5)

#define PEDO_READ_MS    (1000)
#define TEMP_READ_MS    (500)
#define COMPASS_READ_MS (100)
#define MPU_Q30  (1073741824.0f)
#define  Pitch_error  (1.0)
#define  Roll_error   (-2.0)
#define  Yaw_error    (0.0)


static signed char gyro_orientation[9] = {-1, 0, 0,
                                           0,-1, 0,
                                           0, 0, 1};
/* Every time new gyro data is available, this function is called in an
 * ISR context. In this example, it sets a flag protecting the FIFO read
 * function.
 */
# if 0
static Semaphore_Handle newGyroSem;
void gyro_data_ready_cb(void)
{
	Semaphore_post(newGyroSem);
	testCnt++;
}
#endif


/* These next two functions converts the orientation matrix (see
 * gyro_orientation) to a scalar representation for use by the DMP.
 * NOTE: These functions are borrowed from Invensense's MPL.
 */
static  unsigned short inv_row_2_scale(const signed char *row)
{
    unsigned short b;

    if (row[0] > 0)
        b = 0;
    else if (row[0] < 0)
        b = 4;
    else if (row[1] > 0)
        b = 1;
    else if (row[1] < 0)
        b = 5;
    else if (row[2] > 0)
        b = 2;
    else if (row[2] < 0)
        b = 6;
    else
        b = 7;      // error
    return b;
}



static  unsigned short inv_orientation_matrix_to_scalar(const signed char *mtx)
{
    unsigned short scalar;

    /*
       XYZ  010_001_000 Identity Matrix
       XZY  001_010_000
       YXZ  010_000_001
       YZX  000_010_001
       ZXY  001_000_010
       ZYX  000_001_010
     */

    scalar = inv_row_2_scale(mtx);
    scalar |= inv_row_2_scale(mtx + 3) << 3;
    scalar |= inv_row_2_scale(mtx + 6) << 6;


    return scalar;
}


/*自检函数*/
static void run_self_test(void)
{
   int result;

   long gyro[3], accel[3];

   result = mpu_run_self_test(gyro, accel);
   if (result == 0x7)
   {
       /* Test passed. We can trust the gyro data here, so let's push it down
        * to the DMP.
        */
       float sens;
       unsigned short accel_sens;
       mpu_get_gyro_sens(&sens);
       gyro[0] = (long)(gyro[0] * sens);
       gyro[1] = (long)(gyro[1] * sens);
       gyro[2] = (long)(gyro[2] * sens);
       dmp_set_gyro_bias(gyro);
       mpu_get_accel_sens(&accel_sens);
       accel[0] *= accel_sens;
       accel[1] *= accel_sens;
       accel[2] *= accel_sens;
       dmp_set_accel_bias(accel);
       logMsg("setting bias succesfully!!\r\n",0,0,0,0,0,0);
	//	printf("setting bias succesfully ......\n");
   }
	else
	{
	//	printf("bias has not been modified ......\n");
		logMsg("bias has not been modified ......\r\n",0,0,0,0,0,0);
	}

}

static void mpu9250Task(void)
{
    struct int_param_s int_param;
    Semaphore_Params semParams;
    unsigned long sensor_timestamp;
    short gyro[3], accel_short[3], compass[3], sensors;
	unsigned char more;
	long quat[4], temperature;
	int32_t result;
	unsigned short gyro_rate;
	float q0,q1,q2,q3;
	float Pitch,Roll,Yaw,Direction;


    /* IIC初始化 */
    mpu9250I2CInit();
#if 0
    /* 初始化信用量配置 */
	Semaphore_Params_init(&semParams);
	semParams.mode = Semaphore_Mode_BINARY;
	newGyroSem = Semaphore_create(0, &semParams, NULL);
#endif

	/* 初始化MPU9250*/
	result = mpu_init(&int_param);
	if (result) {
		logMsg("MPU9250 Init Failed!!\r\n",0,0,0,0,0,0);
	}

    /* 配置硬件，并启动传感器 */
    /* 唤醒传感器: 陀螺仪，加速度计和磁力计*/
    result = mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL | INV_XYZ_COMPASS);
    if (result) {
    	logMsg("mpu_set_sensors Failed!!\r\n",0,0,0,0,0,0);
    }

    /* 陀螺仪(gyro)和加速度 (accel)数据都放到FIFO中 */
    result = mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);
    if (result) {
		logMsg("mpu_configure_fifo Failed!!\r\n",0,0,0,0,0,0);
	}

    /* 设置陀螺仪和加速度计的采样速率 */
    result = mpu_set_sample_rate(DEFAULT_MPU_HZ);
    if (result) {
		logMsg("mpu_set_sample_rate Failed!!\r\n",0,0,0,0,0,0);
	}

    mpu_get_sample_rate(&gyro_rate);
    logMsg("mpu_get_sample_rate:%d!!\r\n",gyro_rate,0,0,0,0,0);
    /* 设置磁力计的采样速率 */
    mpu_set_compass_sample_rate(1000 / COMPASS_READ_MS);

    /* 初始化DMP */
    result = dmp_load_motion_driver_firmware();
    if (result) {
		logMsg("dmp_load_motion_driver_firmware Failed!!\r\n",0,0,0,0,0,0);
	}

    if(dmp_set_orientation(inv_orientation_matrix_to_scalar(gyro_orientation)))
    {
    	logMsg("dmp_set_orientation Failed!!\r\n",0,0,0,0,0,0);
    }

    result = dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT  | DMP_FEATURE_SEND_RAW_ACCEL |
    		           DMP_FEATURE_SEND_CAL_GYRO | DMP_FEATURE_GYRO_CAL);
    if (result) {
		logMsg("dmp_enable_feature Failed!!\r\n",0,0,0,0,0,0);
	}

    result = dmp_set_fifo_rate(DEFAULT_MPU_HZ);
    if (result) {
		logMsg("dmp_enable_feature Failed!!\r\n",0,0,0,0,0,0);
	}

//    result = dmp_set_interrupt_mode(DMP_INT_CONTINUOUS);
    if (result) {
		logMsg("dmp_enable_feature Failed!!\r\n",0,0,0,0,0,0);
	}

    mpu_reset_fifo();

    run_self_test();

    mpu_set_dmp_state(1);

    while(1){
//    	Semaphore_pend(newGyroSem,BIOS_WAIT_FOREVER);
    	do
    	{
    		result = dmp_read_fifo(gyro, accel_short, quat, &sensor_timestamp, &sensors, &more);
    	} while(more);
    	if(0 == result)
    	{
    		if (sensors & INV_WXYZ_QUAT )
			{
				q0 = quat[0] / MPU_Q30;
				q1 = quat[1] / MPU_Q30;
				q2 = quat[2] / MPU_Q30;
				q3 = quat[3] / MPU_Q30;

				Pitch  = asin(-2 * q1 * q3 + 2 * q0* q2)* 57.3 + Pitch_error; // pitch
				Roll = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2* q2 + 1)* 57.3 + Roll_error; // roll
				Yaw = atan2(2*(q1*q2 + q0*q3),q0*q0+q1*q1-q2*q2-q3*q3) * 57.3 + Yaw_error;
			}
    		logMsg("Gyro: %06d,%06d,%06d\r\n",gyro[0], gyro[1], gyro[2], 0, 0, 0);
    		logMsg("Aceel: x-%d,y-%d,z-%d\r\n",accel_short[0], accel_short[1], accel_short[2], 0, 0, 0);
    		logMsg("quat: %d,%d,%d\r\n",Pitch, Roll, Yaw, 0, 0, 0);

    		mpu_get_temperature(&temperature, &sensor_timestamp);
    		//logMsg("Temptrature: %d(%d)\r\n",temperature, sensor_timestamp, 0, 0, 0, 0);

    		mpu_get_compass_reg(compass, &sensor_timestamp);
    		Direction=	atan2((double) (compass[0]),
    						  (double) (compass[1])
    						 )*(180/M_PI)+180;

    		logMsg("Compass: Direction-%d,z-%d\r\n",Direction, compass[2], 0, 0, 0, 0);
    		//logMsg("%d(%d)\r\n",sensor_timestamp, result, 0, 0, 0, 0);
    	}/*end if(0 == result)*/
        Task_sleep(1000/DEFAULT_MPU_HZ);
    }

}


void testMPU9250TaskInit(void)
{
    Task_Handle task;
	Task_Params taskParams;
    Task_Params_init(&taskParams);
    
	taskParams.priority = 5;
	taskParams.stackSize = 4096;
    
	task = Task_create(mpu9250Task, &taskParams, NULL);
	if (task == NULL) {
		System_printf("Task_create() failed!\n");
		BIOS_exit(0);
	}
}

