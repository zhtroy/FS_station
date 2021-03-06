#ifndef __CLICOMMON_H__
#define __CLICOMMON_H__

#include <assert.h>
#include <stdlib.h>
#include "uartStdio.h"
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define pdFALSE			( ( long ) 0 )
#define pdTRUE			( ( long ) 1 )

#define pdPASS			( pdTRUE )
#define pdFAIL			( pdFALSE )

#define configCOMMAND_INT_MAX_OUTPUT_SIZE 1024


#define taskENTER_CRITICAL() 
#define taskEXIT_CRITICAL() 
#define configASSERT(expr) assert(expr)
#define pvPortMalloc(size) malloc(size)
#define vSerialPutString(pTxBuffer,numBytesToWrite) LogPuts(pTxBuffer, numBytesToWrite)
#define xSerialGetChar() UARTGetc()
//#define xSerialPutChar(byteTx) UARTPutc(byteTx)

typedef long BaseType_t;
typedef unsigned long UBaseType_t;


#ifdef __cplusplus
}
#endif

#endif /* PORTMACRO_H */

