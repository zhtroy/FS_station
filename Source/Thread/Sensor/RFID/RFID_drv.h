/*
 * RFID_drv.h
 *
 *  Created on: 2018-11-13
 *      Author: zhtro
 *
 *  RFID 驱动
 */

#ifndef RFID_DRV_H_
#define RFID_DRV_H_

#include "stdint.h"


typedef void (*RFIDcallback) (uint8_t type, uint8_t data[], uint32_t len );
//API
//-------------------------------//

void RFIDDeviceInit(uint16_t uartDeviceNum);
void RFIDRegisterReadCallBack(RFIDcallback cb);
//循环查询EPC
int RFIDStartLoopCheckEpc();
int RFIDStopLoopCheckEpc();

//处理串口输入字符
void RFIDProcess();
void RFIDPendForData();




#endif /* RFID_DRV_H_ */
