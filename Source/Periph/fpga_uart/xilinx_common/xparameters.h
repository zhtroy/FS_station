#include "soc_C6748.h"			    // DSP C6748 外设寄存器

#ifndef XIL_XPARAMETERS_H	/* prevent circular inclusions */
#define XIL_XPARAMETERS_H	/* by using protection macros */

#define XPAR_DEFAULT_BAUD_RATE 115200

#define XPAR_XUARTNS550_NUM_INSTANCES 1
#define XPAR_UARTNS550_0_DEVICE_ID 0

#define XPAR_UARTNS550_0_DEVICE_ID 0
#define XPAR_UARTNS550_0_BASEADDR (SOC_EMIFA_CS2_ADDR+0x200)
#define XPAR_UARTNS550_0_CLOCK_HZ 80000000

#endif
