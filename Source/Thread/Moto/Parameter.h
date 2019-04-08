/*
 * Parameter.h
 *
 *  Created on: 2019-4-8
 *      Author: zhtro
 */

#ifndef PARAMETER_H_
#define PARAMETER_H_

#include "stdint.h"

/*
 * FIXME: 因为网络传输的原因， pid 的参数现在还是 放大了 1000000倍的
 */
typedef struct pid_parameter_tag{

}pid_parameter_t;

typedef struct parameter_tag{
	uint32_t KI;
	uint32_t KP;
	uint32_t KU;

	/*是否允许变轨*/
    uint8_t EnableChangeRail;

}parameter_t;

extern parameter_t* ParamInstance();



#endif /* PARAMETER_H_ */
