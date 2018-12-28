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
#ifdef TEST_STATE_MACHINE
	testStateMachine_init();
#endif

#ifdef TEST_RFID_TASK
	testRFIDtask();
#endif

#ifdef TEST_CAN_TASK
    testCantaskInit();
#endif

#ifdef TEST_YAKINDU_SM
    testYAKINDU_SM_init();
#endif

#ifdef TEST_SONIC_RADAR
    testSonicRadar_init();
#endif


#ifdef TEST_UART_COMMAND_LINE
    testUARTCommandLineInit();
#endif 


#ifdef TEST_WATCHDOG
    testWatchDogTaskInit();
#endif


#ifdef TEST_NDK
    TaskNDKInit();
#endif

#ifdef TEST_CELL_COM
    testCellCom_init();
#endif

#ifdef TEST_MOTO_TASK
testMototaskInit();
#endif 

#ifdef TEST_BREAK_TASK
testBrakeServoInit();
#endif

}
