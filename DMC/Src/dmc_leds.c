#include "dmc_leds.h"

// C program for implementation of ftoa()
#include<stdio.h>
#include<math.h>
#include <string.h>

struct DMC_LEDS DmcLeds[DMC_LEDS_NO_OF_LEDS];

volatile uint8_t RefreshCounter = 0;
volatile uint16_t BlinkCounter = 0;
volatile uint8_t BlinkState = FALSE;

void DmcLedTickTime(void)
{
	// Called every 1 mS
	RefreshCounter++;
	if (RefreshCounter >= 100)
	{
		RefreshCounter = 0;
		for (uint8_t i = 0; i < DMC_LEDS_NO_OF_LEDS; i++)
		{
			if (DmcLeds[i].State == LED_OFF)
			{
				DmcLedOff(i);
			}
			else
			{
				if (DmcLeds[i].Blink == LED_STEADY)
				{
					DmcLedOn(i);
				}
				else
				{
					DmcLedState(i, BlinkState);
				}
			}
		}
	}

	BlinkCounter++;
	if (BlinkCounter >= 500)
	{
		BlinkCounter = 0;
		BlinkState = !BlinkState;
	}
}

uint8_t DmcLedGetBlinkState(void)
{
	return BlinkState;	// Can be used for synchronous blinking on Alarm screen on the LCD
}

void DmcLedSetState(uint8_t led, uint8_t state)
{
	DmcLeds[led].State = state;
	DmcLeds[led].Blink = LED_STEADY;
}

void DmcLedSetBlink(uint8_t led, uint8_t blink)
{
	DmcLeds[led].State = LED_ON;
	DmcLeds[led].Blink = blink;
}

void DmcLedsInit(void)
{
	for (uint8_t i = 0; i < DMC_LEDS_NO_OF_LEDS; i++)
	{
		DmcLeds[i].State = LED_OFF;
		DmcLeds[i].Blink = LED_STEADY;
	}
}

void DmcLedState(uint8_t led, uint8_t state)
{
	switch (state)
	{
	case LED_OFF:
		DmcLedOff(led);
		break;
	case LED_ON:
		DmcLedOn(led);
		break;
	default:
		break;
	}
}

void DmcLedOn(uint8_t led)
{
	switch (led)
	{
	case LED_RUN:
		HAL_GPIO_WritePin(GPIOC, LED_RUN_Pin, LED_RUN_ON);
		break;
  case LED_RS1_OK:
    HAL_GPIO_WritePin(GPIOA, LED_RS1_OK_Pin, LED_RS1_OK_ON);
    break;
  case LED_RS1_ERR:
    HAL_GPIO_WritePin(GPIOA, LED_RS1_ERR_Pin, LED_RS1_ERR_ON);
    break;
  case LED_RS2_OK:
    HAL_GPIO_WritePin(GPIOA, LED_RS2_OK_Pin, LED_RS2_OK_ON);
    break;
  case LED_RS2_ERR:
    HAL_GPIO_WritePin(GPIOA, LED_RS2_ERR_Pin, LED_RS2_ERR_ON);
    break;
  case LED_CAN1_OK:
    HAL_GPIO_WritePin(GPIOC, LED_CAN1_OK_Pin, LED_CAN1_OK_ON);
    break;
  case LED_CAN2_OK:
    HAL_GPIO_WritePin(GPIOC, LED_CAN2_OK_Pin, LED_CAN2_OK_ON);
    break;
	default:
		break;
	}
}

