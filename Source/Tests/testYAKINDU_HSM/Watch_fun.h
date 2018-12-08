/*
 * Watch_fun.h
 *
 *  Created on: 2018年12月5日
 *      Author: zhtro
 */

#ifndef WATCH_FUN_H_
#define WATCH_FUN_H_

#include <stdint.h>

typedef struct{
	uint16_t sec;
	uint16_t min;
	uint16_t hour;
	uint16_t day;
	uint16_t month;

}timedate_t;

typedef  enum{
	MODE,
	SET,
	TICK,
	NONE
}EventType;

extern void showTime(timedate_t * t);
extern void showDate(timedate_t * t);
extern void tickTime(timedate_t * t);
extern void initTime(timedate_t * t);


#endif /* WATCH_FUN_H_ */
