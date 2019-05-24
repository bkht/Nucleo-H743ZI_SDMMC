/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include <dmc_fat.h>
#include <dmc_print.h>
#include "main.h"
#include "dma.h"
#include "fatfs.h"
#include "lwip.h"
#include "sdmmc.h"
#include "spi.h"
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint32_t msTick = 0;
uint32_t msTickPrevious = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void HAL_IncTicks(void);
void HAL_StartTicks(void);
uint8_t HAL_GetTicks(uint32_t ms);
void MPU_Config(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
  

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  MPU_Config();

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_SPI4_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  dmc_puts("\n");
  dmc_puts("--------------------------------------------------------------------------------\n");
  dmc_puts("MCU type            : ");
  dmc_puts(TERMINAL_CYAN);
  dmc_puts(GetMCUType());
  dmc_puts(TERMINAL_DEFAULT);
  if (strcmp(GetMCUType(), "STM32H743xx") == 0)
  {
    dmc_puts(TERMINAL_LIGHT_GREEN);
    printf(" OK\n");
    dmc_puts(TERMINAL_DEFAULT);
  }
  else
  {
    dmc_puts(TERMINAL_LIGHT_RED);
    printf(" FAIL\n");
    dmc_puts(TERMINAL_DEFAULT);
  }

  GetMcuRevision();



//  run_sdmmc_test();

  dmc_puts("MX_SDMMC1_SD_Init\n");
  MX_SDMMC1_SD_Init();
  dmc_puts("MX_FATFS_Init\n");
  MX_FATFS_Init();

//  MX_LWIP_Init();
  /* USER CODE BEGIN 2 */

  dmc_put_sethuart(&huart3);

  dmc_puts(TERMINAL_DEFAULT);

  HAL_StartTicks();

  dmc_puts("SDPath: ");
  dmc_puts(SDPath);
  dmc_putcr();

  FRESULT res;
  FATFS MMCFatFs;
  FIL myFile;
  FATFS fs;
  DSTATUS status;
  BYTE work[1024]; /* Work area (larger is better for processing time) */


  /* Disk initialization */
  dmc_puts("Initialize disk: ");
  status = disk_initialize(0);
  if (status != RES_OK)
  {
    ShowDiskStatus(status);
  }
  else
  {
    dmc_puts("OK\n");
  }

  /* Disk status */
  dmc_puts("Disk status: ");
  status = disk_status(0);
  if (status != RES_OK)
  {
    ShowDiskStatus(status);
  }
  else
  {
    dmc_puts("OK\n");
  }





//  uint8_t pWData[512];
//  uint8_t pRData[512];
//  uint16_t BlockSize = 512;
//  uint32_t BlockNbr = 2;
//  uint32_t ReadAddr = 0;
//  uint32_t WriteAddr = 0;
//  uint32_t NumOfBlocks = 1;
//  uint32_t Timeout = 1000;
//  uint8_t SD_status = 0;
//
//  for (uint32_t j = 0; j < BlockNbr; j += 1)
//  {
//    WriteAddr = j;
//    ReadAddr = j;
//
//
//    for (uint16_t i = 0; i < BlockSize; i++)
//    {
//      pRData[i] = (i & 0xff);
//    }
//
//    //    disk_write(
//    //      0,    /* Physical drive nmuber to identify the drive */
//    //      pWData, /* Data to be written */
//    //      WriteAddr,   /* Sector address in LBA */
//    //      NumOfBlocks          /* Number of sectors to write */
//    //    );
//
//    //    dmc_puts("BSP_SD_WriteBlocks: ");
//    //    SD_status = BSP_SD_WriteBlocks((uint32_t *)pWData, WriteAddr, NumOfBlocks, Timeout);
//    //    dmc_puts("WR SD_status: ");
//    //    dmc_putint(SD_status);
//    //    dmc_putc(' ');
//
//    disk_read(
//        0,    /* Physical drive number to identify the drive */
//        pRData,   /* Data buffer to store read data */
//        ReadAddr,         /* Sector address in LBA */
//        NumOfBlocks    /* Number of sectors to read */
//    );
//
//    //    dmc_puts("BSP_SD_ReadBlocks: ");
//    //    SD_status = BSP_SD_ReadBlocks((uint32_t *)pRData, ReadAddr, NumOfBlocks, Timeout);
//    //    dmc_puts("RD SD_status: ");
//    //    dmc_putint(SD_status);
//    //    dmc_putc(' ');
//
//    //    uint8_t match = 1;
//    //    if (memcmp(pWData, pRData, BlockSize))
//    //    {
//    //      match = 0;
//    //    }
//    //    dmc_puts("match: ");
//    //    dmc_putint(match);
//    //    dmc_putc(' ');
//
////    for (uint16_t k = 0; k < BlockSize; k += 16)
////    {
////      dmc_puthex8(k + j * BlockSize);
////      dmc_puts("  ");
////      for (uint16_t i = 0; i < 16; i++)
////      {
////        dmc_puthex2(pRData[i + k]);
////        dmc_putc(' ');
////      }
////      dmc_putc(' ');
////      for (uint16_t i = 0; i < 16; i++)
////      {
////        if ((pRData[i + k] >= ' ') && (pRData[i + k] <= 127))
////        {
////          dmc_putc(pRData[i + k]);
////        }
////        else
////        {
////          dmc_putc('.');
////        }
////      }
////      dmc_putcr();
////    }
//  }

  //  /* Format Disk */
  //  /* Create FAT volume */
  //  // The f_mkfs function creates an FAT/exFAT volume on the logical drive.
  //  dmc_puts("Format disk\n");
  //  printf(TERMINAL_GREEN);
  //  res = f_mkfs(SDPath, FM_FAT, 0, work, sizeof(work));
  ////  res = f_mkfs(SDPath, FM_FAT32, 0, work, sizeof(work));
  //  printf(TERMINAL_DEFAULT);
  //  if (res != FR_OK)
  //  {
  //    ShowFatFsError(res);
  //  }
  //  else
  //  {
  //    dmc_puts("OK\n");
  //  }
  //



  /* Mount */
  dmc_puts("Mount disk: ");
  res = f_mount(&fs, SDPath, 1);
  if (res != FR_OK)
  {
    ShowFatFsError(res);
  }
  else
  {
    dmc_puts("OK\n");
  }

  /* Get volume label of the default drive */
  //  printf("Get volume label\n");
  char label[12];
  res = f_getlabel(SDPath, label, 0);
  if (res != FR_OK)
  {
    ShowFatFsError(res);
  }
  printf(TERMINAL_LIGHT_GREEN);
  printf("Volume label: %s\n", label);
  printf(TERMINAL_DEFAULT);

  /* Get volume information and free clusters of drive 1 */
  //  printf("Disk space:\n");
  FATFS *fs2;
  DWORD fre_clust, fre_sect, tot_sect;
  res = f_getfree(SDPath, &fre_clust, &fs2);
  if (res != FR_OK)
  {
    ShowFatFsError(res);
  }
  /* Get total sectors and free sectors */
  tot_sect = (fs2->n_fatent - 2) * fs2->csize;
  fre_sect = fre_clust * fs2->csize;
  /* Print the free space (assuming 512 bytes/sector) */
  printf(TERMINAL_LIGHT_GREEN);
  printf("%10lu kB total disk space\n", tot_sect / 2);  //   15386720 kB total disk space
  printf("%10lu kB available\n", fre_sect / 2);     //   15386688 kB available
  printf(TERMINAL_DEFAULT);

  scan_files(SDPath, 1);


  char *bufptr;
  FIL fil; /* File object */
  char filename[64];
  char pathname[64];
  char rstring[1024];

  /* Read file */
  strcpy(filename, "Info.txt");
  printf("Read file %s\n", filename);
  res = f_open(&fil, filename, FA_READ);
  if (res != FR_OK)
  {
    ShowFatFsError(res);
  }
  printf(TERMINAL_LIGHT_YELLOW);
  while (!f_eof(&fil))
  {
//    while (f_tell(&fil)  <  f_size(&fil)) {
    f_gets(rstring, 64, &fil);
    printf("%s", rstring);
  }
  printf(TERMINAL_DEFAULT);
  /* Close file */
    printf("Close file\n");
  res = f_close(&fil);
  if (res != FR_OK)
  {
    ShowFatFsError(res);
  }

  uint8_t CreateFiles = FALSE;
  CreateFiles = TRUE;
  if (CreateFiles)
  {
    /* Create file */
    // FA_CREATE_NEW    - FAILED: Access denied due to prohibited access
    // FA_CREATE_ALWAYS - Recreates the file if exist (overwrite)
    strcpy(filename, "hello.txt");
    printf("Create file %s\n", filename);
    res = f_open(&fil, filename, FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK)
    {
      ShowFatFsError(res);
    }
    /* Write message */
    UINT wbytes; /* File write counts, Bytes written */
    char wtext[128];
    snprintf(wtext, 128, "Hello, World!\nTest.\n");
    int length = strlen(wtext);
    printf("f_write...\n");
    res = f_write(&fil, wtext, length, &wbytes);
    if (res != FR_OK)
    {
      ShowFatFsError(res);
    }
    printf("done\n");
    /* Close file */
    //  printf("Close file\n");
    printf("f_close...\n");
    res = f_close(&fil);
    if (res != FR_OK)
    {
      ShowFatFsError(res);
    }
    printf("done\n");
    //  Set timestamp
    printf("Set file timestamp of %s\n", filename);
//    RTC_GetRtcDateTime(&DateTime);
    res = set_timestamp(filename, 2000 + 19, 05, 24, 23, 59, 59);
    if (res != FR_OK)
    {
      ShowFatFsError(res);
    }
    printf("done\n");

    /* Append file */
    printf("Append file %s\n", filename);
    res = f_open(&fil, filename, FA_OPEN_APPEND | FA_WRITE);
    if (res != FR_OK)
    {
      ShowFatFsError(res);
    }
    /* Write message */
    snprintf(wtext, 128, "This will be appended to the file.\n");
    length = strlen(wtext);
    res = f_write(&fil, wtext, length, &wbytes);
    if (res != FR_OK)
    {
      ShowFatFsError(res);
    }
    /* Close file */
    //  printf("Close file\n");
    res = f_close(&fil);
    if (res != FR_OK)
    {
      ShowFatFsError(res);
    }
    //  Set timestamp
    printf("Set file timestamp of %s\n", filename);
//    RTC_GetRtcDateTime(&DateTime);
    res = set_timestamp(filename, 2000 + 19, 05, 24, 23, 59, 59);
    if (res != FR_OK)
    {
      ShowFatFsError(res);
    }

    /* Create file */
    strcpy(filename, "hello2.txt");
    printf("Create file %s\n", filename);
    res = f_open(&fil, filename, FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK)
    {
      ShowFatFsError(res);
    }
    /* Write message */
    UINT wbytes2; /* File write counts, Bytes written */
    char wtext2[] = "The quick brown\nfox jumps over\nthe lazy dog.\n";
    int length2 = strlen(wtext2);
    res = f_write(&fil, wtext2, strlen(wtext2), &wbytes2);
    if (res != FR_OK)
    {
      ShowFatFsError(res);
    }
    if (wbytes2 != length2)
    {
      printf("FAILED\n");
    }
    /* Close file */
    //  printf("Close file\n");
    res = f_close(&fil);
    if (res != FR_OK)
    {
      ShowFatFsError(res);
    }
    //  Set timestamp
    printf("Set file timestamp of %s\n", filename);
//    RTC_GetRtcDateTime(&DateTime);
    res = set_timestamp(filename, 2000 + 19, 05, 24, 23, 59, 59);
    if (res != FR_OK)
    {
      ShowFatFsError(res);
    }

    /* Create file */
    strcpy(filename, "my_text_file.txt");
    printf("Create file %s\n", filename);
    res = f_open(&fil, filename, FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK)
    {
      ShowFatFsError(res);
    }
    /* Write to file */
    f_puts("Hello world!!!\n", &fil);
    f_puts("Test.\n", &fil);
    f_printf(&fil, "%d\n", 1234); /* "1234" */
    f_printf(&fil, "%04x\n", 0xAB3); /* "0ab3" */
    f_printf(&fil, "%s\n", "This is an additional line.");
    f_printf(&fil, "And another one...\n");
    /* Close file */
    //  printf("Close file\n");
    res = f_close(&fil);
    if (res != FR_OK)
    {
      ShowFatFsError(res);
    }
    //  Set timestamp
    printf("Set file timestamp of %s\n", filename);
//    RTC_GetRtcDateTime(&DateTime);
    res = set_timestamp(filename, 2000 + 19, 05, 24, 23, 59, 59);
    if (res != FR_OK)
    {
      ShowFatFsError(res);
    }
  }

  /* Read file */
//  strcpy(filename, "hello.txt");
//  printf("Read file %s\n", filename);
//  res = f_open(&fil, filename, FA_READ);
//  if (res != FR_OK)
//  {
//    ShowFatFsError(res);
//  }
//  printf(TERMINAL_LIGHT_YELLOW);
//  while (!f_eof(&fil))
//  {
////    while (f_tell(&fil)  <  f_size(&fil)) {
//    f_gets(rstring, 64, &fil);
//    printf("%s", rstring);
//  }
//  printf(TERMINAL_DEFAULT);
//  /* Close file */
//    printf("Close file\n");
//  res = f_close(&fil);
//  if (res != FR_OK)
//  {
//    ShowFatFsError(res);
//  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if (HAL_GetTicks(500))
    {
      HAL_GPIO_TogglePin(GPIOB, LD1_Pin|LD2_Pin|LD3_Pin);
    }

//    HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD2_Pin|LD3_Pin, GPIO_PIN_RESET);
//    HAL_Delay(500);
//    HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD2_Pin|LD3_Pin, GPIO_PIN_SET);
//    HAL_Delay(500);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
//    MX_LWIP_Process();

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Supply configuration update enable 
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
  /** Configure the main internal regulator output voltage 
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 16; // 8 = 100MHz SDMMC1
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART3|RCC_PERIPHCLK_SPI4
                              |RCC_PERIPHCLK_SPI1|RCC_PERIPHCLK_SPI2
                              |RCC_PERIPHCLK_SDMMC|RCC_PERIPHCLK_USB;
  PeriphClkInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL;
  PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL;
  PeriphClkInitStruct.Spi45ClockSelection = RCC_SPI45CLKSOURCE_D2PCLK1;
  PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
  PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Enable USB Voltage detector 
  */
  HAL_PWREx_EnableUSBVoltageDetector();
}

/* USER CODE BEGIN 4 */
void HAL_IncTicks(void)
{
  msTick += (uint32_t) 1;
}

void HAL_StartTicks(void)
{
  msTickPrevious = msTick;
}

uint8_t HAL_GetTicks(uint32_t ms)
{
  if ((msTick - msTickPrevious) >= ms)
  {
    msTickPrevious = msTick;
    return TRUE;
  }
  return FALSE;
}

/**
  * @brief  Configure the MPU attributes.
  * @note   The Base Address 0x24000000 is the SRAM1 accessible by the SDIO internal DMA.
            the configured region is 512Kb size.
  * @param  None
  * @retval None
  */
void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct;

  /* Disable the MPU */
  HAL_MPU_Disable();

  /* Configure the MPU attributes as WT for SRAM1 */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x24000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_512KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0; // MPU_REGION_NUMBER1
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
