// DayOfWeek 1-7, 1 = Mon

#include <dmc_convert.h>
#include <string.h>
#include "dmc_rtc.h"
#include "dmc_print.h"

#include <time.h>

/* Global Vars */
time_t timestamp;
struct tm currTime;
char DmcRtcDriverDateTimeString[24];	// 2018-11-14 11:20:33 20 positions with termination character

uint32_t DMC_McuRtcTimeOffset = 0;

//struct DMC_MCU_RTC_DATE_TIME DMC_RTC_DateTime;

void DMC_McuRtcInit(void)
{
	/* Enable PWR peripheral clock */
	__HAL_RCC_PWR_CLK_ENABLE();

	/* Allow access to BKP Domain */
	HAL_PWR_EnableBkUpAccess();

//	/* Get RTC status */
//	dmc_puts("HAL_RTCEx_BKUPRead\n");
//	uint32_t status = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR31);
//
//	/* Check if RTC already initialized */
//	if (status == RTC_STATUS_TIME_OK)
//	{
//		dmc_puts("OK\n");
//	}
}

void DMC_McuRtcSetTimeOffset(uint32_t offset)
{
  DMC_McuRtcTimeOffset = offset;
}

uint32_t DMC_McuRtcGetTimeOffset(void)
{
  return DMC_McuRtcTimeOffset;
}

// Convert epoch time to Date/Time structures
// RTC_FromEpoch(ts + 3600, &currentTime, &currentDate);
// RTC_TimeTypeDef currentTime;
// RTC_DateTypeDef currentDate;
// time_t timestamp;
// struct tm currTime;
void DMC_McuRtcFromEpoch(uint32_t epoch, RTC_TimeTypeDef *time, RTC_DateTypeDef *date)
{
//  RTC_TimeTypeDef sTime;
//  RTC_DateTypeDef sDate;

//  dmc_puts("DMC_McuRtcFromEpoch: ");
//  dmc_putintcr(epoch);

  uint32_t tm;
  uint32_t t1;
  uint32_t a;
  uint32_t b;
  uint32_t c;
  uint32_t d;
  uint32_t e;
  uint32_t m;
  int16_t year = 0;
  int16_t month = 0;
  int16_t dow = 0;
  int16_t mday = 0;
  int16_t hour = 0;
  int16_t min = 0;
  int16_t sec = 0;
  uint64_t JD = 0;
  uint64_t JDN = 0;

  // These hardcore math's are taken from http://en.wikipedia.org/wiki/Julian_day

  JD = ((epoch + 43200) / (86400 >> 1)) + (2440587 << 1) + 1;
  JDN = JD >> 1;

  tm = epoch;
  t1 = tm / 60;
  sec = tm - (t1 * 60);
  tm = t1;
  t1 = tm / 60;
  min = tm - (t1 * 60);
  tm = t1;
  t1 = tm / 24;
  hour = tm - (t1 * 24);

  dow = JDN % 7;
  a = JDN + 32044;
  b = ((4 * a) + 3) / 146097;
  c = a - ((146097 * b) / 4);
  d = ((4 * c) + 3) / 1461;
  e = c - ((1461 * d) / 4);
  m = ((5 * e) + 2) / 153;
  mday = e - (((153 * m) + 2) / 5) + 1;
  month = m + 3 - (12 * (m / 10));
  year = (100 * b) + d - 4800 + (m / 10);

  date->Year = year - 2000;
  date->Month = month;
  date->Date = mday;
  date->WeekDay = dow;
  time->Hours = hour;
  time->Minutes = min;
  time->Seconds = sec;
}

void DMC_McuRtcSetRtcFromEpoch(uint32_t epoch)
{
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;

  DMC_McuRtcFromEpoch(epoch + DMC_McuRtcTimeOffset, &sTime, &sDate);

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }

  //  sDate->WeekDay = DMC_McuRtcGetDayOfWeek(mday, month, year - 2000);
  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
}

