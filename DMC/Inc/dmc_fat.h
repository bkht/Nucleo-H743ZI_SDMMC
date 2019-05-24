#ifndef __DMC_FAT_H
#define __DMC_FAT_H
#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"
#include "main.h"
#include "fatfs.h"
//#include "ff.h"

FRESULT scan_files(char* path, uint8_t printLegend);
void ShowFatFsError(FRESULT res);
void ShowDiskStatus(DSTATUS stat);
FRESULT set_timestamp(char *obj, /* Pointer to the file name */
int year, int month, int mday, int hour, int min, int sec);

#ifdef __cplusplus
}
#endif
#endif /* __DMC_FAT_H */
