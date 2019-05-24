/* HAL library for driving the Microchip MCP79412 Real Time Clock
 * datasheet link : http://ww1.microchip.com/downloads/en/DeviceDoc/22266B.pdf
 * WARNING : sda and sdl should be pulled up with 2.2k resistor
 *
 * 64 Bytes SRAM
 * 128 Bytes EEPROM
 *
 */

/*
 http://www.cplusplus.com/reference/ctime/strftime/
 %a   Abbreviated weekday name *  Thu
 %A   Full weekday name * Thursday
 %b   Abbreviated month name *    Aug
 %B   Full month name *   August
 %d   Day of the month, zero-padded (01-31)   23
 %e   Day of the month, space-padded ( 1-31)  23
 %F   Short YYYY-MM-DD date, equivalent to %Y-%m-%d   2001-08-23
 %H   Hour in 24h format (00-23)  14
 %j   Day of the year (001-366)   235
 %m   Month as a decimal number (01-12)   08
 %M   Minute (00-59)  55
 %R   24-hour HH:MM time, equivalent to %H:%M 14:55
 %S   Second (00-61)  02
 %T   ISO 8601 time format (HH:MM:SS), equivalent to %H:%M:%S 14:55:02
 %u   ISO 8601 weekday as number with Monday as 1 (1-7)   4
 %V   ISO 8601 week number (00-53)    34
 %w   Weekday as a decimal number with Sunday as 0 (0-6)  4
 %W   Week number with the first Monday as the first day of week one (00-53)  34
 %X   Time representation *   14:55:02
 %y   Year, last two digits (00-99)   01
 %Y   Year    2001

 http://www.cplusplus.com/reference/ctime/tm/
 Member   Type    Meaning                        Range
 tm_sec   int     seconds after the minute       0-61*
 tm_min   int     minutes after the hour         0-59
 tm_hour  int     hours since midnight           0-23
 tm_mday  int     day of the month               1-31
 tm_mon   int     months since January           0-11
 tm_year  int     years since 1900
 tm_wday  int     days since Sunday              0-6     (0 = Sunday)
 tm_yday  int     days since January 1           0-365
 tm_isdst         int Daylight Saving Time flag
 The Daylight Saving Time flag (tm_isdst) is greater than zero if Daylight Saving Time is in effect,
 zero if Daylight Saving Time is not in effect, and less than zero if the information is not available.
 * tm_sec is generally 0-59. The extra range is to accommodate for leap seconds in certain systems.

 http://www.epochconverter.com/programming/c
 Convert from epoch to human readable date
 time_t     now;
 struct tm  ts;
 char       buf[80];
 // Get current time
 time(&now);
 // Format time, "ddd yyyy-mm-dd hh:mm:ss zzz"
 ts = *localtime(&now);
 strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", &ts);
 printf("%s\n", buf);

 Convert from human readable date to epoch
 struct tm t;
 time_t t_of_day;
 t.tm_year = 2011-1900;
 t.tm_mon = 7;           // Month, 0 - jan
 t.tm_mday = 8;          // Day of the month
 t.tm_hour = 16;
 t.tm_min = 11;
 t.tm_sec = 42;
 t.tm_isdst = -1;        // Is DST on? 1 = yes, 0 = no, -1 = unknown
 t_of_day = mktime(&t);
 printf("seconds since the Epoch: %ld\n", (long) t_of_day)

 https://github.com/raburton/esp8266/blob/master/drivers/ds3231.c
 https://github.com/raburton/esp8266/blob/master/drivers/ds3231.h

 // https://www.unixtimestamp.com/

 */
//#include "mbed.h"
//#include "macros.h"
//#include <stdint.h>
#ifndef __DMC_RTC_MCP79412_H
#define __DMC_RTC_MCP79412_H

