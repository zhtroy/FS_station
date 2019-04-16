#ifndef _FPGA_TTL_H_
#define _FPGA_TTL_H_

#include "stdint.h"

void TTLInit();
void TTLWrite(uint8_t value);
uint8_t TTLRead();
void TTLWriteBit(uint8_t bitNum, uint8_t value);
uint8_t TTLReadBit(uint8_t bitNum);
#endif/*_FPGA_TTL_H_ */
