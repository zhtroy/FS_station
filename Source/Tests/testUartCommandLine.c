#include "CLICommon.h"
#include <stdint.h>
extern void vUARTCommandConsoleStart( uint16_t usStackSize, UBaseType_t uxPriority ); 

void testUARTCommandLineInit()
{
    vUARTCommandConsoleStart(2048,1);
}

