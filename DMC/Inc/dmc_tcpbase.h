#ifndef __DMC_TCPBASE_H
#define __DMC_TCPBASE_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"
#include "main.h"

#include "dmc_fifo.h"
#include "dmc_net.h"

void net_poll(void);

#ifdef __cplusplus
}
#endif
#endif /* __DMC_TCPBASE_H */
