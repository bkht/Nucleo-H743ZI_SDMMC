#ifndef __DMC_ERRORHANDLER_H
#define __DMC_ERRORHANDLER_H
#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"
#include "main.h"

#define DMC_ERROR_MAX_FILE_LENGTH	256

// RM, Page	150
#define BKPSRAMEN 					1<18
#define DBP							1<8
#define BRE							1<9
#define BRR							1<3

#define DMC_BKP_LOCATION_ERROR		0x0000
#define DMC_BKP_LOCATION_LINE		0x0004
#define DMC_BKP_LOCATION_FILE		0x0008

enum DMC_Errors
{
	DMC_ERROR_NONE = 0,
	DMC_ERROR_ADC_UNDEFINED,
	DMC_ERROR_CLOCK_CONFIG,
	DMC_ERROR_ADC_CONFIG,
	DMC_ERROR_DAC_CONFIG,
	DMC_ERROR_DMA_CONFIG,
	DMC_ERROR_I2C_CONFIG,
	DMC_ERROR_RTC_CONFIG,
	DMC_ERROR_TIM_CONFIG,
	DMC_ERROR_CAN_CONFIG,
	DMC_ERROR_SDMMC_CONFIG,
	DMC_ERROR_QUADSPI_CONFIG,
	DMC_ERROR_SPI_CONFIG,
	DMC_ERROR_USART_CONFIG,
	DMC_ERROR_USBD_CONFIG,
	DMC_ERROR_USBH_CONFIG,
	DMC_ERROR_DMC_KEYADC_CONFIG,
	DMC_ERROR_DMC_PWM_CONFIG,
	DMC_ERROR_DMC_TEMPERATURE_CONFIG,
	DMC_ERROR_HAL_MMC_ConfigWideBusOperation,
	DMC_ERROR_GPIO_CONFIG,
	DMC_ERROR_SYSTEMCLOCK
};

struct DMC_ERRORS
{
	char File[DMC_ERROR_MAX_FILE_LENGTH];
	int Line;
	uint32_t Error;
};


void DMC_InitErrorHandler(void);
void DMC_WriteUint32BKPSRAM(uint16_t address, uint32_t value);
uint32_t DMC_ReadUint32BKPSRAM(uint16_t address);
void DMC_WriteStringBKPSRAM(uint16_t address, char *string);
char *DMC_ReadStringBKPSRAM(uint16_t address);

void DMC_TestBKPSRAM(void);
void DMC_PrintError(void);

void DMC_ClearError(void);
uint32_t DMC_GetError(void);
void DMC_PrintError(void);
void DMC_ErrorHandler(char *file, int line);
void DMC_ErrorHandler2(char *file, int line, int error);

#ifdef __cplusplus
}
#endif
#endif /* __DMC_ERRORHANDLER_H */
