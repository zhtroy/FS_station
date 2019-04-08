/*
 * emifa.h
 *
 *  Created on: 2018��11��25��
 *      Author: hk
 */

#ifndef EMIFA_APP_H_
#define EMIFA_APP_H_

#include "stdint.h"
#include "soc_C6748.h"			    // DSP C6748 外设寄存器

void EMIFA_init(void);
uint8_t EMIFAReadUart(uintptr_t *addr,uint8_t offset);
void EMIFAWriteUart(uintptr_t Addr,uint8_t offset,uint8_t Value);

#define EMIFA_read(RegOffset) \
		(*(volatile unsigned short *)((SOC_EMIFA_CS2_ADDR) + (RegOffset)))

#define EMIFA_write(RegOffset, RegisterValue) \
		(*(volatile unsigned short *)((SOC_EMIFA_CS2_ADDR) + (RegOffset))) = (RegisterValue)

uint16_t EMIFAReadWord(uint32_t addr, uint32_t offset);
void EMIFAWriteWord(uint32_t addr, uint32_t offset, uint16_t value);


#endif /* EMIFA_APP_H_ */
