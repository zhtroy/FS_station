#ifndef _COMMON_H_
#define _COMMON_H_

#include "stdint.h"
#include "stdio.h"

extern void sb_puts(const char * chr, int32_t len);
extern void sb_printf(const char *fmt, ...);

#include "debugLog.h"
#include "devStatus.h"
#include "userDelay.h"


#endif
