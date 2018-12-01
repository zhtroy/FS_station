/****************************************************************************/
/*                                                                          */
/*              ���� DSP6748 ��������غ���?                                 */
/*                                                                          */
/*              2014��07��12��                                              */
/*                                                                          */
/****************************************************************************/
#include "stdint.h"
#include "hw_types.h"

#include "TL6748.h"
#include "soc_C6748.h"
#include "hw_syscfg0_C6748.h"

#include "uart.h"
#include "psc.h"

/****************************************************************************/
/*                                                                          */
/*            将库函数重定向到这里的代码，使用串口1    */
/*                                                                          */
/****************************************************************************/
#define UART_STDIO_INSTANCE             (1)
#define UART_CONSOLE_BASE               (SOC_UART_1_REGS)

/****************************************************************************/
/*                                                                          */
/*              ��������                                                    */
/*                                                                          */
/****************************************************************************/
static void UARTStdioInitExpClk(unsigned int baudRate,
                                unsigned int rxTrigLevel);
void UARTConsolePutc(uint8_t data);
uint8_t UARTConsoleGetc(void);
void UARTConsoleInit(void);

/****************************************************************************/
/*                                                                          */
/*              ���ڿ���̨����                                              */
/*                                                                          */
/****************************************************************************/
static void UARTStdioInitExpClk(unsigned int baudRate, unsigned int rxTrigLevel)
{
     // ʹ�ܽ��� / ����
     UARTEnable(UART_CONSOLE_BASE);

     // ���ڲ�������
     // 8λ����λ 1λֹͣλ ��У��
     UARTConfigSetExpClk(UART_CONSOLE_BASE, 
                         SOC_UART_1_MODULE_FREQ,
                         baudRate, 
                         UART_WORDL_8BITS,
                         UART_OVER_SAMP_RATE_16);


     // ʹ�ܽ��� / ���� FIFO
     UARTFIFOEnable(UART_CONSOLE_BASE);

     // ���ý��� FIFO ����
     UARTFIFOLevelSet(UART_CONSOLE_BASE, rxTrigLevel);

}

/****************************************************************************/
/*                                                                          */
/*              ���ڿ���̨��ʼ��                                            */
/*                                                                          */
/****************************************************************************/
void UARTConsoleInit(void)
{
     #if (0 == UART_STDIO_INSTANCE)
     {
          PSCModuleControl(SOC_PSC_0_REGS,9, 0, PSC_MDCTL_NEXT_ENABLE);
          UARTPinMuxSetup(0, FALSE);
     }
     
     #elif (1 == UART_STDIO_INSTANCE)
     {
          PSCModuleControl(SOC_PSC_1_REGS,12, 0, PSC_MDCTL_NEXT_ENABLE);
          UARTPinMuxSetup(1, FALSE);
     }

     #else 
     {
          PSCModuleControl(SOC_PSC_1_REGS,13, 0, PSC_MDCTL_NEXT_ENABLE);
          UARTPinMuxSetup(2, FALSE);
     }
     #endif
     
     UARTStdioInitExpClk(BAUD_115200, UART_RX_TRIG_LEVEL_1);
}

/****************************************************************************/
/*                                                                          */
/*              ���һ���ַ�?                                                */
/*                                                                          */
/****************************************************************************/
void UARTConsolePutc(uint8_t data)
{
     UARTCharPut(UART_CONSOLE_BASE, data);
}

/****************************************************************************/
/*                                                                          */
/*              ȡ��һ���ַ�                                                */
/*                                                                          */
/****************************************************************************/
uint8_t UARTConsoleGetc(void)
{
    return ((uint8_t)UARTCharGet(UART_CONSOLE_BASE));
}
