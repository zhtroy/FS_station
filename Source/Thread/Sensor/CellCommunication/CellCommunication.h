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

/*
 * TODO: 删除cell_state_t
 * 协议解析状态机的各个状态
 */
typedef enum{
	//等待分割字符串
	CELL_WAIT,
	//接收分割字符串
	CELL_DIV,
	//接收HEAD
	CELL_RECV_HEAD,
	//接收DATA
	CELL_RECV_DATA
}cell_state_new_t;
/*
 * 分割字符串
 * 包与包之前用这个分开
 */
extern char cell_packet_divider[];

void CellSendData(char * pbuff, uint32_t size);
/*
 * 接收4G数据， 将数据按 $command^分包后放入消息队列
 */
Void taskCellCommunication(UArg a0, UArg a1);


#endif /* CELLCOMMUNICATION_H_ */
