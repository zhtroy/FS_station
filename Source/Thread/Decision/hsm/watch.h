/*
 * watch.h
 *
 *  Created on: 2018-11-13
 *      Author: zhtro
 */

#ifndef WATCH_H_
#define WATCH_H_


enum WatchEvents {
  Watch_MODE_EVT,
  Watch_SET_EVT,
  Watch_TICK_EVT
};

void Watch_init();
void Watch_Event(enum WatchEvents e);

#endif /* WATCH_H_ */
