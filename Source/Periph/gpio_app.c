/*
 * gpio.c
 *
 *  Created on: 2018��11��27��
 *      Author: hk
 */
#include "gpio_app.h"
#include "gpio.h"
#include "stdint.h"
#include "soc_C6748.h"               // DSP C6748 ����Ĵ���
#include "hw_types.h"
#include "hw_syscfg0_C6748.h"
#include "psc.h"
#include "canModule.h"
//#include "interrupt.h"
//#include "uart_pro.h"
//#include "sja1000.h"
//#include "emifa_app.h"

static void GPIOBankPinInterruputInit(void);
static void GPIOBankPinMuxSet(void);
static void GPIOBankPinInit(void);
static void GPIOBank6Pin0PinMuxSetup(void);
static void GPIOBank6Pin15PinMuxSetup(void);
static void GPIOBank6Pin14PinMuxSetup(void); //test int
static void GPIOBank6Pin10PinMuxSetup(void);
static void GPIOBank0Pin0PinMuxSetup(void);  //uart int
static void GPIOBank0Pin1PinMuxSetup(void);  //can int
static void GPIOBank2Pin5PinMuxSetup(void);


/****************************************************************************/
/*                                                                          */
/*              ���ܶ���                                                    */
/*                                                                          */
/****************************************************************************/
static void rst_delay(uint32_t n)
{
    uint32_t i;

    for (i = 0; i < n; i++);
}


/****************************************************************************/
/*                                                                          */
/*              Ӳ���ж�                                                    */
/*                                                                          */
/****************************************************************************/
static void GPIOBankPinInterruputInit(void)
{
    // 配置UART为上升沿中断
    //GPIOIntTypeSet(SOC_GPIO_0_REGS, 111, GPIO_INT_TYPE_RISEDGE);
	GPIOIntTypeSet(SOC_GPIO_0_REGS, 107, GPIO_INT_TYPE_FALLEDGE);
    GPIOIntTypeSet(SOC_GPIO_0_REGS, 1, GPIO_INT_TYPE_RISEDGE);
    GPIOIntTypeSet(SOC_GPIO_0_REGS, 2, GPIO_INT_TYPE_RISEDGE);
    /* Enable interrupts for Bank*/
    GPIOBankIntEnable(SOC_GPIO_0_REGS, 6);
    GPIOBankIntEnable(SOC_GPIO_0_REGS, 0);
}

/*
void GPIOEXTIsr(void)
{
    uint8_t chn,i;

    EMIFAWriteUart(SOC_EMIFA_CS2_ADDR, UART_INT_EN_REG, 0x0);  //�����ж�

    if(GPIOPinIntStatus(SOC_GPIO_0_REGS, 1) == GPIO_INT_PEND)   //uart
    {
        chn=EMIFAReadUart(SOC_EMIFA_CS2_ADDR, UART_INT_OFFSET_REG);
        for(i=0;i<6;i++)
        {
            if((chn&(0x01<<i))!=0)UartHandler(i);
            
        }
    }
    if (GPIOPinIntStatus(SOC_GPIO_0_REGS, 2) == GPIO_INT_PEND) //can
    {
        chn = EMIFAReadUart(SOC_EMIFA_CS2_ADDR, CAN_INT_OFFSET_REG);
        for (i = 0; i < 6; i++)
        {
            if((chn&(0x01<<i))!=0)CANHandle(i);
        }
    }

    GPIOPinIntClear(SOC_GPIO_0_REGS, 2); // ��� GPIO0[1]�ж�״̬
    GPIOPinIntClear(SOC_GPIO_0_REGS, 1); // ��� GPIO0[0]�ж�״̬
    EMIFAWriteUart(SOC_EMIFA_CS2_ADDR, UART_INT_EN_REG, 0x01);  //ʹ���ж�
}

void GPIOTESTIsr(void)
{
    if(GPIOPinIntStatus(SOC_GPIO_0_REGS, 111) == GPIO_INT_PEND)
    {
        //test interrupt
    }

    // ��� GPIO0[6] �ж�״̬
    GPIOPinIntClear(SOC_GPIO_0_REGS, 111);
}
*/

/****************************************************************************/
/*                                                                          */
/*              GPIO �ܽų�ʼ��                                             */
/*                                                                          */
/****************************************************************************/

static void PSCInit(void)
{
    // 使能GPIO电源
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_GPIO, PSC_POWERDOMAIN_ALWAYS_ON, PSC_MDCTL_NEXT_ENABLE);
}
static void GPIOBankPinMuxSet(void)
{
    GPIOBank6Pin0PinMuxSetup();  //run led
    GPIOBank6Pin15PinMuxSetup(); //fpga rst
    //GPIOBank6Pin14PinMuxSetup(); //test int
    GPIOBank6Pin10PinMuxSetup(); //MPU9250 int
    GPIOBank0Pin0PinMuxSetup();  //uart int
    GPIOBank0Pin1PinMuxSetup();  //can int
    //4G 模块复位 调试时为了restart后不用等4G上电，先屏蔽，等后面再加入
//    GPIOBank2Pin5PinMuxSetup();  //LTE Reset
}

