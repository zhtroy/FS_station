/*
 * net_shell_cmd.c
 *
 *  Created on: 2020-1-11
 *      Author: DELL
 */
#include <netmain.h>
#include <_stack.h>
#include <_oskern.h>
#include <ti/ndk/inc/tools/console.h>
#include "shell.h"

#define MAX_TFTP_FILE_SIZE (8*1024*1024)
extern int web_iap_update(uint8_t type, void *pbuf, int32_t size);

static int test(int argc,char **argv)
{
    ConCmdTest(argc-1, argv[1], argv[2]);
    return 0;
}
MSH_CMD_EXPORT(test, test net work status);

static int ping(int argc, char **argv)
{
    ConCmdPing(argc-1, argv[1], argv[2]);
    return 0;
}
MSH_CMD_EXPORT(ping, Test echo request);

static int ipaddr(int argc, char **argv)
{
    ConCmdIPAddr ( argc-1, argv[1], argv[2], argv[3], argv[4] );
    return 0;
}
MSH_CMD_EXPORT(ipaddr, Configuration of IPAddress);

static int route(int tmp, char **tok)
{
    ConCmdRoute( tmp-1, tok[1], tok[2], tok[3], tok[4] );
    return 0;
}
MSH_CMD_EXPORT(route, Maintain route table);

static int acct(int tmp, char **tok)
{
    ConCmdAcct( tmp-1, tok[1], tok[2], tok[3], tok[4] );
    return 0;
}
MSH_CMD_EXPORT(acct, Manage PPP user accounts);

static int cmdSocket(int tmp, char **tok)
{
    ConCmdSocket( tmp-1, tok[1] );
    return 0;
}
SHELL_EXPORT_CMD(socket,cmdSocket,Print socket table);


static int cmdMem(int tmp, char **tok)
{
    _mmCheck(MMCHECK_MAP, &ConPrintf );
    return 0;
}
SHELL_EXPORT_CMD(nmem,cmdMem,Display memory status);

static int stat(int tmp, char **tok)
{
    ConCmdStat( tmp-1, tok[1] );
    return 0;
}
MSH_CMD_EXPORT(stat, Manage PPP user accounts);


static void tftpUpdate( IPN IPAddr, char *File )
{
    int    rc;
    char   *buffer;
    UINT16 ErrorCode;
    UINT32 Size;

    buffer = mmBulkAlloc(MAX_TFTP_FILE_SIZE);
    if( !buffer )
    {
        ConPrintf("\nFailed allocating temp buffer\n");
        return;
    }

    Size = MAX_TFTP_FILE_SIZE;
    rc = NtTftpRecv( IPAddr, File, buffer, &Size, &ErrorCode );

    if( rc >= 0 )
    {
        ConPrintf("\nFile Retrieved: Size is %d\n",Size);

        if(0 == web_iap_update(1,buffer,Size))
        {
            ConPrintf("Station update Success!! !!\n\nReboot...");
            timerWatchDogInit();
        }
        else
        {
            ConPrintf("Station update Failed!! !!\n");
        }
    }
    else if( rc < 0 )
    {
        ConPrintf("\nTFTP Reported Error: %d\n",rc);
        if( rc == TFTPERROR_ERRORREPLY )
            ConPrintf("TFTP Server Error: %d (%s)\n",ErrorCode,buffer);
    }

    mmBulkFree( buffer );
}



static void ConCmdUpdate( int ntok, char *tok1, char *tok2 )
{
    IPN    IPAddr;

    /* Check for 'stat ip' */
    if( ntok == 0 )
    {
        ConPrintf("\n[TFTP Command]\n");
        ConPrintf("\nCalled to retrieve a file from a TFTP server.\n\n");
        ConPrintf("update x.x.x.x myfile  - Retrieve 'myfile' from IP address\n");
        ConPrintf("update hostname myfile - Resolve 'hostname' and retrieve 'myfile'\n\n");
        ConPrintf("<File max size %dbytes>\n\n",MAX_TFTP_FILE_SIZE);
    }
    else if( ntok == 2 )
    {
       if( !ConStrToIPN( tok1, &IPAddr ) )
           ConPrintf("Invalid address\n\n");
       else
           tftpUpdate( IPAddr, tok2 );
    }
    else
        ConPrintf("\nIllegal argument. Type 'update' for help\n");
}

static int update(int tmp, char **tok)
{
    ConCmdUpdate( tmp-1, tok[1] ,tok[2]);
    return 0;
}
MSH_CMD_EXPORT(update, IAP(In Application Programming) by tftp.);


