/****************************************************************************/
/*                                                                          */
/*              NDK Http HTML & CGI 页面                                    */
/*                                                                          */
/*              2014年09月25日                                              */
/*                                                                          */
/****************************************************************************/
#include <stdio.h>

#include <netmain.h>
#include <_stack.h>
#include <cgiparse.h>
#include <cgiparsem.h>
#include <console.h>
#include <stdint.h>

#pragma DATA_SECTION(index, "HTML");
#include <Web/index.h>
#pragma DATA_SECTION(EVM6748, "HTML");
#include <Web/Image/TL-EVM6748.h>

static int ImageFile(SOCKET htmlSock, int ContentLength, char *pArgs);
//static int CGILED(SOCKET htmlSock, int ContentLength, char *pArgs);
static int CGIFile( SOCKET htmlSock, int ContentLength, char *pArgs );
int CGINet(SOCKET htmlSock, int ContentLength, char *pArgs);
int web_iap_update(uint8_t type, void *pbuf, int32_t size);
extern void timerWatchDogInit(void);
/****************************************************************************/
/*                                                                          */
/*              添加 Web 文件到 EFS 文件系统                                */
/*                                                                          */
/****************************************************************************/
static int OurRealm = 1;

void AddWebFiles(void)
{
    void *pFxn;

    efs_createfile("index.html", index_SIZE, (unsigned char *)index);
    //efs_createfile("Image/TL-EVM6748.jpg", EVM6748_SIZE, (unsigned char *)EVM6748);

    pFxn = (void*)&CGINet;
    efs_createfile("net.cgi", 0, (UINT8*)pFxn);

    //pFxn = (void*)&ImageFile;
    //efs_createfile("image.cgi", 0, (UINT8*)pFxn);

    efs_createfile("protected/%R%", 4, (UINT8 *)&OurRealm );
//    pFxn = (void*)&CGILED;
//    efs_createfile("protected/led.cgi", 0, (UINT8*)pFxn);
    pFxn = (void*)&CGIFile;
    efs_createfile("protected/upload.cgi", 0, (UINT8*)pFxn);
}

/****************************************************************************/
/*                                                                          */
/*              移除 Web 文件到 EFS 文件系统                                */
/*                                                                          */
/****************************************************************************/
void RemoveWebFiles(void)
{
    efs_destroyfile("index.html");
    //efs_destroyfile("Image/TL-EVM6748.jpg");

    efs_destroyfile("protected/%R%");
//    efs_destroyfile("protected/led.cgi");
}

// HTML 代码生成宏定义
#define html(str) httpSendClientStr(htmlSock, (char *)str)

// HTML 宏定义
static const char *pstr_HTML_START = "<html><body text=#000000 bgcolor=#ffffff>";
static const char *pstr_HTML_END = "</body></html>\r\n";
static const char *pstr_ROW_START = "<tr>";
static const char *pstr_ROW_END = "</tr>\r\n";
static const char *pstr_TABLE_START = "<table border cellspacing=0 cellpadding=5>\r\n";
static const char *pstr_TABLE_END = "</table><br>\r\n";

static const char *tablefmt = "<tr><td>%s</td><td>%s</td></tr>\r\n";
static const char *tablefmtd = "<tr><td>%s</td><td>%d</td></tr>\r\n";

/****************************************************************************/
/*                                                                          */
/*              文件上传                                                    */
/*                                                                          */
/****************************************************************************/
static int ImageFile(SOCKET htmlSock, int ContentLength, char *pArgs )
{
    // 发送状态
    httpSendStatusLine(htmlSock, HTTP_OK, CONTENT_TYPE_JPG);
    html(CRLF);

    // 发送数据
   	send(htmlSock, (void *)&EVM6748, sizeof(EVM6748), 0);

	return(1);
}

/****************************************************************************/
/*                                                                          */
/*              LED 控制                                                    */
/*                                                                          */
/****************************************************************************/
/*
extern void LEDInit();
extern void LED(UINT8 led, UINT8 status);

static int CGILED(SOCKET htmlSock, int ContentLength, char *pArgs)
{
    // LED 初始化
    LEDInit();

    // 为当前请求分配缓存空间
    char *buffer;
    buffer = (char *)mmBulkAlloc(ContentLength);
    if(!buffer)
    	goto ERROR;

    // 从客户端接收数据
    if(recv(htmlSock, buffer, ContentLength, MSG_WAITALL) < 1)
    	goto ERROR;

    // 初始化变量
    int parseIndex;
    parseIndex = 0;
    buffer[ContentLength] = '\0';
    char *key;
    char *value;

    // 熄灭所有 LED
    LED(1,0);
    LED(2,0);
    LED(3,0);
    LED(4,0);

    // 判断参数
	do{
		key = cgiParseVars(buffer, &parseIndex);
		value = cgiParseVars(buffer, &parseIndex);

		if(!strcmp("led1", key))
			if(!strcmp("on", value))
				LED(1,1);

		if(!strcmp("led2", key))
			if(!strcmp("on", value))
				LED(2,1);

		if(!strcmp("led3", key))
			if(!strcmp("on", value))
				LED(3,1);

		if(!strcmp("led4", key))
			if(!strcmp("on", value))
				LED(4,1);
    	} while (parseIndex != -1);

    // 发送状态
    httpSendStatusLine(htmlSock, HTTP_OK, CONTENT_TYPE_HTML);
    html(CRLF);

    // 发送 HTML 页面
    html("<script>window.opener=null;window.close()</script>"); // 关闭当前网页

ERROR:
	if(buffer)
		mmBulkFree(buffer);

    // 返回 1 保持 Socket 连接
    return(1);
}
*/

