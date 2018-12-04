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
}
