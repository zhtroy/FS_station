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
