/*
 * NDK.c
 *
 *  Created on: 2018-12-12
 *      Author: zhtro
 */


/****************************************************************************/
/*                                                                          */
/*              广州创龙电子科技有限公司                                    */
/*                                                                          */
/*              Copyright (C) 2015 Tronlong Electronic Technology Co.,Ltd   */
/*                                                                          */
/****************************************************************************/
// C 语言函数库
#define LOG_LVL ELOG_LVL_DEBUG
#define LOG_TAG "ndk"
#include <stdio.h>
#include <string.h>

// NDK
#include <ti/ndk/inc/netmain.h>
#include <ti/ndk/inc/_stack.h>

#include <ti/ndk/inc/nettools/inc/configif.h>

#include <ti/ndk/inc/tools/console.h>
#include <ti/ndk/inc/tools/servers.h>

// SYS/BIOS
#include <ti/sysbios/knl/task.h>
#include <ti/sysbios/knl/Clock.h>

// 外设驱动库
#include "common.h"

#include "emifa/emifa_app.h"


#include "net_msg/msg.h"

#include "Test_config.h"
#include "easyflash.h"
#include "soc_C6748.h"
#include "psc.h"
#include "elog.h"

#include <ti/ndk/nettools/sntp/sntp.h>

#define FPGA_LAN8710_RST    (SOC_EMIFA_CS2_ADDR + (0x18<<1))
/****************************************************************************/
/*                                                                          */
/*              全局变量                                                    */
/*                                                                          */
/****************************************************************************/
// NDK 运行标志
static char NDKFlag = 0;
static unsigned int lastPHYstatus = 4;

// MAC 地址
unsigned char bMacAddr[8];

// 连接状态
char *LinkStr[] = {"No Link", "10Mb/s Half Duplex", "10Mb/s Full Duplex", "100Mb/s Half Duplex", "100Mb/s Full Duplex"};


// 字符串
char *VerStr = "\nTronlong TCP/IP NDK Application ......\r\n";

// 静态 IP 配置
#define SUB_NET "192.168.127"     //不用加括号
char *HostName     = "DSP_C6748";
char  LocalIPAddr[16]  = SUB_NET".XXX";          // 本机IP = 192.168.127 + carID
char *LocalIPMask  = "255.255.0.0";    // DHCP 模式下无效
char *GatewayIP    = SUB_NET".0";    // DHCP 模式下无效
char *DomainName   = "x.51dsp.net";      // DHCP 模式下无效
// 114.114.114.114 国内公共 DNS 服务器
char *DNSServer    = "114.114.114.114";  // 使用时设置为非 0 值

// DHCP 选项
unsigned char DHCP_OPTIONS[] = {DHCPOPT_SERVER_IDENTIFIER, DHCPOPT_ROUTER};

// 句柄
static HANDLE hTcp = 0;



#define SIZE (sizeof(struct sockaddr_in))
static unsigned char ntpServers[SIZE];





/****************************************************************************/
/*                                                                          */
/*              函数声明                                                    */
/*                                                                          */
/****************************************************************************/
void NetworkOpen();
void NetworkClose();
void NetIPAddrChange(IPN IPAddr, unsigned int IfIdx, unsigned int fAdd);

void ServiceReport(unsigned int Item, unsigned int Status, unsigned int Report, HANDLE h);
void DHCPReset(unsigned int IfIdx, unsigned int fOwnTask);
void DHCPStatus();

void AddWebFiles();
void RemoveWebFiles();
void TaskNDKInit();

