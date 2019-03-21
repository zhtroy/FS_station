/*头文件声明*/
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>
#include <ti/sysbios/BIOS.h>
#include "uartStdio.h"
#include "logLib.h"

/* 静态全局变量声明 */
static Task_Handle logTaskId = NULL;            /* 任务句柄*/
static Semaphore_Handle logSem = NULL;          /* 信号量句柄*/
static Mailbox_Handle logMailbox = NULL;        /* 邮箱句柄*/
static int logMsgsLost = 0;	                    /* 丢失消息计数*/

/* 静态函数声明 */
static void logTask (void);
static void lprintf (char *fmt, int arg1, int arg2, int arg3, int arg4,
		     int arg5, int arg6);

/*****************************************************************************
* 函数名称: logInit
* 函数说明: 初始化log的任务、邮箱和信号量
* 输入参数: 无
* 输出参数: 无
* 返 回 值: 0(成功)/-1(失败)
* 备注:
*****************************************************************************/
int logInit(void)
{
    Mailbox_Params mboxParams;
    Task_Params taskParams;
    Semaphore_Params semParams;
    
    if (logTaskId != NULL)
    {
        /*任务已经被初始化了*/
        return (ERROR);         
    }
    
    Mailbox_Params_init(&mboxParams);
    logMailbox = Mailbox_create (sizeof (logMsg_t),MAX_LOGARGS, &mboxParams, NULL);
    if (logMailbox == NULL)
    {
        /*邮箱创建失败*/
        return (ERROR);
    }

    Task_Params_init(&taskParams);
	taskParams.priority = LOG_TASK_PRIORITY;
	taskParams.stackSize = LOG_TASK_STACK_SIZE;
    logTaskId = Task_create (logTask, &taskParams, NULL);

    if (logTaskId == NULL)
    {
        /*任务创建失败*/
        return (ERROR);
    }
    
	Semaphore_Params_init(&semParams);
	semParams.mode = Semaphore_Mode_BINARY;
	logSem = Semaphore_create(1, &semParams, NULL);

    if (logSem == NULL)
    {
        /*信用量创建失败*/
        return (ERROR);
    }

    return (OK);
}

/*****************************************************************************
* 函数名称: logMsg
* 函数说明: 初始化log的任务、邮箱和信号量
* 输入参数: 
*           fmt:参数格式化方式
*           arg1-arg6:待格式化参数
* 输出参数: 无
* 返 回 值: 消息长度/-1(失败)
* 备注:
*****************************************************************************/
int logMsg(char *fmt, int arg1, int arg2,int arg3,int arg4,int arg5,int arg6)
{
    logMsg_t msg;

    msg.fmt    = fmt;
    msg.arg[0] = arg1;
    msg.arg[1] = arg2;
    msg.arg[2] = arg3;
    msg.arg[3] = arg4;
    msg.arg[4] = arg5;
    msg.arg[5] = arg6;

    if(FALSE == Mailbox_post(logMailbox,(Ptr *)&msg,BIOS_NO_WAIT))
	{
        /*邮箱已满，消息丢失*/
	    ++logMsgsLost;
	    return (ERROR);
	}

    return (sizeof (msg));
}

/*****************************************************************************
* 函数名称: logTask
* 函数说明: 静态函数，log任务回调函数
* 输入参数: 无
*           
* 输出参数: 无
* 返 回 值: 无
* 备注:
*****************************************************************************/
static void logTask (void)
{
    static int oldMsgsLost;

    int newMsgsLost;	/* used in case logMsgsLost is changed during use */
    logMsg_t msg;
    char *checkName;

    while(1)
	{
    	if (FALSE == Mailbox_pend (logMailbox, (Ptr *) &msg, BIOS_WAIT_FOREVER))
    	{
            /*获取消息失败*/
    	    lprintf ("logTask: error reading log messages.\n", 0, 0, 0, 0, 0,0);
    	}
    	else
    	{
    	    if (msg.fmt == NULL)
    	    {
                /*无格式化信息*/
    		    lprintf ("<null \"fmt\" parameter>\n", 0, 0, 0, 0, 0, 0);
    	    }
    	    else
    		{
    		    lprintf (msg.fmt, msg.arg[0], msg.arg[1], msg.arg[2],
    				    msg.arg[3], msg.arg[4], msg.arg[5]);
    		}
        }

    	/* check for any more messages lost */

    	newMsgsLost = logMsgsLost;

    	if (newMsgsLost != oldMsgsLost)
    	{
    	    lprintf ("logTask: %d log messages lost.\n",
    		     newMsgsLost - oldMsgsLost, 0, 0, 0, 0, 0);

    	    oldMsgsLost = newMsgsLost;
    	}
	}/*end while(1)*/
}


/*****************************************************************************
* 函数名称: lprintf
* 函数说明: 静态函数，通过串口打印字符串
* 输入参数: 
*           fmt:参数格式化方式
*           arg1-arg6:待格式化参数
* 输出参数: 无
* 返 回 值: 无
* 备注:
*****************************************************************************/
static void lprintf(char *fmt,	int arg1,	int arg2,int arg3,int arg4,int arg5,int arg6 )
{
    /*添加信号量锁，保证打印信息的顺序*/
    Semaphore_pend (logSem, BIOS_WAIT_FOREVER);
    UARTprintf(fmt, arg1, arg2, arg3, arg4, arg5, arg6);
    Semaphore_post (logSem);
}
