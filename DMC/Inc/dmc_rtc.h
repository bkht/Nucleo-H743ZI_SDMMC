#ifndef __DMC_RTC_H
#define __DMC_RTC_H
#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"
#include "main.h"
//#include "rtc.h"

#define RTC_MCU_STATUS_TIME_OK              0x4321       /* RTC time OK */

struct DMC_MCU_RTC_DATE_TIME
{
	uint8_t Second;
	uint8_t Minute;
	uint8_t Hour;
	uint8_t DayOfWeek;
	uint8_t DayOfMonth;
	uint16_t DayOfYear;
	uint8_t Month;
	uint8_t Year;
	uint8_t DaysInMonth;
	uint16_t DaysInYear;
};

void DMC_McuRtcInit(void);
void DMC_SetTimeOffset(uint32_t offset);
uint32_t DMC_GetTimeOffset(void);
void DMC_McuRtcFromEpoch(uint32_t epoch, RTC_TimeTypeDef *time, RTC_DateTypeDef *date);
void DMC_McuRtcSetRtcFromEpoch(uint32_t epoch);
void DMC_McuRtcShowRtc(void);

void DMC_McuRtcSetDateTime(struct DMC_MCU_RTC_DATE_TIME *DateTime);
void DMC_McuRtcGetDateTime(struct DMC_MCU_RTC_DATE_TIME *DateTime);
void DMC_McuRtcGetDateAndTime(RTC_TimeTypeDef *sTime, RTC_DateTypeDef *sDate);
void DMC_McuRtcSetAlarmA(struct DMC_MCU_RTC_DATE_TIME *DateTime);
void DMC_McuRtcSetAlarmB(struct DMC_MCU_RTC_DATE_TIME *DateTime);
void DMC_McuRtcDisableAlarmA(void);
void DMC_McuRtcDisableAlarmB(void);

void DMC_McuRtcAddSecond(struct DMC_MCU_RTC_DATE_TIME *DateTime);
time_t DMC_McuRtcGetTimeStamp(void);

int DMC_McuRtcBcdToDec(int bcd);
int DMC_McuRtcDecToBcd(int dec);

uint8_t DMC_McuRtcGetDayOfWeek(uint8_t d, uint8_t m, uint8_t y);
char * DMC_McuRtcGetDayOfWeekString(uint8_t d, uint8_t m, uint8_t y);
uint8_t DMC_McuRtcYearYYYYIsLeap(uint16_t year);
uint8_t DMC_McuRtcYearIsLeap(uint8_t year);
uint16_t DMC_McuRtcGetDayOfYear(uint16_t year, int month, int day);
uint8_t DMC_McuRtcGetNoOfDaysInMonth(uint16_t year, uint8_t month);
uint16_t DMC_McuRtcGetNoOfDaysInYear(uint16_t year);
char* DMC_McuRtcGetTimeString(struct DMC_MCU_RTC_DATE_TIME *DateTime);
char* DMC_McuRtcGetDateTimeString(struct DMC_MCU_RTC_DATE_TIME *DateTime);

void DMC_McuRtcWriteUint32BKPSRAM(uint16_t location, uint32_t value);
uint32_t DMC_McuRtcReadUint32BKPSRAM(uint16_t location);
void DMC_McuRtcWriteBackupRegister(uint8_t location, uint32_t value);
uint32_t DMC_McuRtcReadBackupRegister(uint8_t location);

#ifdef __cplusplus
}
#endif
#endif /* __DMC_RTC_H */
