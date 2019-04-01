/*
 * NetPacket.h
 *
 *  Created on: 2019-2-27
 *      Author: zhtro
 */

#ifndef NETPACKET_H_
#define NETPACKET_H_

#include "stdint.h"

/*
 * 通信包定义
 */
#define HDR_LEN (32)
#define CELL_DATA_MAX_LEN (1024)

typedef struct net_packet{
	//header part
	uint8_t 	start[4];
	uint16_t	cks;
	uint8_t		ver;
	uint8_t		flag;

	uint16_t	len;
	uint16_t	cmd;
	uint32_t	req;

	uint32_t	src_cab;
	uint32_t	dst_cab;

	uint8_t		rsv[8];
	//data part
	char 		data[CELL_DATA_MAX_LEN];
}net_packet_t;

net_packet_t* net_packet_build_header_from_raw(net_packet_t* p, char* raw);

net_packet_t* net_packet_ctor(net_packet_t* p,
							  uint8_t flag,
							  uint16_t cmd,
							  uint32_t req,
							  uint32_t src_cab,
							  uint32_t dst_cab,
							  const char * p_data,
							  uint16_t data_len);

uint16_t net_packet_to_netorder(net_packet_t * p);

#endif /* NETPACKET_H_ */
