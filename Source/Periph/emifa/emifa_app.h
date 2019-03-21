#ifndef _EMIFA_APP_H_
#define _EMIFA_APP_H_

#include "stdint.h"

/* 函数声明 */
void emifaInit(void);
uint8_t emifaReadByte(uint32_t addr, uint32_t offset);
void emifaWriteByte(uint32_t addr, uint32_t offset, uint8_t value);
uint16_t emifaReadWord(uint32_t addr, uint32_t offset);
void emifaWriteWord(uint32_t addr, uint32_t offset, uint16_t value);
uint32_t emifaReadDword(uint32_t addr, uint32_t offset);
void emifaWriteDword(uint32_t addr, uint32_t offset, uint32_t value);

#endif /* EMIFA_APP_H_ */