void DMC_McuRtcShowRtc(void)
{
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;

  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

  HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

  dmc_puts("Get Date/Time: ");
  dmc_putint(sDate.WeekDay);
  dmc_puts(" ");
  dmc_putint2(sDate.Date, '0');
  dmc_puts("-");
  dmc_putint2(sDate.Month, '0');
  dmc_puts("-");
  dmc_putint2(sDate.Year, '0');
  dmc_puts(" ");
  dmc_putint2(sTime.Hours, '0');
  dmc_puts(":");
  dmc_putint2(sTime.Minutes, '0');
  dmc_puts(":");
  dmc_putint2(sTime.Seconds, '0');
  dmc_putcr();
}

void DMC_McuRtcSetDateTime(struct DMC_MCU_RTC_DATE_TIME *DateTime)
{
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;

	DateTime->DayOfWeek = DMC_McuRtcGetDayOfWeek(DateTime->DayOfMonth, DateTime->Month, DateTime->Year);     // 1-7
	DateTime->DaysInMonth = DMC_McuRtcGetNoOfDaysInMonth((uint16_t) DateTime->Year, DateTime->Month);
	DateTime->DaysInYear = DMC_McuRtcGetNoOfDaysInYear((uint16_t) DateTime->Year);
	if (DateTime->DayOfMonth > DateTime->DaysInMonth)
	{
		DateTime->DayOfMonth = DateTime->DaysInMonth;
	}

	sTime.Hours = DateTime->Hour;
	sTime.Minutes = DateTime->Minute;
	sTime.Seconds = DateTime->Second;
	sTime.TimeFormat = RTC_HOURFORMAT12_AM;
	sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sTime.StoreOperation = RTC_STOREOPERATION_RESET;
	HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);	// FORMAT_BCD

	sDate.WeekDay = DateTime->DayOfWeek;	// 1-7, 1 = mon
	sDate.Date = DateTime->DayOfMonth;
	sDate.Month = DateTime->Month;
	sDate.Year = DateTime->Year;
	HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);	// FORMAT_BCD
}

void DMC_McuRtcGetDateTime(struct DMC_MCU_RTC_DATE_TIME *DateTime)
{
	RTC_TimeTypeDef sTime = {0};
	RTC_DateTypeDef sDate = {0};

	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

	DateTime->Hour = sTime.Hours;
	DateTime->Minute = sTime.Minutes;
	DateTime->Second = sTime.Seconds;

	/* Get subseconds */
//	data->Subseconds = RTC->SSR;

	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	DateTime->DayOfMonth = sDate.Date;
	DateTime->Month = sDate.Month;
	DateTime->Year = sDate.Year;
	DateTime->DayOfWeek = DMC_McuRtcGetDayOfWeek(DateTime->DayOfMonth, DateTime->Month, DateTime->Year);
	DateTime->DayOfYear = DMC_McuRtcGetDayOfYear(DateTime->DayOfMonth, DateTime->Month, DateTime->Year);
	DateTime->DaysInMonth = DMC_McuRtcGetNoOfDaysInMonth(DateTime->Year, DateTime->Month);
	DateTime->DaysInYear = DMC_McuRtcGetNoOfDaysInYear(DateTime->Year);
}

void DMC_McuRtcGetDateAndTime(RTC_TimeTypeDef *sTime, RTC_DateTypeDef *sDate)
{
  HAL_RTC_GetTime(&hrtc, sTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, sDate, RTC_FORMAT_BIN);
}

