/*
  * Copyright 2018 Trackener LTD & Krzysztof Hockuba
  *
  * Permission is hereby granted, free of charge, to any person obtaining
  * a copy of this software and associated documentation files (the "Software"),
  * to deal in the Software without restriction, including without limitation
  * the rights to use, copy, modify, merge, publish, distribute, sublicense,
  * and/or sell copies of the Software, and to permit persons to whom the
  * Software is furnished to do so, subject to the following conditions:
  *
  * The above copyright notice and this permission notice shall be included
  *          in all copies or substantial portions of the Software.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
 */

#ifndef __us_handler
#define __us_handler

#ifdef __cplusplus
extern "C" {
#endif

/** Include necessary CMSIS header **/
#include "stm32h7xx.h"
#include "core_cm7.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif

/**
 * @brief  Initializes DWT_Cycle_Count for DWT_Delay_us function
 * @return Error DWT counter
 *         1: DWT counter Error
 *         0: DWT counter works
 */
uint32_t DWT_Init(void);

/**
 * @brief Returns 1 if DWT counter is initialised.
 * @return DWT Counter status:
 *         0: not initialised
 *         1: initialised
 */
uint8_t DWT_IsInitialised(void);

#pragma GCC push_options
#pragma GCC optimize ("O0")

/**
 * @brief This function returns numbers of microseconds.
 * @return Number of microseconds from the moment DWT was initialised.
 */
__STATIC_INLINE uint32_t DWT_Get_us(void)
{
    if (DWT_IsInitialised() == 0) {
        DWT_Init();
        return 0;
    }
    uint32_t micros = DWT->CYCCNT / (SystemCoreClock / 1000000);
    return micros;
}

#define MICROS_ROLLOVER ((uint32_t) UINT32_MAX / (SystemCoreClock / 1000000))
/**
 * @brief This function returns delta between two us values
 * @param newUs uint32_t new value
 * @param oldUs uint32_t old value
 * @return delta between values
 */
__STATIC_INLINE uint32_t DWT_GetUsDelta(uint32_t newUs, uint32_t oldUs)
{
    uint32_t deltat = newUs - oldUs;
    if ((int32_t) deltat < 0) {
        deltat += MICROS_ROLLOVER;
    }
    return deltat;
}

/**
 * @brief  This function provides a delay (in microseconds)
 * @param  microseconds: delay in microseconds
 */
#define CYCCNT_ROLLOVER ((uint32_t) UINT32_MAX)
__STATIC_INLINE void DWT_Delay_us(volatile uint32_t microseconds)
{
    if (DWT_IsInitialised() == 0) {
        DWT_Init();
        return;
    }
    uint32_t us_start = DWT->CYCCNT;
    uint32_t us_end = us_start + microseconds * (SystemCoreClock / 1000000);

    if (us_end < us_start) {
        // Overflow, wait for CYCCNT to overflow
        while (DWT->CYCCNT > us_start);
    }
    /* Delay till end */
    while (DWT->CYCCNT < us_end);
}

#pragma GCC pop_options


#ifdef __cplusplus
}
#endif

#endif
