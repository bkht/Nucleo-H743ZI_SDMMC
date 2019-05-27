# Nucleo-H743ZI STM32H7 Micro-SD Card on 4-bit SDMMC1 and USB Device Storage

## Hardware:
Nucleo-H743ZI 

## MCU
STM32H743ZI (Revision Y)

## Software:
STM32CubeMX Version 5.2.0
STM32Cube_FW_H7_V1.4.0
Atollic TrueSTUDIO 9.3.0

Implementation of SD-Card on SDMMC1 for STM32H7 boards using STM32 HAL.

## Demo

Uses SDMMC1 in 4-bit mode to access an Micro-SD Card.
USB Devive Storage is used to access the SD-Card contents via the USB port on the Nucleo board.

Some files are read and written, and some output is transmitted via the UART to ST-Link on 115200 Baud.
