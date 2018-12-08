/**
 * \file     gpio.h
 *
 * \brief    This file contains the function prototypes for the device
 *           abstraction layer for GPIO and some related macros.
 */

/*
* Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
*/
/*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

#ifndef      __GPIO_APP_H__
#define      __GPIO_APP_H__

#include "hw_gpio.h"
#ifdef __cplusplus
extern "C" {
#endif

/************* GPIO pin directions.************************************/

/* This is used to configure a GPIO pin as an input pin. */
#define GPIO_DIR_INPUT                1
/* This is used to configure a GPIO pin as an output pin.*/
#define GPIO_DIR_OUTPUT               0

/******************Interrupt Trigger Level Types.**********************/

/* Disable interrupt generation on both the edges of a signal on a pin.*/
#define GPIO_INT_TYPE_NOEDGE          0

/* Enable interrupt generation on falling edge of a signal on a pin.*/
#define GPIO_INT_TYPE_FALLEDGE        1

/* Enable interrupt generation on the rising edge of a signal on a pin.*/
#define GPIO_INT_TYPE_RISEDGE         2

/* Enable interrupt generation on both the edges of a signal on a pin.*/
#define GPIO_INT_TYPE_BOTHEDGE        3

/*****************Interrupt Pending status.*****************************/

/* This signifies interrupt status as cleared.*/
#define GPIO_INT_NOPEND               0

/* This signifies interrupt status as pending.*/
#define GPIO_INT_PEND                 1

/*****************Write values to a pin.********************************/

/* This is used to write a logic 0 to a pin.*/
#define GPIO_PIN_LOW                  0

/* This is used to write a logic 1 to a pin.*/
#define GPIO_PIN_HIGH                 1

/* Pin Multiplexing bit mask to select GP6[0] pin. */
#define PINMUX19_GPIO6_0_ENABLE (SYSCFG_PINMUX19_PINMUX19_27_24_GPIO6_0 << SYSCFG_PINMUX19_PINMUX19_27_24_SHIFT)
/* Pin Multiplexing bit mask to select GP6[15] pin. */
#define PINMUX13_GPIO6_15_ENABLE (SYSCFG_PINMUX13_PINMUX13_3_0_GPIO6_15 << SYSCFG_PINMUX13_PINMUX13_3_0_SHIFT)
/* Pin Multiplexing bit mask to select GP6[0] pin. */
#define PINMUX13_GPIO6_14_ENABLE (SYSCFG_PINMUX13_PINMUX13_7_4_GPIO6_14 << SYSCFG_PINMUX13_PINMUX13_7_4_SHIFT)
/* Pin Multiplexing bit mask to select GP6[15] pin. */
#define PINMUX1_GPIO0_0_ENABLE (SYSCFG_PINMUX1_PINMUX1_31_28_GPIO0_0 << SYSCFG_PINMUX1_PINMUX1_31_28_SHIFT)
/* Pin Multiplexing bit mask to select GP6[0] pin. */
#define PINMUX1_GPIO0_1_ENABLE (SYSCFG_PINMUX1_PINMUX1_27_24_GPIO0_1 << SYSCFG_PINMUX1_PINMUX1_27_24_SHIFT)

#define PINMUX19_GPIO2_15_ENABLE (SYSCFG_PINMUX5_PINMUX5_3_0_GPIO2_15 << SYSCFG_PINMUX5_PINMUX5_3_0_SHIFT)

/*****************************************************************************
**                   FUNCTION DECLARATIONS
*****************************************************************************/
void gpio_init(void);
void gpio_fpga_rst(void);
void gpio_toggle_led(void);
//void GPIOTESTIsr(void);

#ifdef __cplusplus
}
#endif
#endif
















