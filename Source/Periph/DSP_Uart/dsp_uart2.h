/*
 * dsp_uart2.h
 *
 *  Created on: 2018-12-8
 *      Author: zhtro
 */

#ifndef DSP_UART2_H_
#define DSP_UART2_H_

extern void dsp_uart2_init();
extern unsigned int UART2Puts(char *pTxBuffer, int numBytesToWrite);
unsigned char UART2Getc(void);

#endif /* DSP_UART2_H_ */