void EMAC_init()
{
	PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_EMAC, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);
}
/****************************************************************************/
/*                                                                          */
/*              回调函数 EMAC 初始化                                        */
/*                                                                          */
/****************************************************************************/
// 这个函数被驱动调用 不要修改函数名
void EMAC_initialize()
{
	//复位PHY芯片 R127
	EMIFAWriteWord(FPGA_LAN8710_RST,0, 1);
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
	sb_printf("Using MAC Address: %02X-%02X-%02X-%02X-%02X-%02X\n",
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
    sb_printf(
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
	sb_printf("\r\nLink Status: %s on PHY %d\n", LinkStr[linkStatus], phy);

//	if(lastPHYstatus==0 && linkStatus!=0 && NDKFlag)
//	{
//		sb_puts("Link Status has changed!Ready to restart NDK Stack!\n", -2);
//
//		sb_puts("Stoping ......\n", -2);
//		NC_NetStop(1);
//		NDKFlag = 0;
//		sb_puts("Starting ......\n", -2);
//	}
//
//	lastPHYstatus = linkStatus;

}

static uint32_t sync_time = 0;
static uint32_t sync_tick = 0;
uint32_t gettime(void)
{
    return (sync_time + (Clock_getTicks()/1000) - sync_tick);
}

static void settime(uint32_t newtime)
{
    sync_time = newtime+(8*60*60);
    sync_tick = (Clock_getTicks()/1000);
    log_i("NetWork Setting Time:%d",sync_time);
}
/****************************************************************************/
/*                                                                          */
/*              NDK 打开                                                    */
/*                                                                          */
/****************************************************************************/
void NetworkOpen()
{
    struct sockaddr_in  ntp_server_addr;
    int currPos = 0;
    // 服务
    hTcp = DaemonNew(SOCK_STREAMNC, 0, 1000, dtask_tcp_echo, OS_TASKPRINORM, OS_TASKSTKNORM, 0, 3);
    SNTP_start(gettime, settime, 2048);

    ntp_server_addr.sin_family = AF_INET;
    ntp_server_addr.sin_port = htons(123);
    ntp_server_addr.sin_addr.s_addr = inet_addr("192.168.1.100");
    memcpy((ntpServers + currPos), &ntp_server_addr, sizeof(struct sockaddr_in));
    currPos += sizeof(struct sockaddr_in);

    SNTP_setservers((struct sockaddr *)ntpServers, 1);


#ifdef TEST_MSG_LOOPBACK
    msgtestloopback(g_sysParam.carID, 42);
#endif

}

/****************************************************************************/
/*                                                                          */
/*              NDK 关闭                                                    */
/*                                                                          */
/****************************************************************************/
void NetworkClose()
{

    // 关闭控制台
    ConsoleClose();
    SNTP_stop();

    // 删除任务
    TaskSetPri(TaskSelf(), NC_PRIORITY_LOW);

    DaemonFree(hTcp);

    msg_teardown();

}

/****************************************************************************/
/*                                                                          */
/*              IP 地址                                                     */
/*                                                                          */
/****************************************************************************/
void NetworkIPAddr(IPN IPAddr, unsigned int IfIdx, unsigned int fAdd)
{
	if(fAdd)
	{
        sb_printf("Network Added: ", IfIdx);
	}
	else
	{
		sb_printf("Network Removed: ", IfIdx);
	}

	char StrIP[16];
	NtIPN2Str(IPAddr, StrIP);
	sb_printf("%s\r\n", StrIP);
}

/****************************************************************************/
/*                                                                          */
/*              服务状态                                                    */
/*                                                                          */
/****************************************************************************/
char *TaskName[]  = {"Telnet", "HTTP", "NAT", "DHCPS", "DHCPC", "DNS"};
char *ReportStr[] = {"", "Running", "Updated", "Complete", "Fault"};
char *StatusStr[] = {"Disabled", "Waiting", "IPTerm", "Failed", "Enabled"};

void ServiceReport(unsigned int Item, unsigned int Status, unsigned int Report, HANDLE h)
{
	sb_printf("Service Status: %9s: %9s: %9s: %03d\n",
			     TaskName[Item - 1], StatusStr[Status], ReportStr[Report / 256], Report & 0xFF);

    // 配置 DHCP
    if(Item == CFGITEM_SERVICE_DHCPCLIENT &&
       Status == CIS_SRV_STATUS_ENABLED &&
       (Report == (NETTOOLS_STAT_RUNNING|DHCPCODE_IPADD) ||
        Report == (NETTOOLS_STAT_RUNNING|DHCPCODE_IPRENEW)))
    {
        IPN IPTmp;

        // 配置 DNS
        IPTmp = inet_addr(DNSServer);
        if(IPTmp)
        {
            CfgAddEntry(0, CFGTAG_SYSINFO, CFGITEM_DHCP_DOMAINNAMESERVER, 0, sizeof(IPTmp), (UINT8 *)&IPTmp, 0);
        }

        // DHCP 状态
        DHCPStatus();
    }

    // 重置 DHCP 客户端服务
    if(Item == CFGITEM_SERVICE_DHCPCLIENT && (Report & ~0xFF) == NETTOOLS_STAT_FAULT)
    {
        CI_SERVICE_DHCPC dhcpc;
        int tmp;

        // 取得 DHCP 入口数据(传递到 DHCP_reset 索引)
        tmp = sizeof(dhcpc);
        CfgEntryGetData(h, &tmp, (UINT8 *)&dhcpc);

        // 创建 DHCP 复位任务（当前函数是在回调函数中执行所以不能直接调用该函数）
        TaskCreate(DHCPReset, "DHCPreset", OS_TASKPRINORM, 0x1000, dhcpc.cisargs.IfIdx, 1, 0);
    }
}

/****************************************************************************/
/*                                                                          */
/*              DHCP 重置                                                   */
/*                                                                          */
/****************************************************************************/
void DHCPReset(unsigned int IfIdx, unsigned int fOwnTask)
{
    CI_SERVICE_DHCPC dhcpc;
    HANDLE h;
    int rc, tmp;
    unsigned int idx;

    // 如果在新创建的任务线程中被调用
    // 等待实例创建完成
    if(fOwnTask)
    {
        TaskSleep(500);
    }

    // 在接口上检索 DHCP 服务
    for(idx = 1; ; idx++)
    {
        // 寻找 DHCP 实例
        rc = CfgGetEntry(0, CFGTAG_SERVICE, CFGITEM_SERVICE_DHCPCLIENT, idx, &h);
        if(rc != 1)
        {
            goto RESET_EXIT;
        }

        // 取得 DHCP 实例数据
        tmp = sizeof(dhcpc);
        rc = CfgEntryGetData(h, &tmp, (UINT8 *)&dhcpc);

        if((rc<=0) || dhcpc.cisargs.IfIdx != IfIdx)
        {
            CfgEntryDeRef(h);
            h = 0;

            continue;
        }

        // 移除当前 DHCP 服务
        CfgRemoveEntry(0, h);

        // 在当前接口配置 DHCP 服务
        bzero(&dhcpc, sizeof(dhcpc));
        dhcpc.cisargs.Mode   = CIS_FLG_IFIDXVALID;
        dhcpc.cisargs.IfIdx  = IfIdx;
        dhcpc.cisargs.pCbSrv = &ServiceReport;
        CfgAddEntry(0, CFGTAG_SERVICE, CFGITEM_SERVICE_DHCPCLIENT, 0, sizeof(dhcpc), (UINT8 *)&dhcpc, 0);

        break;
    }

RESET_EXIT:
    if(fOwnTask)
    {
        TaskExit();
    }
}

/****************************************************************************/
/*                                                                          */
/*              DHCP 状态                                                   */
/*                                                                          */
/****************************************************************************/
void DHCPStatus()
{
    char IPString[16];
    IPN IPAddr;
    int i, rc;

    // 扫描 DHCPOPT_SERVER_IDENTIFIER 服务
    sb_printf("\nDHCP Server ID:\n");
    for(i = 1; ; i++)
    {
        // 获取 DNS 服务
        rc = CfgGetImmediate(0, CFGTAG_SYSINFO, DHCPOPT_SERVER_IDENTIFIER, i, 4, (UINT8 *)&IPAddr);
        if(rc != 4)
        {
            break;
        }

        // IP 地址
        NtIPN2Str(IPAddr, IPString);
        sb_printf("DHCP Server %d = '%s'\n", i, IPString);
    }

    if(i == 1)
    {
    	sb_printf("None\n\n");
    }
    else
    {
    	sb_printf("\n");
    }

    // 扫描 DHCPOPT_ROUTER 服务
    sb_printf("Router Information:\n");
    for(i = 1; ; i++)
    {
        // 获取 DNS 服务
        rc = CfgGetImmediate(0, CFGTAG_SYSINFO, DHCPOPT_ROUTER, i, 4, (UINT8 *)&IPAddr);
        if(rc != 4)
        {
            break;
        }

        // IP 地址
        NtIPN2Str(IPAddr, IPString);
        sb_printf("Router %d = '%s'\n", i, IPString);
    }

    if(i == 1)
    {
    	sb_printf("None\n\n");
    }
    else
    {
    	sb_printf("\n");
    }
}

/****************************************************************************/
/*                                                                          */
/*              任务（Task）线程                                            */
/*                                                                          */
/****************************************************************************/
Void NDKTask(UArg a0, UArg a1)
{
    int rc;
    uint16_t device_id;

    // 初始化操作系统环境
    // 必须在使用 NDK 之前最先调用
    rc = NC_SystemOpen(NC_PRIORITY_HIGH, NC_OPMODE_INTERRUPT);
    if(rc)
    {
    	sb_printf("NC_SystemOpen Failed (%d)\n", rc);

        for(;;);
    }

    // 创建新的配置
    HANDLE hCfg;
    hCfg = CfgNew();
    if(!hCfg)
    {
    	sb_printf("Unable to create configuration\n");

        goto Exit;
    }

    // 配置主机名
    if(strlen( DomainName ) >= CFG_DOMAIN_MAX || strlen( HostName ) >= CFG_HOSTNAME_MAX)
    {
    	sb_printf("Names too long\n");

        goto Exit;
    }

    // 添加全局主机名到 hCfg(对所有连接域有效)
    CfgAddEntry(hCfg, CFGTAG_SYSINFO, CFGITEM_DHCP_HOSTNAME, 0, strlen(HostName), (UINT8 *)HostName, 0);

    // 静态 IP 配置,使用车辆ID当作地址
    ef_get_env_blob("device_id",&device_id,sizeof(device_id),NULL);
    msg_id2ip(device_id,LocalIPAddr);


	CI_IPNET NA;
	CI_ROUTE RT;
	IPN      IPTmp;

	// 配置 IP
	bzero(&NA, sizeof(NA));
	NA.IPAddr = inet_addr(LocalIPAddr);
	NA.IPMask = inet_addr(LocalIPMask);
	strcpy(NA.Domain, DomainName);
	NA.NetType = 0;

	// 添加地址到接口 1
	CfgAddEntry(hCfg, CFGTAG_IPNET, 1, 0, sizeof(CI_IPNET), (UINT8 *)&NA, 0);

	// 配置 默认网关
	bzero(&RT, sizeof(RT));
	RT.IPDestAddr = 0;
	RT.IPDestMask = 0;
	RT.IPGateAddr = inet_addr(GatewayIP);

	// 配置 路由
	CfgAddEntry(hCfg, CFGTAG_ROUTE, 0, 0, sizeof(CI_ROUTE), (UINT8 *)&RT, 0);

	// 配置 DNS 服务器
	IPTmp = inet_addr(DNSServer);
	if(IPTmp)
	CfgAddEntry(hCfg, CFGTAG_SYSINFO, CFGITEM_DHCP_DOMAINNAMESERVER, 0, sizeof(IPTmp), (UINT8 *)&IPTmp, 0);

	// 输出配置信息
	sb_printf("\r\n\r\nIP Address is set to %s\n", LocalIPAddr);
	sb_printf("IP subnet mask is set to %s\n", LocalIPMask);
	sb_printf("IP default gateway is set to %s\r\n\r\n", GatewayIP);


    // 配置 TELNET 服务
    CI_SERVICE_TELNET telnet;

    bzero(&telnet, sizeof(telnet));
    telnet.cisargs.IPAddr = INADDR_ANY;
    telnet.cisargs.pCbSrv = &ServiceReport;
    telnet.param.MaxCon   = 2;
    telnet.param.Callback = &ConsoleOpen;
    CfgAddEntry(hCfg, CFGTAG_SERVICE, CFGITEM_SERVICE_TELNET, 0, sizeof(telnet), (UINT8 *)&telnet, 0);

    // 配置 HTTP 相关服务
    // 添加静态网页文件到 RAM EFS 文件系统
    AddWebFiles();

    // HTTP 身份认证
	CI_ACCT CA;

	// 命名 HTTP 服务身份认证组(最大长度 31)
	CfgAddEntry(hCfg, CFGTAG_SYSINFO, CFGITEM_SYSINFO_REALM1, 0, 30, (UINT8 *)"DSP_CLIENT_AUTHENTICATE1", 0 );

	// 创建用户
	// 用户名 密码均为 admin
	strcpy(CA.Username, "admin");
	strcpy(CA.Password, "admin");
	CA.Flags = CFG_ACCTFLG_CH1;  // 成为 realm 1 成员
	rc = CfgAddEntry(hCfg, CFGTAG_ACCT, CFGITEM_ACCT_REALM, 0, sizeof(CI_ACCT), (UINT8 *)&CA, 0);

	// 配置 HTTP 服务
    CI_SERVICE_HTTP http;
    bzero(&http, sizeof(http));
    http.cisargs.IPAddr = INADDR_ANY;
    http.cisargs.pCbSrv = &ServiceReport;
    CfgAddEntry(hCfg, CFGTAG_SERVICE, CFGITEM_SERVICE_HTTP, 0, sizeof(http), (UINT8 *)&http, 0);

    // 配置协议栈选项

    // 显示警告消息
    rc = DBG_INFO;
    CfgAddEntry(hCfg, CFGTAG_OS, CFGITEM_OS_DBGPRINTLEVEL, CFG_ADDMODE_UNIQUE, sizeof(uint), (UINT8 *)&rc, 0);

    // 设置 TCP 和 UDP buffer 大小（默认 8192）
    // TCP 发送 buffer 大小
    rc = 8192;
    CfgAddEntry(hCfg, CFGTAG_IP, CFGITEM_IP_SOCKTCPTXBUF, CFG_ADDMODE_UNIQUE, sizeof(uint), (UINT8 *)&rc, 0);

    // TCP 接收 buffer 大小(copy 模式)
    rc = 8192;
    CfgAddEntry(hCfg, CFGTAG_IP, CFGITEM_IP_SOCKTCPRXBUF, CFG_ADDMODE_UNIQUE, sizeof(uint), (UINT8 *)&rc, 0);

    // TCP 接收限制大小(non-copy 模式)
    rc = 8192;
    CfgAddEntry(hCfg, CFGTAG_IP, CFGITEM_IP_SOCKTCPRXLIMIT, CFG_ADDMODE_UNIQUE, sizeof(uint), (UINT8 *)&rc, 0);

    // UDP 接收限制大小
    rc = 8192;
    CfgAddEntry(hCfg, CFGTAG_IP, CFGITEM_IP_SOCKUDPRXLIMIT, CFG_ADDMODE_UNIQUE, sizeof(uint), (UINT8 *)&rc, 0);

#if 1
    // TCP Keep Idle(2 秒)
    rc = 20;
    CfgAddEntry(hCfg, CFGTAG_IP, CFGITEM_IP_TCPKEEPIDLE, CFG_ADDMODE_UNIQUE, sizeof(uint), (UINT8 *)&rc, 0);

    // TCP Keep Interval(0.5秒)
    rc = 5;
    CfgAddEntry(hCfg, CFGTAG_IP, CFGITEM_IP_TCPKEEPINTVL, CFG_ADDMODE_UNIQUE, sizeof(uint), (UINT8 *)&rc, 0);

    // TCP Max Keep Idle(3 秒)
    rc = 30;
    CfgAddEntry(hCfg, CFGTAG_IP, CFGITEM_IP_TCPKEEPMAXIDLE, CFG_ADDMODE_UNIQUE, sizeof(uint), (UINT8 *)&rc, 0);

    // socket 连接超时 (1 s)
    rc = 2;
    CfgAddEntry(hCfg, CFGTAG_IP, CFGITEM_IP_SOCKTIMECONNECT, CFG_ADDMODE_UNIQUE, sizeof(uint), (UINT8 *)&rc, 0);
#endif

    // 使用当前配置启动 NDK 网络

    do
    {
        NDKFlag = 1;
        rc = NC_NetStart(hCfg, NetworkOpen, NetworkClose, NetworkIPAddr);
    } while(rc > 0);

    // 停止消息
    sb_printf("NDK Task has been stop(Return Code %d)!\r\n", rc);

    // 移除 WEB 文件
    RemoveWebFiles();

    // 删除配置
    CfgFree(hCfg);

    // 退出
    goto Exit;

Exit:
    NC_SystemClose();

    TaskExit();
}

/****************************************************************************/
/*                                                                          */
/*              任务（Task）线程初始化                                      */
/*                                                                          */
/****************************************************************************/
void TaskNDKInit()
{
    // NDK 任务
	Task_Handle NDKTaskHandle;
	Task_Params TaskParams;

	Task_Params_init(&TaskParams);
	TaskParams.stackSize = 512 * 1024;
	TaskParams.priority = 5;

	NDKTaskHandle = Task_create(NDKTask, &TaskParams, NULL);
	if(NDKTaskHandle == NULL)
	{
		sb_printf("NDK Task create failed!\r\n");
	}
}
