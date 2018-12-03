/*
 * Decision.c
 *
 *  Created on: 2018-12-2
 *      Author: zhtro
 */
#include "Message/Message.h"
#include <xdc/std.h>



Void DecisionTask(UArg a0, UArg a1)
{
	p_msg_t msg;

	while(1){
		msg= Message_pend();

		//map msg to hsm msg

		//hsm input

	}
}