/****************************************************************************/
/*                                                                          */
/*              文件上传                                                    */
/*                                                                          */
/****************************************************************************/
static int CGIFile(SOCKET htmlSock, int ContentLength, char *pArgs )
{
    CGIPARSEREC recs[8];
    int         len,numrecs,i;
    char        *buffer;
    char        htmlbuf[MAX_RESPONSE_SIZE];

    // 为当前请求分配缓存空间
    buffer = (char *)mmBulkAlloc(ContentLength);
    if(!buffer)
    	goto ERROR;

    // 从客户端接收数据
    len = recv(htmlSock, buffer, ContentLength, MSG_WAITALL);
    if(len < 1)
    	goto ERROR;

    // 分析表单数据并返回数据类型数目
    numrecs = cgiParseMultiVars(buffer, len, recs, 8);
    // 发送状态
    httpSendStatusLine(htmlSock, HTTP_OK, CONTENT_TYPE_HTML);
    html( CRLF );

    // 发送 HTML 页面
    html(pstr_HTML_START);
    html("<h1>文件信息</h1>");
    html(pstr_TABLE_START);

	for(i = 0; i < numrecs; i++)
	{
		sprintf(htmlbuf, tablefmtd, "索引:", i + 1);
		html(htmlbuf);

		sprintf(htmlbuf, tablefmt, "名称:", recs[i].Name);
		html(htmlbuf);

		if(recs[i].Filename)
		{
			if(!recs[i].Filename[0])
			 sprintf(htmlbuf, tablefmt, "文件名:", "<i>NULL</i>");
			else
			 sprintf(htmlbuf, tablefmt, "文件名:", recs[i].Filename);
			html(htmlbuf);
		}

		if(recs[i].Type)
		{
			sprintf(htmlbuf, tablefmt, "类型:", recs[i].Type);
			html(htmlbuf);
		}

		sprintf(htmlbuf, tablefmtd, "文件大小:", recs[i].DataSize);
		html(htmlbuf);

        if(recs[i].Filename[0])
        {
            if(0 == strncmp(recs[i].Filename,"fs_boot.ais",strlen(recs[i].Filename)))
            {
                sprintf(htmlbuf, tablefmt, "更新状态:", "Don't update Bootloader !!");
#if 0
                if(0 == web_iap_update(0,recs[i].Data,recs[i].DataSize))
                {
                    sprintf(htmlbuf, tablefmt, "更新状态:", "Bootloader update Success!!");
                }
                else
                {
                    sprintf(htmlbuf, tablefmt, "更新状态:", "Bootloader update Failed!!");
                }
#endif
            }
            else if(0 == strncmp(recs[i].Filename,"fs_station.bin",strlen(recs[i].Filename)))
            {
                if(0 == web_iap_update(1,recs[i].Data,recs[i].DataSize))
                {
                    sprintf(htmlbuf, tablefmt, "更新状态:", "Station update Success!!");
                    timerWatchDogInit();
                }
                else
                {
                    sprintf(htmlbuf, tablefmt, "更新状态:", "Station update Failed!!");
                }
            }
            else
            {
                sprintf(htmlbuf, tablefmt, "更新状态:", "Non-update file!!");
            }
            html(htmlbuf);
        }
		/*
		if(recs[i].DataSize < 512)
		{
			if(!recs[i].Data[0])
				sprintf(htmlbuf, tablefmt, "文件大小:", "<i>NULL</i>");
			else
				sprintf(htmlbuf, tablefmt, "文件大小:", recs[i].Data);
			html(htmlbuf);
		}
		*/
	}

	html(pstr_TABLE_END);
	html(pstr_HTML_END);

ERROR:
	if(buffer)
		mmBulkFree(buffer);

	// 返回 1 保持 Socket 连接
	return(1);
}

/****************************************************************************/
/*                                                                          */
/*              网络状态                                                    */
/*                                                                          */
/****************************************************************************/
static void CreateIPUse(SOCKET htmlSock);
static void CreatehtmlSockets(SOCKET htmlSock);
static void CreateRoute(SOCKET htmlSock);