/* C++ detection */
#ifdef __cplusplus
extern "C"
{
#endif

#include "stm32h7xx_hal.h"
#include "main.h"
#include "i2c.h"
#include "dmc_macros.h"
//#include "summertime.h"
//#include "daylight.h"

// I2C speed
#define RTC_I2C_SPEED_100		100
#define RTC_I2C_SPEED_400		400
#define RTC_I2C_SPEED_1000		1000

// MCP7941x I2C Addresses
#define MCP79412_RTC_ADDR       0x6F
#define MCP79412_EEPROM_ADDR    0x57
#define MAC_LOCATION 0xF2   // Starts at 0xF0 but we are only interested in 6 bytes.
#define RTC_LOCATION 0x00

#ifndef NULL
#define NULL 	(void *)0
#endif

//#ifndef FALSE
#define FALSE 0U
//#endif

//#ifndef TRUE
#define TRUE  1U
//#endif

//MCP7941x Register Addresses
#define TIME_REG            0x00    // 7 registers, Seconds, Minutes, Hours, DOW, Date, Month, Year
#define DAY_REG             0x03    // the RTC Day register contains the OSCON, VBAT, and VBATEN bits
#define YEAR_REG            0x06    // RTC year register
#define CTRL_REG            0x07    // control register
#define CALIB_REG           0x08    // calibration register
#define UNLOCK_ID_REG       0x09    // unlock ID register
#define ALM0_REG            0x0A    // alarm 0, 6 registers, Seconds, Minutes, Hours, DOW, Date, Month
#define ALM1_REG            0x11    // alarm 1, 6 registers, Seconds, Minutes, Hours, DOW, Date, Month
#define ALM0_DAY            0x0D    // DOW register has alarm config/flag bits
#define PWRDWN_TS_REG       0x18    // power-down timestamp, 4 registers, Minutes, Hours, Date, Month
#define PWRUP_TS_REG        0x1C    // power-up timestamp, 4 registers, Minutes, Hours, Date, Month
#define TIMESTAMP_SIZE      8       // number of bytes in the two timestamp registers
#define SRAM_START_ADDR     0x20    // first SRAM address
#define SRAM_END_ADDR       0x5F    // last SRAM address (96-32 = 64 Bytes SRAM)
#define SRAM_SIZE           64      // number of bytes of SRAM
#define EEPROM_START_ADDR   0x00    // first EEPROM address
#define EEPROM_END_ADDR     0x7F    // last EEPROM address (128 Bytes EEPROM)
#define EEPROM_SIZE         128     // number of bytes of EEPROM
#define EEPROM_PAGE_SIZE    8       // number of bytes on an EEPROM page
#define UNIQUE_ID_ADDR      0xF0    // starting address for unique ID
#define UNIQUE_ID_SIZE      8       // number of bytes in unique ID

#define UNLOCK_ID_CODE1     0x55    // PROTECTED EEPROM UNLOCK SEQUENCE
#define UNLOCK_ID_CODE2     0xAA    // PROTECTED EEPROM UNLOCK SEQUENCE

//Control Register bits
#define OUT     7   // sets logic level on MFP when not used as square wave output
#define SQWE    6   // set to enable square wave output
#define ALM1    5   // alarm 1 is active
#define ALM0    4   // alarm 0 is active
#define EXTOSC  3   // set to drive the RTC registers from an external oscillator instead of a crystal
#define RS2     2   // RS2:0 set square wave output frequency: 0==1Hz, 1==4096Hz, 2==8192Hz, 3=32768Hz
#define RS1     1
#define RS0     0

//Other Control Bits
#define ST      7   // Seconds register (TIME_REG) oscillator start/stop bit, 1==Start, 0==Stop
#define HR1224  6   // Hours register (TIME_REG+2) 12 or 24 hour mode (24 hour mode==0)
#define AMPM    5   // Hours register (TIME_REG+2) AM/PM bit for 12 hour mode
#define OSCON   5   // Day register (TIME_REG+3) oscillator running (set and cleared by hardware)
#define VBAT    4   // Day register (TIME_REG+3) set by hardware when Vcc fails and RTC runs on battery.
// VBAT is cleared by software, clearing VBAT also clears the timestamp registers
#define VBATEN  3   // Day register (TIME_REG+3) VBATEN==1 enables backup battery, VBATEN==0 disconnects the VBAT pin (e.g. to save battery)
#define LP      5   // Month register (TIME_REG+5) leap year bit

//Alarm Control Bits
#define ALMPOL  7   // Alarm Polarity: Defines the logic level for the MFP when an alarm is triggered.
#define ALMC2   6   // Alarm configuration bits determine how alarms match. See ALM_MATCH defines below.
#define ALMC1   5
#define ALMC0   4
#define ALMIF   3   // Alarm Interrupt Flag: Set by hardware when an alarm was triggered, cleared by software.
// Note ALM_MATCH_DAY triggers alarm at midnight
#define ALARM_0 0   // constants for calling functions
#define ALARM_1 1

#define MCP79412_SET        0
#define MCP79412_CLEAR      1
#define MCP79412_REPLACE    2

#define NTP_OFFSET          2208988800ULL

#define I2C_TIMEOUT			10000

//convenience macros to convert to and from tm years
#define  tmYearToCalendar(Y) ((Y) + 1970)  // full four digit year
#define  CalendarYrToTm(Y)   ((Y) - 1970)
#define  tmYearToY2k(Y)      ((Y) - 30)    // offset is from 2000
#define  y2kYearToTm(Y)      ((Y) + 30)

//    typedef struct DateTime {
//		int year;
//		int mon;
//		int mday;
//		int wday;
//		int yday;
//		int hour;
//		int min;
//		int sec;
//    };
//
//	struct tm {
//		uint8_t tm_sec;
//		uint8_t tm_min;
//		uint8_t tm_hour;
//		uint8_t tm_wday;
//		uint8_t tm_mday;
//		uint8_t tm_mon;
//		uint8_t tm_year;
//		uint8_t tm_sec;
//		uint8_t tm_min;
//		uint8_t tm_hour;
//		uint8_t tm_mday;
//		uint8_t tm_mon;
//		uint8_t tm_year;
//		uint8_t tm_isdst;
//	};

enum RtcSqwave
{
	SQWAVE_1_HZ,
	SQWAVE_4096_HZ,
	SQWAVE_8192_HZ,
	SQWAVE_32768_HZ,
	SQWAVE_NONE
};

enum RtcMatch
{
	ALM_MATCH_SECONDS,
	ALM_MATCH_MINUTES,
	ALM_MATCH_HOURS,
	ALM_MATCH_DAY,
	ALM_MATCH_DATE,
	ALM_RESERVED_5,
	ALM_RESERVED_6,
	ALM_MATCH_DATETIME,
	ALM_DISABLE
};

struct DMC_I2C_RTC_DATE_TIME
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

//void DMC_RTC_SetRtc(uint8_t second, uint8_t minute, uint8_t hour, uint8_t dayOfMonth, uint8_t month, uint8_t year);

void DMC_I2cRtcIncSysTick(void);
void DMC_I2cRtcResetSysTick(void);
uint32_t DMC_I2cRtcGetSysTick(void);

void DMC_I2cRtcSetI2cHandle(I2C_HandleTypeDef hi2c);
void DMC_I2cRtcInit(I2C_HandleTypeDef hi2c);
void DMC_I2cRtcSetTimeOffset(uint32_t offset);
uint32_t DMC_I2cRtcGetTimeOffset(void);
void DMC_I2cRtcFromEpoch(uint32_t epoch, RTC_TimeTypeDef *time, RTC_DateTypeDef *date);
void DMC_I2cRtcSetRtcFromEpoch(uint32_t epoch);
void DMC_I2cRtcSetDateAndTime(RTC_TimeTypeDef *sTime, RTC_DateTypeDef *sDate);
void DMC_I2cRtcGetDateAndTime(RTC_TimeTypeDef *sTime, RTC_DateTypeDef *sDate);

uint8_t DMC_I2cRtcRegisterWrite(uint16_t address, uint8_t *data, uint16_t length);
uint8_t DMC_I2cRtcRegisterRead(uint16_t address, uint8_t *data, uint16_t length);
uint8_t DMC_I2cRtcSramWrite(uint16_t address, uint8_t *data, uint16_t length);
uint8_t DMC_I2cRtcSramRead(uint16_t address, uint8_t *data, uint16_t length);
uint8_t DMC_I2cRtcEepromWrite(uint16_t address, uint8_t *data, uint16_t length);
uint8_t DMC_I2cRtcEepromRead(uint16_t address, uint8_t *data, uint16_t length);

void DMC_I2cRtcSetFrequency(uint16_t freq);

uint8_t DMC_I2cRtcGetFlag(uint8_t reg, uint8_t mask, uint8_t *flag);
void DMC_I2cRtcSetFlag(uint8_t reg, uint8_t bits, uint8_t mode);
int DMC_I2cRtcReadRegister(uint8_t reg);
void DMC_I2cRtcReadRegisters(uint8_t reg, uint8_t *outbuf, uint8_t length);
void DMC_I2cRtcWriteRegister(uint8_t reg, uint8_t byte);
void DMC_I2cRtcWriteRegisters(uint8_t reg, uint8_t *inbuf, uint8_t length);

void DMC_I2cRtcGetMacAddress(uint8_t *mac_address);
void DMC_I2cRtcSetMacAddress(uint8_t *mac_address);

void DMC_I2cRtcUnlockUniqueID();
//void RTC_SetRtcDateTime(uint8_t second, uint8_t minute, uint8_t hour,
//		uint8_t dayOfMonth, uint8_t month, uint8_t year);
void DMC_I2cRtcSetDateTime(struct DMC_I2C_RTC_DATE_TIME *DateTime);
//void RTC_GetRtcDateTime(uint8_t *second, uint8_t *minute, uint8_t *hour,
//		uint8_t *dayOfWeek, uint8_t *dayOfMonth, uint8_t *month, uint8_t *year);
void DMC_I2cRtcGetDateTime(struct DMC_I2C_RTC_DATE_TIME *DateTime);
uint8_t DMC_I2cRtcCheckTimeLost(void);
void DMC_I2cRtcEnableClock();
void DMC_I2cRtcDisableClock();
void DMC_I2cRtcEnableBattery();

void DMC_I2cRtcWriteSramByte(uint8_t location, uint8_t data);
uint8_t DMC_I2cRtcWriteSramBytes(uint8_t location, uint8_t *data, uint8_t length);
uint8_t DMC_I2cRtcReadSramByte(uint8_t location);
uint8_t DMC_I2cRtcReadSramBytes(uint8_t location, uint8_t *data, uint8_t length);

void DMC_I2cRtcWriteEepromByte(uint8_t location, uint8_t data);
uint8_t DMC_I2cRtcWriteEepromBytes(uint8_t location, uint8_t *data, uint8_t length);
uint8_t DMC_I2cRtcReadEepromByte(uint8_t location);
uint8_t DMC_I2cRtcReadEepromBytes(uint8_t location, uint8_t *data, uint8_t length);
void DMC_I2cRtcWaitEepromWriteIsReady(void);

int DMC_I2cRtcCalibrationRead(void);
void DMC_I2cRtcCalibrationWrite(int value);
void DMC_I2cRtcReadUniqueId(uint8_t *uniqueID);
void DMC_I2cRtcGetEUI64(uint8_t *uniqueID);
uint8_t DMC_I2cRtcCheckPowerFail(time_t *powerDown, time_t *powerUp);
void DMC_I2cRtcSquareWave(uint8_t freq);
void DMC_I2cRtcSetAlarm(uint8_t alarmNumber, time_t alarmTime);
void DMC_I2cRtcEnableAlarm(uint8_t alarmNumber, uint8_t alarmType);
uint8_t DMC_I2cRtcCheckAlarm(uint8_t alarmNumber);
void DMC_I2cRtcSetOutLevelMPF(uint8_t level);
void DMC_I2cRtcSetAlarmPolarityMPF(uint8_t polarity);
uint8_t DMC_I2cRtcCheckIsRunning(void);
void DMC_I2cRtcSetVBATENBit(uint8_t enable);

//    uint8_t getSummerTime(void);
//    int dayOfYearC(void);
//    char * getSunRise(void);
//    char * getSunSet(void);
//    char * getDayLength(void);
//    int getSunRiseMinute(void);
//    int getSunSetMinute(void);
//    uint8_t checkSunRise(void);

void substr1(char *s, char *d, int pos, int len);
char * substr2(char *s, int pos, int len);

// Mbed dateTime
struct tm RTC_SetSystemDateTime(uint8_t second, uint8_t minute, uint8_t hour,
		uint8_t dayOfMonth, uint8_t month, uint8_t year);
void DMC_I2cRtcGetSystemDateTime(uint8_t *second, uint8_t *minute, uint8_t *hour,
		uint8_t *dayOfWeek, uint8_t *dayOfMonth, uint8_t *month, uint8_t *year);
void DMC_I2cRtcSetRTCToSystemDateTime(void);
void DMC_I2cRtcSetSystemToRTCDateTime(void);
void DMC_I2cRtcSetRTCFromTm(struct tm *t);
struct tm DMC_I2cRtcGetTmFromRTC(void);

time_t DMC_I2cRtcGetSecondsEpoch(void);
void DMC_I2cRtcSetSecondsEpoch(time_t t);

void DMC_I2cRtcGetRtcDateTimeAsTm(void);
time_t DMC_I2cRtcConvertDateTimeToTimestamp(uint8_t second, uint8_t minute, uint8_t hour,
		uint8_t dayOfMonth, uint8_t month, uint8_t year);

//int RTC_bcd2dec(int k); // bcd to decimal conversion
//int RTC_dec2bcd(int k); // decimal to bcd conversion
int DMC_I2cRtcBcdToDec(int val);
int DMC_I2cRtcDecToBcd(int val);

uint8_t DMC_I2cRtcGetDayOfWeek(uint8_t d, uint8_t m, uint8_t y);
char * DMC_I2cRtcGetDayOfWeekString(uint8_t d, uint8_t m, uint8_t y);
uint8_t DMC_I2cRtcYearYYYYIsLeap(uint16_t year);
uint8_t DMC_I2cRtcYearIsLeap(uint8_t year);
uint16_t DMC_I2cRtcGetDayOfYear(uint16_t year, int month, int day);
uint8_t DMC_I2cRtcGetNoOfDaysInMonth(uint16_t year, uint8_t month);
uint16_t DMC_I2cRtcGetNoOfDaysInYear(uint16_t year);

char* DMC_I2cRtcGetDateTimeString(void);

uint8_t DMC_I2cRtcCompareSramBytes(uint8_t location, uint8_t *compare, uint8_t length);
uint8_t DMC_I2cRtcCompareEepromBytes(uint8_t location, uint8_t *compare, uint8_t length);
void DMC_I2cRtcEepromWriteRequest(uint8_t location, uint8_t *data, uint8_t length);
void DMC_I2cRtcEepromWriteResetTickCounter(void);
void DMC_I2cRtcEepromWriteTick(void);
void DMC_I2cRtcEepromWritePlanned(void);

#ifdef __cplusplus
}
#endif

#endif /* __DMC_RTC_MCP79412_H */
