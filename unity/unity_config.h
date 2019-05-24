/*
 * unity_config.h
 *
 *  Created on: Jan 18, 2019
 *      Author: khockuba
 */

#ifndef UNITY_UNITY_CONFIG_H_
#define UNITY_UNITY_CONFIG_H_

#include "usart.h"
#include "stm32h7xx_hal.h"

#define UNITY_INCLUDE_EXEC_TIME
#define UNITY_OUTPUT_COLOR

#define UNITY_EXEC_TIME_START() Unity.CurrentTestStartTime = HAL_GetTick();
#define UNITY_EXEC_TIME_STOP() Unity.CurrentTestStopTime = HAL_GetTick();

#define UNITY_OUTPUT_CHAR(a) huart3.Instance->TDR = a; while ((huart3.Instance->ISR & USART_ISR_TXE) == 0);
#define UNITY_PRINT_EOL()    UNITY_OUTPUT_CHAR('\r'); UNITY_OUTPUT_CHAR('\n')
#define UNITY_OUTPUT_FLUSH()

// This will guard mutex
#define UNIT_INIT_MUTEX() if (unitTestMutex == NULL) {\
                                osMutexDef(unitTestM); \
                                unitTestMutex = osMutexCreate(osMutex(unitTestM)); \
                        }
#define UNITY_BEGIN() UnityBegin(__FILE__)
#define UNITY_END() UnityEnd();

#endif /* UNITY_UNITY_CONFIG_H_ */