int CGINet(SOCKET htmlSock, int ContentLength, char *pArgs)
{
//	  GET 方法
//    if(!ContentLength)
//    {
//        if(pArgs)
//        {
//            value = pArgs;
//            goto CHECKARGS;
//        }
//
//        http405(htmlSock);
//    }

	// 返回网络状态信息
	CreateIPUse(htmlSock);
	CreatehtmlSockets(htmlSock);
	CreateRoute(htmlSock);

	html(pstr_HTML_END);

	// 返回 1 保持 Socket 连接
	return(1);
}

void CreateIPUse(SOCKET htmlSock)
{
    IPN     myIP;
    IPN     yourIP;
    char    pszmyIP[32];
    char    pszyourIP[32];
    struct  sockaddr_in Info;
    int     InfoLength;
    char    tmpBuf[128];
    HOSTENT *dnsInfo;
    char    htmlbuf[MAX_RESPONSE_SIZE];
    int     rc;

    InfoLength = sizeof(Info);
    getsockname(htmlSock, (PSA)&Info, &InfoLength);
    myIP = Info.sin_addr.s_addr;
    NtIPN2Str(myIP, pszmyIP);

    InfoLength = sizeof(Info);
    getpeername(htmlSock, (PSA)&Info, &InfoLength);
    yourIP = Info.sin_addr.s_addr;
    NtIPN2Str(yourIP, pszyourIP);

    httpSendStatusLine(htmlSock, HTTP_OK, CONTENT_TYPE_HTML);
    html( CRLF );

    html("<h1>IP 地址</h1>\r\n");
    html(pstr_TABLE_START);
    html(pstr_ROW_START);
    sprintf(htmlbuf,"<td>HTTP 服务器 IP 地址</td><td>%s</td>", pszmyIP);
    html(htmlbuf);
    html(pstr_ROW_END);

    html(pstr_ROW_START);
    html("<td>HTTP 服务器主机名</td>");
    rc = DNSGetHostByAddr(myIP, tmpBuf, sizeof(tmpBuf));
    if(rc)
        sprintf(htmlbuf, "<td>%s</td>", DNSErrorStr(rc));
    else
    {
        dnsInfo = (HOSTENT*)tmpBuf;
        sprintf(htmlbuf, "<td>%s</td>", dnsInfo->h_name);
    }
    html(htmlbuf);
    html(pstr_ROW_END);

    html(pstr_ROW_START);
    sprintf(htmlbuf, "<td>客户端 IP 地址</td><td>%s</td>", pszyourIP);
    html(htmlbuf);
    html(pstr_ROW_END);

    html(pstr_ROW_START);
    html("<td>客户端主机名</td>");
    rc = DNSGetHostByAddr(yourIP, tmpBuf, sizeof(tmpBuf));
    if( rc )
        sprintf(htmlbuf, "<td>%s</td>", DNSErrorStr(rc));
    else
    {
        dnsInfo = (HOSTENT*)tmpBuf;
        sprintf(htmlbuf, "<td>%s</td>", dnsInfo->h_name);
    }
    html(htmlbuf);
    html(pstr_ROW_END);
    html(pstr_TABLE_END);
}

static void DumphtmlSockets(SOCKET htmlSock, uint htmlSockProt);
void CreatehtmlSockets(SOCKET htmlSock)
{
    html("<h1>TCP/IP Socket 状态</h1>\r\n");
    html("<h2>TCP Sockets</h2>\r\n");
    DumphtmlSockets(htmlSock, SOCKPROT_TCP);
    html("<h2>UDP Sockets</h2>\r\n");
    DumphtmlSockets(htmlSock, SOCKPROT_UDP);
}

static const char *States[] = {"CLOSED","LISTEN","SYNSENT","SYNRCVD",
                                "ESTABLISHED","CLOSEWAIT","FINWAIT1","CLOSING",
                                "LASTACK","FINWAIT2","TIMEWAIT"};

