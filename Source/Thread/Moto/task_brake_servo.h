#ifndef __TASK_BRAKE_SERVO_H
#define __TASK_BRAKE_SERVO_H
#include <stdint.h>

#define UART_DEV_2 (2)
#define SERVOR_MOTOR_UART UART_DEV_2

#define MODBUS_ACK_OK           (0)
#define MODBUS_ACK_NOTOK        (1)
#define MODBUS_ACK_CRC_ERR      (2)
#define MODBUS_ACK_FRAME_ERR    (3)
#define MODBUS_ACK_TIMEOUT		(4)

#define LEFTRAIL    (1)
#define RIGHTRAIL   (2)

#define SLEEPTIME 3

#define PI 3.141592654											//定义常量PI
#define DM 0.375												//轮子直径
#define KM 3.285714												//速比
#define DELTA 1.0												//减速度差阈值
#define AMAX 10.0												//最大减速度10m/s2
#define BRAKETIME 10											//刹车计算时间
#define MAXSTEP 100												//刹车行程总步数


typedef struct{
    uint8_t     id;
    uint8_t     cmd;
    uint8_t     addrH;
    uint8_t     addrL;
    uint8_t     dataH;
    uint8_t     dataL;
    uint16_t    crc;
} modbusCmd_t;



#endif

