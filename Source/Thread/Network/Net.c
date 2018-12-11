/*
 * Net.c
 *
 *  Created on: 2018-12-11
 *      Author: zhtro
 */


#include <xdc/std.h>
#include <netmain.h>
#include "emifa/emifa_app.h"

// 函数必须要有两个参数
extern int TcpTest(SOCKET s, UINT32 unused);

static HANDLE hTcp = 0;


/****************************************************************************/
/*                                                                          */
/*              全局变量                                                    */
/*                                                                          */
/****************************************************************************/
// MAC 地址
unsigned char bMacAddr[8];

// 连接状态
char *LinkStr[] = {"No Link", "10Mb/s Half Duplex", "10Mb/s Full Duplex", "100Mb/s Half Duplex", "100Mb/s Full Duplex"};

/****************************************************************************/
/*                                                                          */
/*              回调函数 EMAC 初始化                                        */
/*                                                                          */
/****************************************************************************/
// 这个函数被驱动调用 不要修改函数名
void EMAC_initialize()
{
	//reset
	EMIFA_write(0x30,1);


	// 管脚复用配置
	// 使能 MII 模式
	EMACPinMuxSetup();
}

/****************************************************************************/
/*                                                                          */
/*              回调函数 获取 MAC 地址                                      */
/*                                                                          */
/****************************************************************************/
// 这个函数被驱动调用 不要修改函数名
void EMAC_getConfig(unsigned char *pMacAddr)
{
	// 根据芯片 ID 生成 MAC 地址
	bMacAddr[0] = 0x00;
	bMacAddr[1] = (*(volatile unsigned int *)(0x01C14008) & 0x0000FF00) >> 8;
	bMacAddr[2] = (*(volatile unsigned int *)(0x01C14008) & 0x000000FF) >> 0;
	bMacAddr[3] = (*(volatile unsigned int *)(0x01C1400C) & 0x0000FF00) >> 8;
	bMacAddr[4] = (*(volatile unsigned int *)(0x01C1400C) & 0x000000FF) >> 0;
	bMacAddr[5] = (*(volatile unsigned int *)(0x01C14010) & 0x000000FF) >> 0;
	UARTprintf("Using MAC Address: %02X-%02X-%02X-%02X-%02X-%02X\n",
    		bMacAddr[0], bMacAddr[1], bMacAddr[2], bMacAddr[3], bMacAddr[4], bMacAddr[5]);

    // 传递 MAC 地址
    mmCopy(pMacAddr, bMacAddr, 6);
}

/****************************************************************************/
/*                                                                          */
/*              回调函数 设置 MAC 地址                                      */
/*                                                                          */
/****************************************************************************/
// 这个函数被驱动调用 不要修改函数名
void EMAC_setConfig(unsigned char *pMacAddr)
{
    mmCopy(bMacAddr, pMacAddr, 6);
    UARTprintf(
            "Setting MAC Addr to: %02x-%02x-%02x-%02x-%02x-%02x\n",
            bMacAddr[0], bMacAddr[1], bMacAddr[2],
            bMacAddr[3], bMacAddr[4], bMacAddr[5]);
}

/****************************************************************************/
/*                                                                          */
/*              回调函数 获取连接状态                                       */
/*                                                                          */
/****************************************************************************/
// 这个函数被驱动调用 不要修改函数名
void EMAC_linkStatus(unsigned int phy, unsigned int linkStatus)
{
	UARTprintf("Link Status: %s on PHY %d\n",LinkStr[linkStatus],phy);
}

/****************************************************************************/
/*                                                                          */
/*              回调函数 获取 IP 地址                                       */
/*                                                                          */
/****************************************************************************/
void NetGetIPAddr(IPN IPAddr, unsigned int IfIdx, unsigned int fAdd)
{
	if(fAdd)
	{
        UARTprintf("Network Added: ", IfIdx);
	}
	else
	{
		UARTprintf("Network Removed: ", IfIdx);
	}

	char StrIP[16];
	NtIPN2Str(IPAddr, StrIP);
	UARTprintf("%s\r\n", StrIP);
}


void NetOpenHook()
{
    // 创建一个服务器 端口 1025
	hTcp = DaemonNew(SOCK_STREAMNC, 0, 1025, TcpTest, OS_TASKPRINORM, OS_TASKSTKNORM, 0, 3);
}

void NetCloseHook()
{
    DaemonFree(hTcp);
}
