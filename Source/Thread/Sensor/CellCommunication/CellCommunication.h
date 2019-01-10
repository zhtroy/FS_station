/*
 * CellCommunication.h
 *
 *  Created on: 2018-12-31
 *      Author: zhtro
 */

#ifndef CELLCOMMUNICATION_H_
#define CELLCOMMUNICATION_H_

#include "stdint.h"
#include <xdc/std.h>


#define CELL_BUFF_SIZE (128)
#define LTE_WORK_MODE (1)
#define LTE_TEST_MODE (0)

typedef enum{
	cell_wait,
	cell_recv
}cell_state_t;

#define CELL_HEAD ('$')
#define CELL_END  ('^')


void CellSendData(char * pbuff, uint32_t size);
/*
 * 接收4G数据， 将数据按 $command^分包后放入消息队列
 */
Void taskCellCommunication(UArg a0, UArg a1);


#endif /* CELLCOMMUNICATION_H_ */
