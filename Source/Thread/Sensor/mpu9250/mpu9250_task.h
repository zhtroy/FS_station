#ifndef _MPU9250_TASK_H
#define _MPU9250_TASK_H

typedef struct _MPU9250_DATA_OBJ{
	float accelX; 		/*加速度*/
	float accelY;
	float accelZ;
	float pitch;		/*俯仰*/
	float roll;			/*横滚*/
	float yaw;			/*偏航*/
	float direct;		/*磁力计：方向*/
	float horizontal;	/*磁力计:水平*/
	float temp;			/*温度*/
}mpu9250DataObj_t;


typedef struct _MPU9250_DATA_CORRECT{
	int16_t xMax;
	int16_t xMin;
	int16_t yMax;
	int16_t yMin;
	int16_t zMax;
	int16_t zMin;
}mpu9250DataCorrect_t;

void MPU9250GetSelfTestMode(uint8_t *mode);
void MPU9250SetSelfTestMode(uint8_t mode);
void MPU9250CompassCorrectEnable(void);
void MPU9250CompassCorrectDisable(void);
void MPU9250TaskInit(void);

#endif