void DmcLedOff(uint8_t led)
{
	switch (led)
	{
	case LED_RUN:
		HAL_GPIO_WritePin(GPIOC, LED_RUN_Pin, LED_RUN_OFF);
		break;
	case LED_RS1_OK:
		HAL_GPIO_WritePin(GPIOA, LED_RS1_OK_Pin, LED_RS1_OK_OFF);
		break;
  case LED_RS1_ERR:
    HAL_GPIO_WritePin(GPIOA, LED_RS1_ERR_Pin, LED_RS1_ERR_OFF);
    break;
  case LED_RS2_OK:
    HAL_GPIO_WritePin(GPIOA, LED_RS2_OK_Pin, LED_RS2_OK_OFF);
    break;
  case LED_RS2_ERR:
    HAL_GPIO_WritePin(GPIOA, LED_RS2_ERR_Pin, LED_RS2_ERR_OFF);
    break;
  case LED_CAN1_OK:
    HAL_GPIO_WritePin(GPIOC, LED_CAN1_OK_Pin, LED_CAN1_OK_OFF);
    break;
  case LED_CAN2_OK:
    HAL_GPIO_WritePin(GPIOC, LED_CAN2_OK_Pin, LED_CAN2_OK_OFF);
    break;
	default:
		break;
	}
}

void DmcLedToggle(uint8_t led)
{
	switch (led)
	{
	case LED_RUN:
		HAL_GPIO_TogglePin(GPIOC, LED_RUN_Pin);
		break;
	case LED_RS1_OK:
		HAL_GPIO_TogglePin(GPIOA, LED_RS1_OK_Pin);
		break;
  case LED_RS1_ERR:
    HAL_GPIO_TogglePin(GPIOA, LED_RS1_ERR_Pin);
    break;
  case LED_RS2_OK:
    HAL_GPIO_TogglePin(GPIOA, LED_RS2_OK_Pin);
    break;
  case LED_RS2_ERR:
    HAL_GPIO_TogglePin(GPIOA, LED_RS2_ERR_Pin);
    break;
  case LED_CAN1_OK:
    HAL_GPIO_TogglePin(GPIOC, LED_CAN1_OK_Pin);
    break;
  case LED_CAN2_OK:
    HAL_GPIO_TogglePin(GPIOC, LED_CAN2_OK_Pin);
    break;
	default:
		break;
	}
}

void DmcLedsOff(void)
{
  DmcLedOff(LED_RUN);
  DmcLedOff(LED_RS1_OK);
  DmcLedOff(LED_RS1_ERR);
  DmcLedOff(LED_RS2_OK);
  DmcLedOff(LED_RS2_ERR);
  DmcLedOff(LED_CAN1_OK);
  DmcLedOff(LED_CAN2_OK);
}

void DmcLedsOn(void)
{
  DmcLedOn(LED_RUN);
  DmcLedOn(LED_RS1_OK);
  DmcLedOn(LED_RS1_ERR);
  DmcLedOn(LED_RS2_OK);
  DmcLedOn(LED_RS2_ERR);
  DmcLedOn(LED_CAN1_OK);
  DmcLedOn(LED_CAN2_OK);
}

void DmcLedSet(uint8_t led)
{
  if (led & 1<<0)
  {
    DmcLedOn(LED_RUN);
  }
  else
  {
    DmcLedOff(LED_RUN);
  }
  if (led & 1<<1)
  {
    DmcLedOn(LED_RS1_OK);
  }
  else
  {
    DmcLedOff(LED_RS1_OK);
  }
  if (led & 1<<2)
  {
    DmcLedOn(LED_RS1_ERR);
  }
  else
  {
    DmcLedOff(LED_RS1_ERR);
  }
  if (led & 1<<3)
  {
    DmcLedOn(LED_RS2_OK);
  }
  else
  {
    DmcLedOff(LED_RS2_OK);
  }
  if (led & 1<<4)
  {
    DmcLedOn(LED_RS2_ERR);
  }
  else
  {
    DmcLedOff(LED_RS2_ERR);
  }
  if (led & 1<<5)
  {
    DmcLedOn(LED_CAN1_OK);
  }
  else
  {
    DmcLedOff(LED_CAN1_OK);
  }
  if (led & 1<<6)
  {
    DmcLedOn(LED_CAN2_OK);
  }
  else
  {
    DmcLedOff(LED_CAN2_OK);
  }
}

