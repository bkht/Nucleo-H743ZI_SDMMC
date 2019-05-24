// There are several options for storing settings/data on microcontrollers non-volatile memory:
// Use flash (emulated as EEPROM for ease of use) (slow and wear out MCU flash, not recommended):
// up to 40 millisecond halt for erase, endurance of 'only' 10 kcycles.
// Using the backup registers or external EEPROM or FRAM.
// Depending on the frequency you want to update the variable with you can decide if the durability of
// EEPROM is sufficient, else you can use FRAM which will last forever. (10 trillion writes)
// RTC backup registers
// The RTC backup registers, + a battery (32 backup data registers (128 bytes)) according to the
// Backup SRAM (BKP SRAM)
// The backup domain includes 4 Kbytes of backup SRAM addressed in 32-bit, 16-bit or 8-bit mode.
// Reference Manual Pages 117, 122 And 79
// https://www.st.com/content/ccc/resource/technical/document/reference_manual/group0/96/8b/0d/ec/16/22/43/71/DM00224583/files/DM00224583.pdf/jcr:content/translations/en.DM00224583.pdf
//

#include <string.h>
#include "dmc_errorhandler.h"
#include "dmc_print.h"

struct DMC_ERRORS DMC_Error;
char DMC_ERROR_File[DMC_ERROR_MAX_FILE_LENGTH];

void DMC_InitErrorHandler(void)
{
	DMC_Error.Error = 0;
	DMC_Error.Line = 0;
	DMC_Error.File[0] = '\0';
//	PWR->CR1 = DBP;				// Access to RTC and RTC Backup registers and backup SRAM enabled
//	SET_BIT(PWR->CR1, PWR_CR1_DBP);
//	PWR->CSR1 = BRE;			// Backup regulator enabled
//	SET_BIT(PWR->CSR1, PWR_CSR1_BRE);
//	RCC->APB1ENR |= BKPSRAMEN;	// Enable the backup SRAM clock
//	SET_BIT(RCC->APB1ENR, BKPSRAMEN);
//#define SET_BIT(REG, BIT)     ((REG) |= (BIT))
//#define CLEAR_BIT(REG, BIT)   ((REG) &= ~(BIT))
//#define READ_BIT(REG, BIT)    ((REG) & (BIT))
//#define CLEAR_REG(REG)        ((REG) = (0x0))
//#define WRITE_REG(REG, VAL)   ((REG) = (VAL))
//#define READ_REG(REG)         ((REG))
//#define MODIFY_REG(REG, CLEARMASK, SETMASK)  WRITE_REG((REG), (((READ_REG(REG)) & (~(CLEARMASK))) | (SETMASK)))
//#define POSITION_VAL(VAL)     (__CLZ(__RBIT(VAL)))


	/* Enable PWR peripheral clock */
	__HAL_RCC_PWR_CLK_ENABLE();

	/* Allow access to BKP Domain */
	HAL_PWR_EnableBkUpAccess();

	__BKPSRAM_CLK_ENABLE();
}

void DMC_WriteUint32BKPSRAM(uint16_t address, uint32_t value)
{
	*(__IO uint32_t *) (D3_BKPSRAM_BASE + address) = value;

	// Wait until the Backup SRAM low power Regulator is ready
//	while(!READ_BIT(PWR->CSR1, BRR));
//	while(!PWR->CSR1 & BRR);
}

uint32_t DMC_ReadUint32BKPSRAM(uint16_t address)
{
	return *(__IO uint32_t *) (D3_BKPSRAM_BASE + address);
}

void DMC_WriteStringBKPSRAM(uint16_t address, char *string)
{
	uint16_t length = strlen(string);
	for (uint16_t i = 0; i < length; i++)
	{
		*(__IO char *) (D3_BKPSRAM_BASE + address + i) = string[i];
	}

	// Wait until the Backup SRAM low power Regulator is ready
//	while(!READ_BIT(PWR->CSR1, BRR));
//	while(!PWR->CSR1 & BRR);
}

char *DMC_ReadStringBKPSRAM(uint16_t address)
{
	for (uint16_t i = 0; i < DMC_ERROR_MAX_FILE_LENGTH; i++)
	{
		DMC_ERROR_File[i] = *(__IO char *) (D3_BKPSRAM_BASE + address + i);
		if (DMC_ERROR_File[i] == 0)
		{
			break;
		}
	}

	return DMC_ERROR_File;
}

