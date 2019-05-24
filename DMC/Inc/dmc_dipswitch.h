#ifndef __DMC_DIPSWITCH_H
#define __DMC_DIPSWITCH_H

/* C++ detection */
#ifdef __cplusplus
extern "C"
{
#endif

#define DIPSWITCH_ON      TRUE
#define DIPSWITCH_OFF     FALSE

#define DIPSWITCH_1_VALUE (1<<3)
#define DIPSWITCH_2_VALUE (1<<2)
#define DIPSWITCH_3_VALUE (1<<1)
#define DIPSWITCH_4_VALUE (1<<0)

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"
#include "main.h"

uint8_t ReadDipSwitches(void);
uint8_t ReadDipSwitch(uint8_t switch_number);

/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif /* __DMC_DIPSWITCH_H */
