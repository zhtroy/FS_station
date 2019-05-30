/*
 * TestEntry.c
 *
 *  Created on: 2018-12-4
 *      Author: zhtro
 */
#include "Test_.h"
#include "Test_config.h"


void TestEntry()
{

#ifdef TEST_UART_COMMAND_LINE
    testUARTCommandLineInit();
#endif 


#ifdef TEST_WATCHDOG
    testWatchDogTaskInit();
#endif

}
