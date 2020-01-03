/*
 * TestEntry.c
 *
 *  Created on: 2018-12-4
 *      Author: zhtro
 */
#include "Test_.h"
#include "Test_config.h"
#include "Station/s2c_com_new.h"


void TestEntry()
{

#ifdef TEST_UART_COMMAND_LINE
    testUARTCommandLineInit();
#endif 

    testEasylogTask();

    testShellTask();

#ifdef TEST_NDK
    TaskNDKInit();
#endif
    S2CTaskInit();

#ifdef TEST_WATCHDOG
    testWatchDogTaskInit();
#endif

}
