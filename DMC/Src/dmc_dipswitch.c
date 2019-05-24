#include "dmc_dipswitch.h"

uint8_t ReadDipSwitches(void)
{
	uint8_t SwitchValues = 0;
	// Switches connect to GND, so 'ON' means we will read a low level
	if (HAL_GPIO_ReadPin(GPIOE, S1_1_Pin) == GPIO_PIN_RESET)
	{
		SwitchValues |= DIPSWITCH_1_VALUE;
	}
	if (HAL_GPIO_ReadPin(GPIOE, S1_2_Pin) == GPIO_PIN_RESET)
	{
		SwitchValues |= DIPSWITCH_2_VALUE;
	}
	if (HAL_GPIO_ReadPin(GPIOE, S1_3_Pin) == GPIO_PIN_RESET)
	{
		SwitchValues |= DIPSWITCH_3_VALUE;
	}
	if (HAL_GPIO_ReadPin(GPIOE, S1_4_Pin) == GPIO_PIN_RESET)
	{
		SwitchValues |= DIPSWITCH_4_VALUE;
	}
	return SwitchValues;
}

uint8_t ReadDipSwitch(uint8_t switch_number)
{
	uint8_t status = 0;
	switch (switch_number)
	{
	case 1:
		status = !HAL_GPIO_ReadPin(GPIOE, S1_1_Pin);
		break;
	case 2:
		status = !HAL_GPIO_ReadPin(GPIOE, S1_2_Pin);
		break;
	case 3:
		status = !HAL_GPIO_ReadPin(GPIOE, S1_3_Pin);
		break;
	case 4:
		status = !HAL_GPIO_ReadPin(GPIOE, S1_4_Pin);
		break;
	default:
		break;
	}

	return status;
}
