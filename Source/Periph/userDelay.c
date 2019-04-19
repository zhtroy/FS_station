#include <xdc/runtime/Timestamp.h>
#include <xdc/runtime/Types.h>
#include "common.h"

/*****************************************************************************
 * 函数名称: int32_t userGetMS(int32_t *countMS)
 * 函数说明: 获取毫秒时间戳
 * 输入参数: 无
 * 输出参数:
 *        countMS: 毫秒计数指针
 * 返 回 值: 0(成功)/-1(失败)
 * 备注:
*****************************************************************************/
int32_t userGetMS(int32_t *countMS)
{
    Types_FreqHz freq;
    Types_Timestamp64 timestamp;
    long long timecycle;
    long long freqency;
    Timestamp_getFreq(&freq);
    Timestamp_get64(&timestamp);
    timecycle = _itoll(timestamp.hi, timestamp.lo);
    freqency  = _itoll(freq.hi, freq.lo);
    countMS[0] = timecycle/(freqency/1000);
    return 0;
}