static void GPIOBankPinInit(void)
{
    GPIODirModeSet(SOC_GPIO_0_REGS, 97, GPIO_DIR_OUTPUT);  // GPIO6[0]
    GPIODirModeSet(SOC_GPIO_0_REGS, 112, GPIO_DIR_OUTPUT); // GPIO6[15]
    //GPIODirModeSet(SOC_GPIO_0_REGS, 111, GPIO_DIR_INPUT);  // GPIO6[14]
    GPIODirModeSet(SOC_GPIO_0_REGS, 107, GPIO_DIR_INPUT);  // GPIO6[10]
    GPIODirModeSet(SOC_GPIO_0_REGS, 1, GPIO_DIR_INPUT);    // GPIO0[0]
    GPIODirModeSet(SOC_GPIO_0_REGS, 2, GPIO_DIR_INPUT);    // GPIO0[1]

    //4G 模块复位 调试时为了restart后不用等4G上电，先屏蔽，等后面再加入
    /*
    GPIODirModeSet(SOC_GPIO_0_REGS, 48, GPIO_DIR_OUTPUT);  // GPIO2[15]
    GPIOPinWrite(SOC_GPIO_0_REGS, 48, GPIO_PIN_LOW);
    rst_delay(10000);
    GPIOPinWrite(SOC_GPIO_0_REGS, 48, GPIO_PIN_HIGH);
    */
}

static void GPIOBank6Pin0PinMuxSetup(void)  
{
     unsigned int savePinmux = 0;

     /*
     ** Clearing the bit in context and retaining the other bit values
     ** in PINMUX19 register.
     */
     savePinmux = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(19)) &
                  ~(SYSCFG_PINMUX19_PINMUX19_27_24));

     /* Setting the pins corresponding to GP6[0] in PINMUX19 register.*/
     HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(19)) =
          (PINMUX19_GPIO6_0_ENABLE | savePinmux);

}

static void GPIOBank2Pin5PinMuxSetup(void)  
{
     unsigned int savePinmux = 0;

     /*
     ** Clearing the bit in context and retaining the other bit values
     ** in PINMUX19 register.
     */
     savePinmux = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(5)) &
                  ~(SYSCFG_PINMUX5_PINMUX5_3_0));

     /* Setting the pins corresponding to GP6[0] in PINMUX19 register.*/
     HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(5)) =
          (PINMUX19_GPIO2_15_ENABLE | savePinmux);

}


static void GPIOBank6Pin15PinMuxSetup(void)  
{
     unsigned int savePinmux = 0;

     /*
     ** Clearing the bit in context and retaining the other bit values
     ** in PINMUX13 register.
     */
     savePinmux = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(13)) &
                  ~(SYSCFG_PINMUX13_PINMUX13_3_0));

     /* Setting the pins corresponding to GP6[15] in PINMUX13 register.*/
     HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(13)) =
          (PINMUX13_GPIO6_15_ENABLE | savePinmux);

}

static void GPIOBank6Pin14PinMuxSetup(void)
{
    unsigned int savePinmux = 0;

    /*
     ** Clearing the bit in context and retaining the other bit values
     ** in PINMUX13 register.
     */
    savePinmux = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(13)) &
                  ~(SYSCFG_PINMUX13_PINMUX13_7_4));

    /* Setting the pins corresponding to GP6[14] in PINMUX13 register.*/
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(13)) =
        (PINMUX13_GPIO6_14_ENABLE | savePinmux);
}

static void GPIOBank6Pin10PinMuxSetup(void)
{
    unsigned int savePinmux = 0;

    /*
     ** Clearing the bit in context and retaining the other bit values
     ** in PINMUX13 register.
     */
    savePinmux = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(13)) &
                  ~(SYSCFG_PINMUX13_PINMUX13_23_20));

    /* Setting the pins corresponding to GP6[14] in PINMUX13 register.*/
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(13)) =
        (PINMUX13_GPIO6_10_ENABLE | savePinmux);
}

static void GPIOBank0Pin0PinMuxSetup(void)
{
    unsigned int savePinmux = 0;

    /*
     ** Clearing the bit in context and retaining the other bit values
     ** in PINMUX1 register.
     */
    savePinmux = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(1)) &
                  ~(SYSCFG_PINMUX1_PINMUX1_31_28));

    /* Setting the pins corresponding to GP0[0] in PINMUX1 register.*/
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(1)) =
        (PINMUX1_GPIO0_0_ENABLE | savePinmux);
}

static void GPIOBank0Pin1PinMuxSetup(void)
{
    unsigned int savePinmux = 0;

    /*
     ** Clearing the bit in context and retaining the other bit values
     ** in PINMUX13 register.
     */
    savePinmux = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(1)) &
                  ~(SYSCFG_PINMUX1_PINMUX1_27_24));

    /* Setting the pins corresponding to GP0[1] in PINMUX1 register.*/
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(1)) =
        (PINMUX1_GPIO0_1_ENABLE | savePinmux);
}


//接口==========================
void gpio_fpga_rst(void)
{
    GPIOPinWrite(SOC_GPIO_0_REGS, 112, GPIO_PIN_LOW);
    rst_delay(0xfffff);
    GPIOPinWrite(SOC_GPIO_0_REGS, 112, GPIO_PIN_HIGH);
    rst_delay(0xfffff);
}

void gpio_toggle_led(void)
{
    static uint8_t led_sta=0;

    if(led_sta==0)
        GPIOPinWrite(SOC_GPIO_0_REGS, 97, GPIO_PIN_LOW);
    else
        GPIOPinWrite(SOC_GPIO_0_REGS, 97, GPIO_PIN_HIGH);

    led_sta=~led_sta;
}

void gpio_init(void)
{
	PSCInit();
    GPIOBankPinMuxSet();
    GPIOBankPinInit();
    GPIOBankPinInterruputInit();
}