void DMC_TestBKPSRAM(void)
{
	uint16_t size = 4096;
	dmc_puts("DMC_WriteBKPSRAM\n");
	// Write to Backup SRAM with 32-Bit Data
	for (uint16_t i = 0; i < size; i += 4)
	{
		DMC_WriteUint32BKPSRAM(i, (uint32_t) i);
	}

	// Check the written Data
	uint8_t errorIndex = 0;
	for (uint16_t i = 0; i < size; i += 4)
	{
		if (DMC_ReadUint32BKPSRAM(i) != (uint32_t) i)
		{
			errorIndex++;
			dmc_putint(i);
			dmc_puts("   ");
			dmc_putintcr(*(__IO uint32_t *) (D3_BKPSRAM_BASE + i));
		}
	}
	dmc_putintcr(errorIndex);

	DMC_WriteStringBKPSRAM(0x0008, "Hello world!");
	dmc_putscr(DMC_ReadStringBKPSRAM(0x0008));

	// Wait until the Backup SRAM low power Regulator is ready
	while(!PWR->CSR1 & BRR);
	dmc_puts("Done!\n");
}

void DMC_ClearError(void)
{
	DMC_InitErrorHandler();
	DMC_WriteUint32BKPSRAM(DMC_BKP_LOCATION_ERROR, 0);
	DMC_WriteUint32BKPSRAM(DMC_BKP_LOCATION_LINE, 0);
	DMC_WriteUint32BKPSRAM(DMC_BKP_LOCATION_FILE, 0);
}

uint32_t DMC_GetError(void)
{
	DMC_Error.Error = DMC_ReadUint32BKPSRAM(DMC_BKP_LOCATION_ERROR);
	return DMC_Error.Error;
}

void DMC_PrintError(void)
{
	if (DMC_Error.Error == 0)
	{
		dmc_puts("\n----------\n");
		dmc_puts("No errors.\n");
		dmc_puts("----------\n");
		return;
	}
	DMC_Error.Error = DMC_ReadUint32BKPSRAM(DMC_BKP_LOCATION_ERROR);
	DMC_Error.Line = DMC_ReadUint32BKPSRAM(DMC_BKP_LOCATION_LINE);
	strncpy(DMC_Error.File, DMC_ReadStringBKPSRAM(DMC_BKP_LOCATION_FILE), DMC_ERROR_MAX_FILE_LENGTH);
	dmc_puts("\n----------\n");
	dmc_puts("ERROR (read)\n");
	dmc_puts("File: ");
	dmc_putscr(DMC_Error.File);
	dmc_puts("Line: ");
	dmc_putintcr(DMC_Error.Line);
	dmc_puts("Error: ");
	dmc_putintcr(DMC_Error.Error);
	dmc_puts("----------\n");
}

