/******************************************************************************
*
* (c) Copyright 2009-2011 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xil_io.h
*
* This file contains the interface for the general IO component, which
* encapsulates the Input/Output functions for processors that do not
* require any special I/O handling.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 3.00a hbm  07/28/09 Initial release
* 3.00a hbm  07/21/10 Added Xil_EndianSwap32/16, Xil_Htonl/s, Xil_Ntohl/s
* 3.03a sdm  08/18/11 Added INST_SYNC and DATA_SYNC macros.
*
* </pre>
*
* @note
*
* This file may contain architecture-dependent items.
*
******************************************************************************/

#ifndef XIL_IO_DSP_H			/* prevent circular inclusions */
#define XIL_IO_DSP_H			/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"

/************************** Constant Definitions *****************************/

/*
 * The following macros allow optimized I/O operations for memory mapped I/O.
 * It should be noted that macros cannot be used if synchronization of the I/O
 * operation is needed as it will likely break some code.
 */

/*****************************************************************************/
/**
*
* Perform an input operation for an 8-bit memory location by reading from the
* specified address and returning the value read from that address.
*
* @param	Addr contains the address to perform the input operation at.
*
* @return	The value read from the specified input address.
*
* @note		None.
*
******************************************************************************/
#define Xil_In8(Addr)  (*(volatile u8  *)(Addr))

/*****************************************************************************/
/**
*
* Perform an input operation for a 16-bit memory location by reading from the
* specified address and returning the value read from that address.
*
* @param	Addr contains the address to perform the input operation at.
*
* @return	The value read from the specified input address.
*
* @note		None.
*
******************************************************************************/
//#define Xil_In16(Addr) (*(volatile u16 *)(Addr))
#define Xil_In16(Addr) (*(volatile unsigned short *)(Addr))

/*****************************************************************************/
/**
*
* Perform an input operation for a 32-bit memory location by reading from the
* specified address and returning the value read from that address.
*
* @param	Addr contains the address to perform the input operation at.
*
* @return	The value read from the specified input address.
*
* @note		None.
*
******************************************************************************/
#define Xil_In32(Addr)  (*(volatile u32 *)(Addr))


/*****************************************************************************/
/**
*
* Perform an output operation for an 8-bit memory location by writing the
* specified value to the specified address.
*
* @param	Addr contains the address to perform the output operation at.
* @param	value contains the value to be output at the specified address.
*
* @return	None
*
* @note		None.
*
******************************************************************************/
#define Xil_Out8(Addr, Value)  \
	(*(volatile u8  *)((Addr)) = (Value))

/*****************************************************************************/
/**
*
* Perform an output operation for a 16-bit memory location by writing the
* specified value to the specified address.
*
* @param	Addr contains the address to perform the output operation at.
* @param	value contains the value to be output at the specified address.
*
* @return	None
*
* @note		None.
*
******************************************************************************/
#define Xil_Out16(Addr, Value) \
	(*(volatile unsigned short *)((Addr)) = (Value))

/*****************************************************************************/
/**
*
* Perform an output operation for a 32-bit memory location by writing the
* specified value to the specified address.
*
* @param	addr contains the address to perform the output operation at.
* @param	value contains the value to be output at the specified address.
*
* @return	None
*
* @note		None.
*
******************************************************************************/
#define Xil_Out32(Addr, Value) \
	(*(volatile u32 *)((Addr)) = (Value))

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