static void DumphtmlSockets(SOCKET htmlSock, uint htmlSockProt)
{
    UINT8   *pBuf;
    int     Entries,i;
    SOCKPCB *ppcb;
    char    str[32];
    char    htmlbuf[MAX_RESPONSE_SIZE];

    pBuf = mmBulkAlloc(2048);
    if( !pBuf )
        return;

    llEnter();
    Entries = SockGetPcb(htmlSockProt, 2048, pBuf);
    llExit();

    html(pstr_TABLE_START);
    html(pstr_ROW_START);
    html("<td>本地 IP</td><td>本地端口</td>");
    html("<td>远程 IP</td><td>远程端口</td>\r\n");
    if( htmlSockProt == SOCKPROT_TCP )
        html("<td>状态</td>\r\n");
    html(pstr_ROW_END);

    for(i=0; i<Entries; i++)
    {
        ppcb = (SOCKPCB *)(pBuf+(i*sizeof(SOCKPCB)));

        html(pstr_ROW_START);
        NtIPN2Str(ppcb->IPAddrLocal, str);
        sprintf(htmlbuf, "<td>%-15s</td><td>%-5u</td>", str, htons(ppcb->PortLocal));
        html(htmlbuf);
        NtIPN2Str(ppcb->IPAddrForeign, str);
        sprintf(htmlbuf, "<td>%-15s</td><td>%-5u</td>\r\n", str, htons(ppcb->PortForeign));
        html(htmlbuf);
        if(htmlSockProt == SOCKPROT_TCP)
        {
            sprintf(htmlbuf,"<td>%s</td>\r\n",States[ppcb->State]);
            html(htmlbuf);
        }
        html(pstr_ROW_END);
    }

    html(pstr_TABLE_END);

    mmBulkFree(pBuf);
}

void CreateRoute(SOCKET htmlSock)
{
    HANDLE  hRt,hIF,hLLI;
    uint    wFlags,IFType,IFIdx;
    UINT32  IPAddr,IPMask;
    char    str[32];
    UINT8   MacAddr[6];
    char    htmlbuf[MAX_RESPONSE_SIZE];

    html("<h1>TCP/IP 路由表</h1>\r\n");

    llEnter();
    hRt = RtWalkBegin();
    llExit();

    html(pstr_TABLE_START);
    html(pstr_ROW_START);
    html("<td>地址</td><td>子网掩码</td>");
    html("<td>标志</td><td>网关</td>\r\n");
    html(pstr_ROW_END);

    while(hRt)
    {
        html(pstr_ROW_START);

        llEnter();
        IPAddr = RtGetIPAddr(hRt);
        IPMask = RtGetIPMask(hRt);
        wFlags = RtGetFlags(hRt);
        hIF    = RtGetIF(hRt);
        if(hIF)
        {
            IFType = IFGetType(hIF);
            IFIdx  = IFGetIndex(hIF);
        }
        else
            IFType = IFIdx = 0;
        llExit();

        NtIPN2Str(IPAddr, str);
        sprintf(htmlbuf, "<td>%-15s</td>", str);
        html(htmlbuf);
        NtIPN2Str(IPMask, str);
        sprintf(htmlbuf, "<td>%-15s</td>", str);
        html(htmlbuf);

        if(wFlags & FLG_RTE_UP)
            strcpy(str,"U");
        else
            strcpy(str," ");
        if(wFlags & FLG_RTE_GATEWAY)
            strcat(str,"G");
        else
            strcat(str," ");
        if(wFlags & FLG_RTE_HOST)
            strcat(str,"H");
        else
            strcat(str," ");
        if(wFlags & FLG_RTE_STATIC)
            strcat(str,"S");
        else
            strcat(str," ");
        if(wFlags & FLG_RTE_CLONING)
            strcat(str,"C");
        else
            strcat(str," ");
        if(wFlags & FLG_RTE_IFLOCAL)
            strcat(str,"L");
        else
            strcat(str," ");

        sprintf(htmlbuf, "<td>%s</td>", str);
        html(htmlbuf);

        if(wFlags & FLG_RTE_GATEWAY)
        {
            llEnter();
            IPAddr = RtGetGateIP(hRt);
            llExit();
            NtIPN2Str(IPAddr, str);
            sprintf(htmlbuf, "<td>%-15s</td>", str);
            html(htmlbuf);
        }

        else if(IFType == HTYPE_ETH && (wFlags&FLG_RTE_HOST) && !(wFlags&FLG_RTE_IFLOCAL))
        {
            llEnter();
            if(!(hLLI = RtGetLLI( hRt )) || !LLIGetMacAddr(hLLI, MacAddr, 6))
                llExit();
            else
            {
                llExit();
                sprintf(htmlbuf,"<td>%02X:%02X:%02X:%02X:%02X:%02X</td>",
                           MacAddr[0], MacAddr[1], MacAddr[2],
                           MacAddr[3], MacAddr[4], MacAddr[5]);
                html(htmlbuf);
            }
        }

        else if(IFIdx)
        {
            if(wFlags & FLG_RTE_IFLOCAL)
            {
                sprintf(htmlbuf,"<td>local (if-%d)</td>", IFIdx);
                html(htmlbuf);
            }
            else
            {
                sprintf(htmlbuf,"<td>if-%d</td>", IFIdx);
                html(htmlbuf);
            }
        }

        html(pstr_ROW_END);

        llEnter();
        hRt = RtWalkNext(hRt);
        llExit();
    }
    llEnter();
    RtWalkEnd(0);
    llExit();

    html(pstr_TABLE_END);
}
