/*
 * hsmUtil.h
 *
 *  Created on: 2019-4-2
 *      Author: zhtro
 */

#ifndef HSMUTIL_H_
#define HSMUTIL_H_

#include "hsm.h"

/*
 * event 类型转换
 * pEvt 指向Msg及其子类的指针
 * type 想要转换成的子类
 *
 * 返回: type类型的指针
 */
#define EVT_CAST(pEvt, type) ((type *) pEvt)

/*
 * event 设置事件类型
 *
 * pEvt 指向Msg及其子类的指针
 * msg 想要切换的事件类型
 *
 * 返回 ：无
 */
#define EVT_SETTYPE(pEvt, msg) EVT_CAST(pEvt, Msg)->evt = (int) msg


#endif /* HSMUTIL_H_ */
