// QSPI_HandleTypeDef hqspi;

#include "dmc_sram_qspi.h"

#define BUFFER_SIZE         ((uint32_t)0x0200)
#define WRITE_READ_ADDR     ((uint32_t)0x0050)

uint8_t qspi_aTxBuffer[BUFFER_SIZE];
uint8_t qspi_aRxBuffer[BUFFER_SIZE];

