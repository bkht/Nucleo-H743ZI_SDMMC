#ifndef __DMC_MCU_H
#define __DMC_MCU_H

/* C++ detection */
#ifdef __cplusplus
extern "C"
{
#endif

#include "stm32h7xx_hal.h"

void GetMcuRevision(void);
uint16_t GetMcuDevID(void);
uint16_t GetMcuRevID(void);
uint8_t GetMcuRevIDChar(void);
uint32_t Get_RCC_CSR(void);
void Show_RCC_CSR(uint32_t RCC_CSR);
char* GetMCUFamily(void);
char* GetMCUType(void);

/* C++ detection */
#ifdef __cplusplus
}
#endif

#endif /* __DMC_MCU_H */