void DMC_ErrorHandler(char *file, int line)
{
	uint32_t error = DMC_ERROR_ADC_UNDEFINED;
	if(strstr(file, "main.c") != NULL)
	{
		error = DMC_ERROR_CLOCK_CONFIG;
	}
	if(strstr(file, "adc.c") != NULL)
	{
		error = DMC_ERROR_ADC_CONFIG;
	}
	if(strstr(file, "dac.c") != NULL)
	{
		error = DMC_ERROR_DAC_CONFIG;
	}
	if(strstr(file, "dma.c") != NULL)
	{
		error = DMC_ERROR_DMA_CONFIG;
	}
	if(strstr(file, "i2c.c") != NULL)
	{
		error = DMC_ERROR_I2C_CONFIG;
	}
	if(strstr(file, "rtc.c") != NULL)
	{
		error = DMC_ERROR_RTC_CONFIG;
	}
	if(strstr(file, "tim.c") != NULL)
	{
		error = DMC_ERROR_TIM_CONFIG;
	}
	if(strstr(file, "sdmmc.c") != NULL)
	{
		error = DMC_ERROR_SDMMC_CONFIG;
	}
	if(strstr(file, "spi.c") != NULL)
	{
		error = DMC_ERROR_SPI_CONFIG;
	}
	if(strstr(file, "quadspi.c") != NULL)
	{
		error = DMC_ERROR_QUADSPI_CONFIG;
	}
	if(strstr(file, "usart.c") != NULL)
	{
		error = DMC_ERROR_USART_CONFIG;
	}
	if(strstr(file, "usbd_conf.c") != NULL)
	{
		error = DMC_ERROR_USBD_CONFIG;
	}
	if(strstr(file, "usbh_conf.c") != NULL)
	{
		error = DMC_ERROR_USBH_CONFIG;
	}
	if(strstr(file, "dmc_keyadc.c") != NULL)
	{
		error = DMC_ERROR_DMC_KEYADC_CONFIG;
	}
	if(strstr(file, "dmc_keyadc.c") != NULL)
	{
		error = DMC_ERROR_DMC_KEYADC_CONFIG;
	}
	if(strstr(file, "dmc_pwm.c") != NULL)
	{
		error = DMC_ERROR_DMC_PWM_CONFIG;
	}
	if(strstr(file, "dmc_temperature.c") != NULL)
	{
		error = DMC_ERROR_DMC_TEMPERATURE_CONFIG;
	}
	if(strstr(file, "can.c") != NULL)
	{
		error = DMC_ERROR_CAN_CONFIG;
	}
	if(strstr(file, "main.c") != NULL)
	{
		if ((line > 1400) && (line < 1450))
		{
			error = DMC_ERROR_HAL_MMC_ConfigWideBusOperation;
		}
		if ((line > 4380) && (line < 4456))
		{
			error = DMC_ERROR_SYSTEMCLOCK;
		}
	}

	dmc_puts("\n----------\n");
	dmc_puts("ERROR\n");
	dmc_puts("File: ");
	dmc_putscr(file);
	dmc_puts("Line: ");
	dmc_putintcr(line);
//	dmc_puts("Error: ");
//	dmc_putintcr(error);
	dmc_puts("----------\n");

//	strncpy(DMC_Error.File, file, DMC_ERROR_MAX_FILE_LENGTH);
//	DMC_Error.Line = line;
//	DMC_Error.Error = error;
//	DMC_WriteUint32BKPSRAM(DMC_BKP_LOCATION_ERROR, error);
//	DMC_WriteUint32BKPSRAM(DMC_BKP_LOCATION_LINE, line);
//	DMC_WriteStringBKPSRAM(DMC_BKP_LOCATION_FILE, file);

//	DMC_PrintError();
//    WdtConfigure(5.0);
    while(1)
    {
      dmc_putc('.');
      HAL_Delay(1000);
    }
}

void DMC_ErrorHandler2(char *file, int line, int error)
{
  dmc_puts("\n");
  dmc_puts("--------------------\n");
  dmc_puts("ERROR\n");
  dmc_puts("File: ");
  dmc_putscr(file);
  dmc_puts("Line: ");
  dmc_putintcr(line);
  dmc_puts("Error: ");
  dmc_putint(error);
  dmc_puts(" = ");
  switch(error)
  {
  case HAL_OK:
    dmc_puts("HAL_OK\n");
    break;
  case HAL_ERROR:
    dmc_puts("HAL_ERROR\n");
    break;
  case HAL_BUSY:
    dmc_puts("HAL_BUSY\n");
    break;
  case HAL_TIMEOUT:
    dmc_puts("HAL_TIMEOUT\n");
    break;
  default:
    dmc_puts("HAL_UNDEFINED\n");
  }
  dmc_puts("--------------------\n");

//  strncpy(DMC_Error.File, file, DMC_ERROR_MAX_FILE_LENGTH);
//  DMC_Error.Line = line;
//  DMC_Error.Error = error;
//  DMC_WriteUint32BKPSRAM(DMC_BKP_LOCATION_ERROR, error);
//  DMC_WriteUint32BKPSRAM(DMC_BKP_LOCATION_LINE, line);
//  DMC_WriteStringBKPSRAM(DMC_BKP_LOCATION_FILE, file);

//  DMC_PrintError();
//    WdtConfigure(5.0);
    while(1)
    {
      dmc_putc('.');
      HAL_Delay(1000);
    }
}
