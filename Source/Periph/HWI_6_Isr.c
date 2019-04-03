
#include "interrupt.h"
#include "soc_C6748.h"
#include "mpu9250_iic.h"

extern void IICInterruptHandler(IICObj_t *insPtr);

#if 0
extern void gyro_data_ready_cb(void);
#endif

extern IICObj_t mpu9250IICInst;

void HWI_5_Isr(void)
{
     IICInterruptHandler(&mpu9250IICInst);
}

#if 0
void HWI_8_Isr(void)
{
	gyro_data_ready_cb();
}
#endif
