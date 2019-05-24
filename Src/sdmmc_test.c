/*
 * sdmmc_test.c
 *
 *  Created on: Mar 23, 2019
 *      Author: khockuba
 */

#include <stdbool.h>

#include "stm32h7xx_hal.h"
#include "sdmmc_sdio.h"
#include "unity.h"
#include "us_handler.h"

#define SD_BUFFER __attribute__((section(".ram_d1"), aligned(4)))
uint8_t SD_BUFFER buffer_in[512 * 64];
uint8_t SD_BUFFER buffer_out[512 * 64];

void rng_init(void) {
    __HAL_RCC_RNG_CLK_ENABLE();
    RNG->CR |= RNG_CR_RNGEN;
}

uint32_t rng_get(void) {
    TEST_ASSERT((RNG->SR & RNG_SR_CEIS) == 0);
    TEST_ASSERT((RNG->SR & RNG_SR_SEIS) == 0);
    while ((RNG->SR & RNG_SR_DRDY) == 0);
    return RNG->DR;
}

uint64_t cardsize = 0;

void _init_sdmmc(void) {
    bool ret = true;
    DWT_Init();
    SD_Initialize_LL(SDMMC2);
    TEST_ASSERT_TRUE(ret);
    SD_Init(0x03);
    rng_init();
    TEST_ASSERT_TRUE(ret);
    TEST_ASSERT(SD_OK == SD_GetCardInfo());
    printf("Detected sd card:\n");
    printf("Card MNF %d\n", SD_CardInfo.SD_cid.ManufacturerID);
    printf("Card MNF Date %d\n", SD_CardInfo.SD_cid.ManufactDate);
    printf("Card OEM Appli ID %d\n", SD_CardInfo.SD_cid.OEM_AppliID);
    printf("Card Rev %d\n", SD_CardInfo.SD_cid.ProdRev);
    printf("Card SN %lu\n", SD_CardInfo.SD_cid.ProdSN);
    printf("Card blocks %llu\n", SD_CardInfo.CardCapacity);
    printf("Card block size %lu\n", SD_CardInfo.CardBlockSize);
    printf("Card size %llub\n", SD_CardInfo.CardCapacity * SD_CardInfo.CardBlockSize);
}

// Sector is 512B
void _write_read_sector_sdmmc(void) {
    SD_Error_t ret = false;
    uint32_t rd_time, wr_time;
    uint32_t *ptr = (uint32_t*) buffer_in;
    for (int i = 0; i < (512 / 4); i++) {
        *ptr++ = rng_get();
    }
    uint32_t address = rng_get() & SD_CardInfo.CardCapacity;
    if (address > (SD_CardInfo.CardCapacity - 512)) {
        address -= 512;
    }
    wr_time = HAL_GetTick();
    ret = SD_WriteBlocks_DMA(address, (uint32_t*)buffer_in, 512, 1);
    TEST_ASSERT_EQUAL(SD_OK, ret);
    while (SD_CheckWrite());
    while (SD_GetState() == false);
    wr_time = HAL_GetTick() - wr_time;
    rd_time = HAL_GetTick();
    ret = SD_ReadBlocks_DMA(address, (uint32_t*) buffer_out, 512, 1);
    TEST_ASSERT_EQUAL(SD_OK, ret);
    while (SD_CheckRead());
    while (SD_GetState() == false);
    rd_time = HAL_GetTick() - rd_time;
    printf(" Write time %lums, read time %lums\n", wr_time, rd_time);
    TEST_ASSERT_EQUAL_MEMORY_ARRAY(buffer_in, buffer_out, 1, 512);
}

// Sector is 512B
void _write_read_multi_sector_sdmmc(void) {
    SD_Error_t ret = false;
    uint32_t rd_time, wr_time;
    uint32_t *ptr = (uint32_t*) buffer_in;
    for (int i = 0; i < (16 * 512 / 4); i++) {
        *ptr++ = rng_get();
    }
    uint32_t address = rng_get() & SD_CardInfo.CardCapacity;
    if (address > (SD_CardInfo.CardCapacity - (16 * 512))) {
        address -= (16 * 512);
    }
    wr_time = HAL_GetTick();
    ret = SD_WriteBlocks_DMA(address, (uint32_t*)buffer_in, 512, 16);
    TEST_ASSERT_EQUAL(SD_OK, ret);
    while (SD_CheckWrite());
    while (SD_GetState() == false);
    wr_time = HAL_GetTick() - wr_time;
    rd_time = HAL_GetTick();
    ret = SD_ReadBlocks_DMA(address, (uint32_t*) buffer_out, 512, 16);
    TEST_ASSERT_EQUAL(SD_OK, ret);
    while (SD_CheckRead());
    while (SD_GetState() == false);
    rd_time = HAL_GetTick() - rd_time;
    printf(" Write time %lums, read time %lums\n", wr_time, rd_time);
    TEST_ASSERT_EQUAL_MEMORY_ARRAY(buffer_in, buffer_out, 1, 16 * 512);
}

void _write_multi_test_sdmmc(void) {
    SD_Error_t ret = false;
    uint32_t wr_time;
    uint32_t *ptr = (uint32_t*) buffer_in;
    for (int i = 0; i < (16 * 512 / 4); i++) {
        *ptr++ = rng_get();
    }
    uint32_t address = rng_get() & SD_CardInfo.CardCapacity;
    if (address > (SD_CardInfo.CardCapacity - (16 * 512))) {
        address -= (16 * 512);
    }
    wr_time = HAL_GetTick();
    for (int i = 0; i < 1000; i++) {
        ret = SD_WriteBlocks_DMA(address, (uint32_t*)buffer_in, 512, 16);
        TEST_ASSERT_EQUAL(SD_OK, ret);
        while (SD_CheckWrite());
        while (SD_GetState() == false);
    }
    wr_time = HAL_GetTick() - wr_time;
    printf(" Write time %lums\n", wr_time);
    float data_written = 512.0f * 16 * 1000 / 1024 / 1024;
    float time = (float)wr_time / 1000;
    printf(" Write speed %fMB/s\n", data_written / time);
}

void _read_multi_test_sdmmc(void) {
    SD_Error_t ret = false;
    uint32_t rd_time;
    uint32_t address = rng_get() & SD_CardInfo.CardCapacity;
    if (address > (SD_CardInfo.CardCapacity - (16 * 512))) {
        address -= (16 * 512);
    }
    rd_time = HAL_GetTick();
    for (int i = 0; i < 1000; i++) {
        ret = SD_ReadBlocks_DMA(address, (uint32_t*)buffer_out, 512, 16);
        TEST_ASSERT_EQUAL(SD_OK, ret);
        while (SD_CheckRead());
        while (SD_GetState() == false);
    }
    rd_time = HAL_GetTick() - rd_time;
    printf(" Read time %lums\n", rd_time);
    float data_written = 512.0f * 16 * 1000 / 1024 / 1024;
    float time = (float)rd_time / 1000;
    printf(" Read speed %fMB/s\n", data_written / time);
}

void run_sdmmc_test(void) {
    UNITY_BEGIN();
    RUN_TEST(_init_sdmmc);
    RUN_TEST(_write_read_sector_sdmmc);
    RUN_TEST(_write_read_multi_sector_sdmmc);
    RUN_TEST(_write_multi_test_sdmmc);
    RUN_TEST(_read_multi_test_sdmmc);
    UNITY_END();
}




