#include "emifa.h"         
#include "hw_syscfg0_C6748.h"       
#include "soc_C6748.h"              
#include "psc.h" 
#include "emifa_app.h"
#include "emifa/EMIFAPinmuxSetup.h"
#include "common.h"

/*****************************************************************************
 * 函数名称: pscInit
 * 函数说明: 使能EMIFA电源，并配置为永不睡眠
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 无
 * 备注: 
*****************************************************************************/
static void pscInit(void)
{

	PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_EMIFA, PSC_POWERDOMAIN_ALWAYS_ON,
					 PSC_MDCTL_NEXT_ENABLE);
}

/*****************************************************************************
 * 函数名称: emifaSetup
 * 函数说明: 配置EMIFA的管脚映射，工作模式等
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 无
 * 备注: 
*****************************************************************************/
static void emifaSetup(void)
{

    /* 管脚映射 */
	EMIFAPinMuxSetup();

    /* 设置位宽为：16Bits */
	EMIFAAsyncDevDataBusWidthSelect(SOC_EMIFA_0_REGS,EMIFA_CHIP_SELECT_2,
									EMIFA_DATA_BUSWITTH_16BIT);

	/*异步Normal模式*/
	EMIFAAsyncDevOpModeSelect(SOC_EMIFA_0_REGS,EMIFA_CHIP_SELECT_2,
							   EMIFA_ASYNC_INTERFACE_NORMAL_MODE);

	/* Disable WAIT信号 */
	EMIFAExtendedWaitConfig(SOC_EMIFA_0_REGS,EMIFA_CHIP_SELECT_2,
							 EMIFA_EXTENDED_WAIT_DISABLE);

	/* 时序参数W_SETUP/R_SETUP   W_STROBE/R_STROBE    W_HOLD/R_HOLD	TA */
	EMIFAWaitTimingConfig(SOC_EMIFA_0_REGS,EMIFA_CHIP_SELECT_2,
						   EMIFA_ASYNC_WAITTIME_CONFIG(0, 3, 1, 1, 8, 2, 0 ));
}

/*****************************************************************************
 * 函数名称: emifaInit
 * 函数说明: 初始化EMIFA
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 无
 * 备注: 
*****************************************************************************/
void emifaInit()
{
	pscInit();
	emifaSetup();
}

/*****************************************************************************
 * 函数名称: emifaReadByte
 * 函数说明: EMIFA读函数，每次读一个Byte
 * 输入参数: 
 *           addr：基地址
 *           offset：偏移量
 * 输出参数: 无
 * 返 回 值: EMIFA返回值
 * 备注: emifaReadByte(0x00000000,0x4) -> 获取地址0x00000004的数据 
*****************************************************************************/
uint8_t emifaReadByte(uint32_t addr, uint32_t offset)
{
    return *((volatile uint8_t *)addr + offset);
}

/*****************************************************************************
 * 函数名称: emifaWriteByte
 * 函数说明: EMIFA读函数，每次读一个Byte
 * 输入参数: 
 *           addr：基地址
 *           offset：偏移量
 *           value：写入数据
 * 输出参数: 无
 * 返 回 值: 无
 * 备注: emifaWriteByte(0x00000000,0x4,0xAA) -> 将0xAA写入到地址0x00000004
*****************************************************************************/
void emifaWriteByte(uint32_t addr, uint32_t offset, uint8_t value)
{
    *((volatile uint8_t *)addr + offset) = value;
}

/*****************************************************************************
 * 函数名称: emifaReadWord
 * 函数说明: EMIFA读函数，每次读一个Word(2Bytes)
 * 输入参数: 
 *           addr：基地址
 *           offset：偏移量
 * 输出参数: 无
 * 返 回 值: EMIFA返回值
 * 备注: emifaReadWord(0x00000000,0x4) -> 获取地址0x00000008（偏移4个Word）的
 *       数据 
*****************************************************************************/
uint16_t emifaReadWord(uint32_t addr, uint32_t offset)
{
    return *((volatile uint16_t *)addr + offset);
}

/*****************************************************************************
 * 函数名称: emifaWriteWord
 * 函数说明: EMIFA读函数，每次读一个Word（2Bytes）
 * 输入参数: 
 *           addr：基地址
 *           offset：偏移量
 *           value：写入数据
 * 输出参数: 无
 * 返 回 值: 无
 * 备注: emifaWriteWord(0x00000000,0x4,0xAA) -> 将0xAA写入到地址0x00000008（偏
 *       移4个Word）的数据
*****************************************************************************/
void emifaWriteWord(uint32_t addr, uint32_t offset, uint16_t value)
{
    *((volatile uint16_t *)addr + offset) = value;
}

/*****************************************************************************
 * 函数名称: emifaReadDword
 * 函数说明: EMIFA读函数，每次读一个Dword(4Bytes)
 * 输入参数: 
 *           addr：基地址
 *           offset：偏移量
 * 输出参数: 无
 * 返 回 值: EMIFA返回值
 * 备注: emifaReadDword(0x00000000,0x4) -> 获取地址0x00000010（偏移4个Dword）的
 *       数据 
*****************************************************************************/
uint32_t emifaReadDword(uint32_t addr, uint32_t offset)
{
    return *((volatile uint32_t *)addr + offset);
}

/*****************************************************************************
 * 函数名称: emifaWriteWord
 * 函数说明: EMIFA读函数，每次读一个Dword（4Bytes）
 * 输入参数: 
 *           addr：基地址
 *           offset：偏移量
 *           value：写入数据
 * 输出参数: 无
 * 返 回 值: 无
 * 备注: emifaWriteWord(0x00000000,0x4,0xAA) -> 将0xAA写入到地址0x00000010（偏
 *       移4个Dword）的数据
*****************************************************************************/
void emifaWriteDword(uint32_t addr, uint32_t offset, uint32_t value)
{
    *((volatile uint32_t *)addr + offset) = value;
}

