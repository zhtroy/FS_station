/*
 * emifa.h
 *
 *  Created on: 2018��11��25��
 *      Author: hk
 */

#ifndef EMIFA_APP_H_
#define EMIFA_APP_H_

#include "stdint.h"

void EMIFA_init(void);
uint8_t EMIFAReadUart(uintptr_t *addr,uint8_t offset);
void EMIFAWriteUart(uintptr_t Addr,uint8_t offset,uint8_t Value);




#endif /* EMIFA_APP_H_ */
