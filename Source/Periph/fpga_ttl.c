#include "emifa/emifa_app.h"
#include "fpga_ttl.h"
#include "soc_C6748.h"

#define FPGA_RELAY_IN  (SOC_EMIFA_CS2_ADDR + (0x0B<<1))
#define FPGA_TTL_OUT (SOC_EMIFA_CS2_ADDR + (0x12<<1))
#define FPGA_TTL_DIR (SOC_EMIFA_CS2_ADDR + (0x14<<1))
#define FPGA_TTL_EN	 (SOC_EMIFA_CS2_ADDR + (0x15<<1))
#define FPGA_RELAY_EN (SOC_EMIFA_CS2_ADDR + (0x0D<<1))

void TTLInit()
{
	/*
	 * 1.关闭控制器使能;
	 * 2.设置默认输出，默认全部拉高;
	 * 3.配置TTL[7:0]和TTL[15:8]的方向
	 * 	 DIR[0]=1: TTL[7:0]为输出
	 * 	 DIR[1]=0: TTL[15:8]为输入
	 * 4.使能控制器
	 */
	EMIFAWriteWord(FPGA_TTL_EN, 0, 0x01);
	TTLWrite(0xff);
	EMIFAWriteWord(FPGA_TTL_DIR, 0, 0x01);
	EMIFAWriteWord(FPGA_TTL_EN, 0, 0x00);
	EMIFAWriteWord(FPGA_RELAY_EN, 0, 0x00);
}

void TTLWrite(uint8_t value)
{
	EMIFAWriteWord(FPGA_TTL_OUT, 0, value);
}

uint8_t TTLRead()
{
	return EMIFAReadWord(FPGA_RELAY_IN, 0);
}

void TTLWriteBit(uint8_t bitNum, uint8_t value)
{
	uint8_t regv;
	regv = EMIFAReadWord(FPGA_TTL_OUT, 0);
	if(value == 0)
		EMIFAWriteWord(FPGA_TTL_OUT, 0, regv & (~(0x01 << bitNum)));
	else
		EMIFAWriteWord(FPGA_TTL_OUT, 0, regv | (0x01 << bitNum));
}

uint8_t TTLReadBit(uint8_t bitNum)
{
	uint8_t regv;
	regv = EMIFAReadWord(FPGA_RELAY_IN,0);

	if((regv & (0x01 << bitNum)) == 0)
		return 0;
	else
		return 1;
}
