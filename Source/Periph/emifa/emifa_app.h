#ifndef _EMIFA_APP_H_
#define _EMIFA_APP_H_

#include "stdint.h"

/* 函数声明 */
void EMIFAInit(void);
uint8_t EMIFAReadByte(uint32_t addr, uint32_t offset);
void EMIFAWriteByte(uint32_t addr, uint32_t offset, uint8_t value);
uint16_t EMIFAReadWord(uint32_t addr, uint32_t offset);
void EMIFAWriteWord(uint32_t addr, uint32_t offset, uint16_t value);
uint32_t EMIFAReadDword(uint32_t addr, uint32_t offset);
void EMIFAWriteDword(uint32_t addr, uint32_t offset, uint32_t value);

#endif /* EMIFA_APP_H_ */
