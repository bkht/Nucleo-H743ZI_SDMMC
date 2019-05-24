#ifndef __DMC_LEDS_H
#define __DMC_LEDS_H
#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"
#include "main.h"

#define LED_RUN_ON 		      GPIO_PIN_RESET
#define LED_RS1_OK_ON       GPIO_PIN_RESET
#define LED_RS1_ERR_ON      GPIO_PIN_RESET
#define LED_RS2_OK_ON       GPIO_PIN_RESET
#define LED_RS2_ERR_ON      GPIO_PIN_RESET
#define LED_CAN1_OK_ON      GPIO_PIN_RESET
#define LED_CAN2_OK_ON      GPIO_PIN_RESET

#if (LED_RUN_ON == GPIO_PIN_RESET)
#define LED_RUN_OFF 		GPIO_PIN_SET
#else
#define LED_RUN_OFF 		GPIO_PIN_RESET
#endif

#if (LED_RS1_OK_ON == GPIO_PIN_RESET)
#define LED_RS1_OK_OFF    GPIO_PIN_SET
#else
#define LED_RS1_OK_OFF    GPIO_PIN_RESET
#endif

#if (LED_RS1_ERR_ON == GPIO_PIN_RESET)
#define LED_RS1_ERR_OFF   GPIO_PIN_SET
#else
#define LED_RS1_ERR_OFF   GPIO_PIN_RESET
#endif

#if (LED_RS2_OK_ON == GPIO_PIN_RESET)
#define LED_RS2_OK_OFF    GPIO_PIN_SET
#else
#define LED_RS2_OK_OFF    GPIO_PIN_RESET
#endif

#if (LED_RS2_ERR_ON == GPIO_PIN_RESET)
#define LED_RS2_ERR_OFF   GPIO_PIN_SET
#else
#define LED_RS2_ERR_OFF   GPIO_PIN_RESET
#endif

#if (LED_CAN1_OK_ON == GPIO_PIN_RESET)
#define LED_CAN1_OK_OFF    GPIO_PIN_SET
#else
#define LED_CAN1_OK_OFF    GPIO_PIN_RESET
#endif

#if (LED_CAN2_OK_ON == GPIO_PIN_RESET)
#define LED_CAN2_OK_OFF    GPIO_PIN_SET
#else
#define LED_CAN2_OK_OFF    GPIO_PIN_RESET
#endif

#define DMC_LEDS_NO_OF_LEDS		7

//#define LED_OFF     0
//#define LED_ON      1
//
typedef enum DmcLedState
{
	LED_OFF,
	LED_ON
};

typedef enum DmcLedBlink
{
	LED_STEADY,
	LED_BLINK
};

typedef enum DmcLed
{
  LED_RUN,
	LED_RS1_OK,
	LED_RS1_ERR,
  LED_RS2_OK,
  LED_RS2_ERR,
  LED_CAN1_OK,
  LED_CAN2_OK
};

struct DMC_LEDS
{
	uint8_t State;
	uint8_t Blink;
};

void DmcLedTickTime(void);
uint8_t DmcLedGetBlinkState(void);
void DmcLedSetState(uint8_t led, uint8_t state);
void DmcLedSetBlink(uint8_t led, uint8_t blink);
void DmcLedsInit(void);
void DmcLedState(uint8_t led, uint8_t state);
void DmcLedOn(uint8_t led);
void DmcLedOff(uint8_t led);
void DmcLedToggle(uint8_t led);
void DmcLedsOff(void);
void DmcLedsOn(void);
void DmcLedSet(uint8_t led);

#ifdef __cplusplus
}
#endif
#endif /* __DMC_LEDS_H */