void DMC_McuRtcSetAlarmA(struct DMC_MCU_RTC_DATE_TIME *DateTime)
{
	RTC_AlarmTypeDef sAlarm;

	/**Enable the Alarm A */
	sAlarm.AlarmTime.Hours = DateTime->Hour;
	sAlarm.AlarmTime.Minutes = DateTime->Minute;
	sAlarm.AlarmTime.Seconds = DateTime->Second;
	sAlarm.AlarmTime.TimeFormat = RTC_HOURFORMAT12_AM;
	sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
	sAlarm.AlarmMask = RTC_ALARMMASK_SECONDS;
	sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
	sAlarm.AlarmDateWeekDay = DateTime->DayOfWeek;
	sAlarm.Alarm = RTC_ALARM_A;
	HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, FORMAT_BIN);	// FORMAT_BIN, FORMAT_BCD
}

void DMC_McuRtcSetAlarmB(struct DMC_MCU_RTC_DATE_TIME *DateTime)
{
	RTC_AlarmTypeDef sAlarm;

	/**Enable the Alarm A */
	sAlarm.AlarmTime.Hours = DateTime->Hour;
	sAlarm.AlarmTime.Minutes = DateTime->Minute;
	sAlarm.AlarmTime.Seconds = DateTime->Second;
	sAlarm.AlarmTime.TimeFormat = RTC_HOURFORMAT12_AM;
	sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
	sAlarm.AlarmMask = RTC_ALARMMASK_SECONDS;
	sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
	sAlarm.AlarmDateWeekDay = DateTime->DayOfWeek;
	sAlarm.Alarm = RTC_ALARM_B;
	HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, FORMAT_BIN);	// FORMAT_BIN, FORMAT_BCD
}

void DMC_McuRtcDisableAlarmA(void)
{
	HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);
}

void DMC_McuRtcDisableAlarmB(void)
{
	HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_B);
}

void DMC_McuRtcAddSecond(struct DMC_MCU_RTC_DATE_TIME *DateTime)
{
	DateTime->Second++;
	if (DateTime->Second < 60)
	{
		return;
	}
	DateTime->Second = 0;
	DateTime->Minute++;
	if (DateTime->Minute < 60)
	{
		return;
	}
	DateTime->Minute = 0;
	DateTime->Hour++;
	if (DateTime->Hour < 24)
	{
		return;
	}
	DateTime->Hour = 0;
	DateTime->DayOfWeek++;
	if (DateTime->DayOfWeek > 7)
	{
		DateTime->DayOfWeek = 1;
	}
	DateTime->DayOfMonth++;
	DateTime->DaysInMonth = DMC_McuRtcGetNoOfDaysInMonth((uint16_t) DateTime->Year, DateTime->Month);
	if (DateTime->DayOfMonth <= DateTime->DaysInMonth)
	{
		return;
	}
	DateTime->DayOfMonth = 1;
	DateTime->Month++;
	if (DateTime->Month <= 12)
	{
		return;
	}
	DateTime->Month = 1;
	DateTime->Year++;

	DateTime->DaysInMonth = DMC_McuRtcGetNoOfDaysInMonth((uint16_t) DateTime->Year, DateTime->Month);
	DateTime->DaysInYear = DMC_McuRtcGetNoOfDaysInYear((uint16_t) DateTime->Year);
}

/* Code to get timestamp
 *
 *  You must call HAL_RTC_GetDate() after HAL_RTC_GetTime() to unlock the values
 *  in the higher-order calendar shadow registers to ensure consistency between the time and date values.
 *  Reading RTC current time locks the values in calendar shadow registers until Current date is read
 *  to ensure consistency between the time and date values.
 */
time_t DMC_McuRtcGetTimeStamp(void)
{
	struct DMC_MCU_RTC_DATE_TIME DateTime;

	DMC_McuRtcGetDateTime(&DateTime);

    currTime.tm_year = DateTime.Year + 100;  // In fact: 2000 + 18 - 1900
    currTime.tm_mday = DateTime.DayOfMonth;
    currTime.tm_mon  = DateTime.Month - 1;

    currTime.tm_hour = DateTime.Hour;
    currTime.tm_min  = DateTime.Minute;
    currTime.tm_sec  = DateTime.Second;

    timestamp = mktime(&currTime);
    return timestamp;
}

