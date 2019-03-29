#ifndef _LOG_LIB_H
#define _LOG_LIB_H
#ifdef __cplusplus
extern "C" {
#endif

/* 宏定义 */
#define MAX_LOGARGS	(6)	            /* 消息携带的最大参数个数 */

#define LOG_TASK_PRIORITY (5)       /* log任务的优先级 */
#define LOG_TASK_STACK_SIZE  (4096) /* log任务占用的栈大小 */

#ifndef OK
#define OK (0)
#endif

#ifndef ERROR
#define ERROR (-1)
#endif

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

/* 类型定义 */
typedef struct				    /* log消息 */
{
    char *	fmt;			    /* 格式化字符串指针 */
    int		arg [MAX_LOGARGS];	/* 格式化字符串的参数 */
} logMsg_t;


/* 函数声明 */
int logInit(void);
int logMsg(char *fmt, int arg1, int arg2,int arg3,int arg4,int arg5,int arg6);


#ifdef __cplusplus
}
#endif

#endif
