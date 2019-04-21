/*
 * EPCdef.h
 *
 *  Created on: 2019-4-17
 *      Author: zhtro
 */

#ifndef EPCDEF_H_
#define EPCDEF_H_

#define EPC_STRAIGHT   			(0x01)
#define EPC_PRE_CURVE  			(0x02)
#define EPC_CURVING    			(0x03)
#define EPC_UPHILL	   			(0x04)
#define EPC_PRE_DOWNHILL  		(0x05)
#define EPC_DOWNHILL  			(0x06)
#define EPC_PRE_SEPERATE  		(0x07)
#define EPC_SEPERATE  			(0x08)
#define EPC_ENTER_STATION  		(0x09)
#define EPC_STOP_STATION  		(0x0A)
#define EPC_LEAVE_STATION  		(0x0B)
#define EPC_PRE_MERGE 			(0x0C)
#define EPC_MERGE  				(0x0D)

#define EPC_AUXILIARY_TRACK_START (0x55)
#define EPC_AUXILIARY_TRACK_END   (0xAA)

#endif /* EPCDEF_H_ */