// Convert binary coded decimal to normal decimal numbers:
int DMC_McuRtcBcdToDec(int bcd)
{
	return (((bcd & 0xF0) >> 4) * 10 + (bcd % 16));
}

// Convert normal decimal numbers to binary coded decimal:
int DMC_McuRtcDecToBcd(int dec)
{
	return ((dec / 10 * 16) + (dec % 10));
}

uint8_t DMC_McuRtcGetDayOfWeek(uint8_t d, uint8_t m, uint8_t y)  // y > 1752, 1 <= m <= 12
{
//    struct tm *t;
//    t->tm_year = year - 1900;    // adjust for tm structure required values
//    t->tm_mon  = month - 1;        // adjust for tm structure required values
//    t->tm_mday = mday;
//    t->tm_hour = 0;
//    t->tm_min = 0;
//    t->tm_sec = 0;
//    time_t secondsEpoch = mktime(&t);   // seconds since the Epoch
//    t = localtime(&secondsEpoch);
//    return t->tm_wday;                   // (0=Sunday, 6=Saturday)

	// https://en.wikipedia.org/wiki/Determination_of_the_day_of_the_week
//	y += 2000;
	static uint8_t t[] =
	{ 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
	y -= m < 3;
	return ((y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7) + 1; // 01 - 07, 01 = Sunday
}

char * DMC_McuRtcGetDayOfWeekString(uint8_t d, uint8_t m, uint8_t y)  // y > 1752, 1 <= m <= 12
{
	char *DaysOfWeek[] = { "", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	return DaysOfWeek[DMC_McuRtcGetDayOfWeek(d, m, y)];
}

uint8_t DMC_McuRtcYearYYYYIsLeap(uint16_t year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

uint8_t DMC_McuRtcYearIsLeap(uint8_t year)
{
    return (year % 4 == 0);
}

uint16_t DMC_McuRtcGetDayOfYear(uint16_t year, int month, int day)
{
    static const int days[2][13] = {
        {0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
        {0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}
    };

    if (year > 2000)
	{
		year -= 2000;
	}

    uint8_t leap = DMC_McuRtcYearIsLeap(year);

    return days[leap][month] + day;
}

uint8_t DMC_McuRtcGetNoOfDaysInMonth(uint16_t year, uint8_t month)
{
    static const int days[2][13] = {
        {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
        {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
    };

    if (year > 2000)
	{
		year -= 2000;
	}

    uint8_t leap = DMC_McuRtcYearIsLeap(year);

    return days[leap][month];
}

uint16_t DMC_McuRtcGetNoOfDaysInYear(uint16_t year)
{
    static const int days[2] = {
		365, 366
    };

    if (year > 2000)
	{
		year -= 2000;
	}

    uint8_t leap = DMC_McuRtcYearIsLeap(year);

    return days[leap];
}

/**
  * @brief  Display the current time.
  * @param  showtime : pointer to buffer
  * @retval None
  */
char* DMC_McuRtcGetTimeString(struct DMC_MCU_RTC_DATE_TIME *DateTime)
{
	char tmpStr[5];

	strncpy(DmcRtcDriverDateTimeString, "", 24);

	// hour
	dmc_itoa_len_0(DateTime->Hour, tmpStr, 10, 2);
	strncat(DmcRtcDriverDateTimeString, tmpStr, 24);	// 10
	// ':'
	strncat(DmcRtcDriverDateTimeString, ":", 24);			// :
	// minute
	dmc_itoa_len_0(DateTime->Minute, tmpStr, 10, 2);
	strncat(DmcRtcDriverDateTimeString, tmpStr, 24);	// 49
	// ':'
	strncat(DmcRtcDriverDateTimeString, ":", 24);			// :
	// second
	dmc_itoa_len_0(DateTime->Second, tmpStr, 10, 2);
	strncat(DmcRtcDriverDateTimeString, tmpStr, 24);	// 41

	return DmcRtcDriverDateTimeString;

	/* Display time Format : hh:mm:ss */
//	snprintf((char*)showtime, 9, "%02d:%02d:%02d", DateTime.Hour, DateTime.Minute, DateTime.Second);
}

char* DMC_McuRtcGetDateTimeString(struct DMC_MCU_RTC_DATE_TIME *DateTime)
{
	char tmpStr[5];

//	struct DMC_MCU_RTC_DATE_TIME DateTime;
//	DateTime.Year = 18;			// 00-99
//	DateTime.Month = 12;		// 01-12
//	DateTime.DayOfMonth = 31;	// 01-31
//	DateTime.Hour = 23;			// 00-23
//	DateTime.Minute = 59;		// 00-59
//	DateTime.Second = 58;		// 00-59
//	DateTime.DayOfWeek = 0;		// 1-7, 1 = Sun

//	RTC_GetRtcDateTime(&DateTime);
	strncpy(DmcRtcDriverDateTimeString, "", 24);

	// Weekday
	strncat(DmcRtcDriverDateTimeString, DMC_McuRtcGetDayOfWeekString(DateTime->DayOfMonth, DateTime->Month, DateTime->Year), 24);
	// ' '
	strncat(DmcRtcDriverDateTimeString, " ", 24);

	// 2018-11-14 10:40:41
	// year
	dmc_itoa_len_0(2000 + DateTime->Year, tmpStr, 10, 4);		// 2018
	strncat(DmcRtcDriverDateTimeString, tmpStr, 24);
	// '-'
	strncat(DmcRtcDriverDateTimeString, "-", 24);			// -
	// month
	dmc_itoa_len_0(DateTime->Month, tmpStr, 10, 2);			// 11
	strncat(DmcRtcDriverDateTimeString, tmpStr, 24);
	// '-'
	strncat(DmcRtcDriverDateTimeString, "-", 24);			// -
	// dayOfMonth
	dmc_itoa_len_0(DateTime->DayOfMonth, tmpStr, 10, 2);		// 14
	strncat(DmcRtcDriverDateTimeString, tmpStr, 24);
	// ' '
	strncat(DmcRtcDriverDateTimeString, " ", 24);
	// hour
	dmc_itoa_len_0(DateTime->Hour, tmpStr, 10, 2);
	strncat(DmcRtcDriverDateTimeString, tmpStr, 24);	// 10
	// ':'
	strncat(DmcRtcDriverDateTimeString, ":", 24);			// :
	// minute
	dmc_itoa_len_0(DateTime->Minute, tmpStr, 10, 2);
	strncat(DmcRtcDriverDateTimeString, tmpStr, 24);	// 49
	// ':'
	strncat(DmcRtcDriverDateTimeString, ":", 24);			// :
	// second
	dmc_itoa_len_0(DateTime->Second, tmpStr, 10, 2);
	strncat(DmcRtcDriverDateTimeString, tmpStr, 24);	// 41

	return DmcRtcDriverDateTimeString;
}

void DMC_McuRtcWriteUint32BKPSRAM(uint16_t location, uint32_t value)
{
	*(__IO uint32_t *) (D3_BKPSRAM_BASE + location) = value;
}

uint32_t DMC_McuRtcReadUint32BKPSRAM(uint16_t location)
{
	return *(__IO uint32_t *) (D3_BKPSRAM_BASE + location);
}

void DMC_McuRtcWriteBackupRegister(uint8_t location, uint32_t value)
{
	/* Write data to backup register */
	*(uint32_t *)((&RTC->BKP0R) + 4 * location) = value;
}

uint32_t DMC_McuRtcReadBackupRegister(uint8_t location)
{
	/* Read data from backup register */
	return *(uint32_t *)((&RTC->BKP0R) + 4 * location);
}
