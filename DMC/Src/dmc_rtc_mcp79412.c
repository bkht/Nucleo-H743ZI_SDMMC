// DayOfWeek 1-7, 1 = Sun
// Jack 30-01-2019, also check this out
// https://github.com/JChristensen/MCP79412RTC/blob/master/src/MCP79412RTC.cpp

#include <string.h>
#include "dmc_rtc_mcp79412.h"
//#include "dmc_macros.h"
#include "dmc_convert.h"

char _address_RTC;
char _address_EEPROM;
//char buffer[32];
uint8_t _error;

uint8_t buf[128];

volatile uint32_t DMC_I2cRtcEepromWriteTickCounter = 0;
volatile uint8_t DMC_I2cRtcEepromWriteRequested = FALSE;
volatile uint8_t DMC_I2cRtcEepromWriteAllowed = FALSE;

uint8_t DMC_I2cRtcEepromDataArray[EEPROM_SIZE];
uint8_t DMC_I2cRtcEepromDataLocation = 0;
uint8_t DMC_I2cRtcEepromDataLength = 0;

uint32_t DMC_I2cRtcTimeOffset = 0;

//    typedef long int __time_t;
typedef __time_t time_t;

struct DMC_I2C_RTC_DATE_TIME DMC_I2C_RTC_DateTime;

struct tm
{
	uint8_t tm_sec;
	uint8_t tm_min;
	uint8_t tm_hour;
	uint8_t tm_wday;
	uint8_t tm_mday;
	uint8_t tm_mon;
	uint8_t tm_year;
	uint8_t tm_isdst;
};

//	DateTime datetime;
struct tm *t;
time_t secondsEpoch;
uint8_t second, minute, hour, dayOfWeek, dayOfMonth, month, year;

char *RtcDriverDaysOfWeek[] =
{ "", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
char RtcDriverDateTimeString[24];	// 2018-11-14 11:20:33 20 positions with termination character

// Group 1: functions used to communicate with those devices that do not have secondary address (such as the sensor we’ll use in the later part):
//	HAL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)
//	HAL_StatusTypeDef HAL_I2C_Master_Receive(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)

// Group 2: functions used to communicate with those devices that have secondary address (such as memory chip AT24Cxxx):
//	HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)
//	HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)

I2C_HandleTypeDef RtcHi2c2;

uint32_t DMC_I2cRtcSysTickCounterNew = 0;
uint32_t DMC_I2cRtcSysTickCounterOld = 0;

void DMC_I2cRtcIncSysTick(void)
{
	DMC_I2cRtcSysTickCounterNew++;
}

void DMC_I2cRtcResetSysTick(void)
{
	DMC_I2cRtcSysTickCounterOld = DMC_I2cRtcSysTickCounterNew;
}

uint32_t DMC_I2cRtcGetSysTick(void)
{
	return DMC_I2cRtcSysTickCounterNew - DMC_I2cRtcSysTickCounterOld;
}

void DMC_I2cRtcSetI2cHandle(I2C_HandleTypeDef hi2c)
{
	RtcHi2c2 = hi2c;
}

void DMC_I2cRtcInit(I2C_HandleTypeDef hi2c)
{
	RtcHi2c2 = hi2c;
	// The control byte begins with a 4-bit control code. For the MCP7941X,
	// this is set as ‘1010’ for EEPROM read and write operations,
	// and ‘1101’ for SRAM/RTCC register read and write operations.
	_address_RTC = MCP79412_RTC_ADDR << 1;
	_address_EEPROM = MCP79412_EEPROM_ADDR << 1;
	DMC_I2cRtcSquareWave(SQWAVE_1_HZ);
	DMC_I2cRtcEepromWriteRequested = FALSE;
	DMC_I2cRtcEepromWriteResetTickCounter();
}

void DMC_I2cRtcSetTimeOffset(uint32_t offset)
{
  DMC_I2cRtcTimeOffset = offset;
}

uint32_t DMC_I2cRtcGetTimeOffset(void)
{
  return DMC_I2cRtcTimeOffset;
}


// Convert epoch time to Date/Time structures
// RTC_FromEpoch(ts + 3600, &currentTime, &currentDate);
// RTC_TimeTypeDef currentTime;
// RTC_DateTypeDef currentDate;
// time_t timestamp;
// struct tm currTime;
void DMC_I2cRtcFromEpoch(uint32_t epoch, RTC_TimeTypeDef *time, RTC_DateTypeDef *date)
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

void DMC_I2cRtcSetRtcFromEpoch(uint32_t epoch)
{
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;

  DMC_McuRtcFromEpoch(epoch + DMC_I2cRtcTimeOffset, &sTime, &sDate);

//  dmc_puts("DMC_I2cRtcSetRtcFromEpoch: ");
//  dmc_putint(sDate.WeekDay);
//  dmc_puts(" ");
//  dmc_putint2(sDate.Date, '0');
//  dmc_puts("-");
//  dmc_putint2(sDate.Month, '0');
//  dmc_puts("-");
//  dmc_putint2(sDate.Year, '0');
//  dmc_puts(" ");
//  dmc_putint2(sTime.Hours, '0');
//  dmc_puts(":");
//  dmc_putint2(sTime.Minutes, '0');
//  dmc_puts(":");
//  dmc_putint2(sTime.Seconds, '0');
//  dmc_putcr();

  DMC_I2cRtcSetDateAndTime(&sTime, &sDate);
}

void DMC_I2cRtcSetDateAndTime(RTC_TimeTypeDef *sTime, RTC_DateTypeDef *sDate)
{
  uint8_t buf[8];
//  sDate->WeekDay = DMC_I2cRtcGetDayOfWeek(sDate->Date, sDate->Month, sDate->Year);     // 1-7

//  uint8_t noOfDaysInMonth = DMC_I2cRtcGetNoOfDaysInMonth((uint16_t) sDate->Year, sDate->Month);
//  if (sDate->Date > noOfDaysInMonth)
//  {
//    sDate->Date = noOfDaysInMonth;
//  }

//  dmc_puts("DMC_I2cRtcSetDateAndTime: ");
//  dmc_putint(sDate->WeekDay);
//  dmc_puts(" ");
//  dmc_putint2(sDate->Date, '0');
//  dmc_puts("-");
//  dmc_putint2(sDate->Month, '0');
//  dmc_puts("-");
//  dmc_putint2(sDate->Year, '0');
//  dmc_puts(" ");
//  dmc_putint2(sTime->Hours, '0');
//  dmc_puts(":");
//  dmc_putint2(sTime->Minutes, '0');
//  dmc_puts(":");
//  dmc_putint2(sTime->Seconds, '0');
//  dmc_putcr();

//  printf("%d %02d-%02d-20%02d %02d:%02d:%02d\n", dayOfWeek, dayOfMonth, month, year, hour, minute, second);

  buf[0] = RTC_LOCATION;
  buf[1] = DMC_I2cRtcDecToBcd(sTime->Seconds) & 0x7f; // set seconds and disable clock (01111111, Bit 7, ST = 0)
  buf[2] = DMC_I2cRtcDecToBcd(sTime->Minutes) & 0x7f;               // set minutes (01111111)
  buf[3] = DMC_I2cRtcDecToBcd(sTime->Hours) & 0x3f; // set hours and to 24hr format (00111111, Bit 6 = 0)
  buf[4] = _BV(VBATEN) | (DMC_I2cRtcDecToBcd(sDate->WeekDay) & 0x07); // set the day and enable battery backup (00000111)|(00001000, Bit 3 = 1)
  buf[5] = DMC_I2cRtcDecToBcd(sDate->Date) & 0x3f;    // set the date in month (00111111)
  buf[6] = DMC_I2cRtcDecToBcd(sDate->Month) & 0x1f;                // set the month (00011111)
  buf[7] = DMC_I2cRtcDecToBcd(sDate->Year);                        // set the year (11111111)
  uint8_t w1 = DMC_I2cRtcRegisterWrite(_address_RTC, buf, 8);

  // Start Clock:
  buf[0] = RTC_LOCATION;
  buf[1] = _BV(ST) | DMC_I2cRtcDecToBcd(sTime->Seconds); // set seconds and enable clock (10000000)
  uint8_t w2 = DMC_I2cRtcRegisterWrite(_address_RTC, buf, 2);

  _error = ((w1 != 0) || (w2 != 0));
}

// Get the date/time
//void RTC_GetRtcDateTime(uint8_t *second, uint8_t *minute, uint8_t *hour, uint8_t *dayOfWeek, uint8_t *dayOfMonth,
//    uint8_t *month, uint8_t *year)
void DMC_I2cRtcGetDateAndTime(RTC_TimeTypeDef *sTime, RTC_DateTypeDef *sDate)
{
  uint8_t buf[8];

  buf[0] = RTC_LOCATION;
  int w = DMC_I2cRtcRegisterWrite(_address_RTC, buf, 1);
  int r = DMC_I2cRtcRegisterRead(_address_RTC, buf, 7);

  _error = ((w != 0) || (r != 0));

  // A few of these need masks because certain bits are control bits
  sTime->Seconds = DMC_I2cRtcBcdToDec(buf[0] & 0x7f);  // 01111111 0-59
  sTime->Minutes = DMC_I2cRtcBcdToDec(buf[1] & 0x7f);  // 01111111 0-59
  sTime->Hours = DMC_I2cRtcBcdToDec(buf[2] & 0x3f);  // 00111111 1-23
//  DateTime->DayOfWeek = RTC_BcdToDec(buf[3] & 0x07);  // 00000111 1-7
  sDate->Date = DMC_I2cRtcBcdToDec(buf[4] & 0x3f);  // 00111111 1-31
  sDate->Month = DMC_I2cRtcBcdToDec(buf[5] & 0x1f);  // 00011111 1-12
  sDate->Year = DMC_I2cRtcBcdToDec(buf[6]);         // 11111111 0-99
}

// Functions used to communicate with those devices that do not have a secondary address,
// such as a sensors.
// Returns 0 on success (ack), non-0 on failure (nack)
uint8_t DMC_I2cRtcRegisterWrite(uint16_t address, uint8_t *data, uint16_t length)
{
	return HAL_I2C_Master_Transmit(&RtcHi2c2, address, data, length, I2C_TIMEOUT);
}

uint8_t DMC_I2cRtcRegisterRead(uint16_t address, uint8_t *data, uint16_t length)
{
	return HAL_I2C_Master_Receive(&RtcHi2c2, address, data, length, I2C_TIMEOUT);
}

// Functions used to communicate with those devices that have a secondary address,
// such as memory chips (for example AT24Cxxx).
// Returns 0 on success (ack), non-0 on failure (nack)
uint8_t DMC_I2cRtcSramWrite(uint16_t address, uint8_t *data, uint16_t length)
{
	return HAL_I2C_Mem_Write(&RtcHi2c2, _address_RTC, address, 1, data, length, I2C_TIMEOUT);
}

uint8_t DMC_I2cRtcSramRead(uint16_t address, uint8_t *data, uint16_t length)
{
	return HAL_I2C_Mem_Read(&RtcHi2c2, _address_RTC, address, 1, data, length, I2C_TIMEOUT);
}

uint8_t DMC_I2cRtcEepromWrite(uint16_t address, uint8_t *data, uint16_t length)
{
	return HAL_I2C_Mem_Write(&RtcHi2c2, _address_EEPROM, address, 1, data, length, I2C_TIMEOUT);
}

uint8_t DMC_I2cRtcEepromRead(uint16_t address, uint8_t *data, uint16_t length)
{
	return HAL_I2C_Mem_Read(&RtcHi2c2, _address_EEPROM, address, 1, data, length, I2C_TIMEOUT);
}

void DMC_I2cRtcSetFrequency(uint16_t freq)
{
	if (RtcHi2c2.Instance == I2C2)
	{
		MX_I2C2_Init_Frequency_kHz(freq);
	}
}

// get a uint8_t containing just the requested bits
// pass the register address to read, a mask to apply to the register and
// an uint8_t* for the output
// you can test this value directly as TRUE/FALSE for specific bit mask
// of use a mask of 0xff to just return the whole register uint8_t
// returns TRUE to indicate success
/** Get flag
 * @param reg : register address
 * @param mask : flag mask
 * @return The register content
 */
uint8_t DMC_I2cRtcGetFlag(uint8_t reg, uint8_t mask, uint8_t *flag)
{
	uint8_t buf[1];
	buf[0] = reg;
	uint8_t w = DMC_I2cRtcRegisterWrite(_address_RTC, buf, 1);
	uint8_t r = DMC_I2cRtcRegisterRead(_address_RTC, buf, 1);
	_error = ((w != 0) || (r != 0));
	// return only requested flag
	*flag = (buf[0] & mask);
	return flag == 0 ? FALSE : TRUE;
}

// set/clear bits in a uint8_t register, or replace the uint8_t altogether
// pass the register address to modify, a uint8_t to replace the existing
// value with or containing the bits to set/clear and one of
// MCP79412_SET/MCP79412_CLEAR/MCP79412_REPLACE
// returns TRUE to indicate success
/** Set flag
 * @param reg : register address
 * @param bits : bits to set or reset
 * @param mode : MCP79412_REPLACE, MCP79412_SET, MCP79412_CLEAR
 * @return none
 */
void DMC_I2cRtcSetFlag(uint8_t reg, uint8_t bits, uint8_t mode)
{
	uint8_t buf[2];
	buf[0] = reg;
	// get status register
	uint8_t w = DMC_I2cRtcRegisterWrite(_address_RTC, buf, 1);
	uint8_t r = DMC_I2cRtcRegisterRead(_address_RTC, buf + 1, 1);
	// clear the flag
	if (mode == MCP79412_REPLACE)
		buf[1] = bits;
	else if (mode == MCP79412_SET)
		buf[1] |= bits;
	else
		buf[1] &= ~bits;
	uint8_t w2 = DMC_I2cRtcRegisterWrite(_address_RTC, buf, 2);
	_error = ((w != 0) || (r != 0) || (w2 != 0));
}

// read a register
int DMC_I2cRtcReadRegister(uint8_t reg)
{
	uint8_t buf[1];
	buf[0] = reg;
	uint8_t w = DMC_I2cRtcRegisterWrite(_address_RTC, buf, 1);
	uint8_t r = DMC_I2cRtcRegisterRead(_address_RTC, buf, 1);
	_error = ((w != 0) || (r != 0));
	return (buf[0]);
}

// read registers
void DMC_I2cRtcReadRegisters(uint8_t reg, uint8_t *outbuf, uint8_t length)
{
	uint8_t buf[1];
	buf[0] = reg;
	uint8_t w = DMC_I2cRtcRegisterWrite(_address_RTC, buf, 1);
	uint8_t r = DMC_I2cRtcRegisterRead(_address_RTC, outbuf, length);
	_error = ((w != 0) || (r != 0));
}

// write a register
void DMC_I2cRtcWriteRegister(uint8_t reg, uint8_t data)
{
	uint8_t buf[2];
	buf[0] = reg;
	buf[1] = data;
	uint8_t w = DMC_I2cRtcRegisterWrite(_address_RTC, buf, 2);
	_error = (w != 0);
}

// write registers
void DMC_I2cRtcWriteRegisters(uint8_t reg, uint8_t *inbuf, uint8_t length)
{
	uint8_t buf[32];
	buf[0] = reg;
	for (int i = 1; i <= length; i++)
	{
		buf[i] = inbuf[i - 1];
	}
	uint8_t w = DMC_I2cRtcRegisterWrite(_address_RTC, buf, length + 1);
	_error = (w != 0);
}

// Function to read the MAC address from the EEPROM
void DMC_I2cRtcGetMacAddress(uint8_t *mac_address)
{
	uint8_t buf[1];
	buf[0] = MAC_LOCATION;
	uint8_t w = DMC_I2cRtcRegisterWrite(_address_EEPROM, buf, 1);
	uint8_t r = DMC_I2cRtcRegisterRead(_address_EEPROM, mac_address, 6);
	_error = ((w != 0) || (r != 0));
}

// Unlock the unique id area and write in the MAC address to the EEPROM
void DMC_I2cRtcSetMacAddress(uint8_t *mac_address)
{
	uint8_t buf[7];
	DMC_I2cRtcUnlockUniqueID();
	buf[0] = MAC_LOCATION;
	for (int i = 1; i <= 6; i++)
	{
		buf[i] = (char) mac_address[i - 1];
	}
	uint8_t w = DMC_I2cRtcRegisterWrite(_address_EEPROM, buf, 7);

	_error = (w != 0);
}

// Unlock the unique id area ready for writing
void DMC_I2cRtcUnlockUniqueID()
{
	// Write 0x55 to the memory location 0x09
	uint8_t buf[2];
	buf[0] = UNLOCK_ID_REG;
	buf[1] = UNLOCK_ID_CODE1;
	uint8_t w1 = DMC_I2cRtcRegisterWrite(_address_RTC, buf, 2);

	// Write 0xAA to the memory location 0x09
	buf[0] = UNLOCK_ID_REG;
	buf[1] = UNLOCK_ID_CODE2;
	uint8_t w2 = DMC_I2cRtcRegisterWrite(_address_RTC, buf, 2);

	_error = ((w1 != 0) || (w2 != 0));
}

// Set the date/time, set to 24hr and enable the clock
// (assumes you're passing in valid numbers)
//void RTC_SetRtcDateTime(uint8_t second,        // 0-59
//		uint8_t minute,        // 0-59
//		uint8_t hour,          // 1-23
//		uint8_t dayOfMonth,    // 1-31
//		uint8_t month,         // 1-12
//		uint8_t year)          // 0-99

void DMC_I2cRtcSetDateTime(struct DMC_I2C_RTC_DATE_TIME *DateTime)
{
	uint8_t buf[8];
	DateTime->DayOfWeek = DMC_I2cRtcGetDayOfWeek(DateTime->DayOfMonth, DateTime->Month, DateTime->Year);     // 1-7

	uint8_t noOfDaysInMonth = DMC_I2cRtcGetNoOfDaysInMonth((uint16_t) DateTime->Year, DateTime->Month);
	if (DateTime->DayOfMonth > noOfDaysInMonth)
	{
		DateTime->DayOfMonth = noOfDaysInMonth;
	}

//	printf("%d %02d-%02d-20%02d %02d:%02d:%02d\n", dayOfWeek, dayOfMonth, month, year, hour, minute, second);

	buf[0] = RTC_LOCATION;
	buf[1] = DMC_I2cRtcDecToBcd(DateTime->Second) & 0x7f; // set seconds and disable clock (01111111, Bit 7, ST = 0)
	buf[2] = DMC_I2cRtcDecToBcd(DateTime->Minute) & 0x7f;               // set minutes (01111111)
	buf[3] = DMC_I2cRtcDecToBcd(DateTime->Hour) & 0x3f; // set hours and to 24hr format (00111111, Bit 6 = 0)
	buf[4] = _BV(VBATEN) | (DMC_I2cRtcDecToBcd(DateTime->DayOfWeek) & 0x07); // set the day and enable battery backup (00000111)|(00001000, Bit 3 = 1)
	buf[5] = DMC_I2cRtcDecToBcd(DateTime->DayOfMonth) & 0x3f;    // set the date in month (00111111)
	buf[6] = DMC_I2cRtcDecToBcd(DateTime->Month) & 0x1f;                // set the month (00011111)
	buf[7] = DMC_I2cRtcDecToBcd(DateTime->Year);                        // set the year (11111111)
	uint8_t w1 = DMC_I2cRtcRegisterWrite(_address_RTC, buf, 8);

	// Start Clock:
	buf[0] = RTC_LOCATION;
	buf[1] = _BV(ST) | DMC_I2cRtcDecToBcd(DateTime->Second); // set seconds and enable clock (10000000)
	uint8_t w2 = DMC_I2cRtcRegisterWrite(_address_RTC, buf, 2);

	_error = ((w1 != 0) || (w2 != 0));
}

// Get the date/time
//void RTC_GetRtcDateTime(uint8_t *second, uint8_t *minute, uint8_t *hour, uint8_t *dayOfWeek, uint8_t *dayOfMonth,
//		uint8_t *month, uint8_t *year)
void DMC_I2cRtcGetDateTime(struct DMC_I2C_RTC_DATE_TIME *DateTime)
{
	uint8_t buf[8];

	buf[0] = RTC_LOCATION;
	int w = DMC_I2cRtcRegisterWrite(_address_RTC, buf, 1);
	int r = DMC_I2cRtcRegisterRead(_address_RTC, buf, 7);

	_error = ((w != 0) || (r != 0));

	// A few of these need masks because certain bits are control bits
	DateTime->Second = DMC_I2cRtcBcdToDec(buf[0] & 0x7f);  // 01111111 0-59
	DateTime->Minute = DMC_I2cRtcBcdToDec(buf[1] & 0x7f);  // 01111111 0-59
	DateTime->Hour = DMC_I2cRtcBcdToDec(buf[2] & 0x3f);  // 00111111 1-23
//	DateTime->DayOfWeek = RTC_BcdToDec(buf[3] & 0x07);  // 00000111 1-7
	DateTime->DayOfMonth = DMC_I2cRtcBcdToDec(buf[4] & 0x3f);  // 00111111 1-31
	DateTime->Month = DMC_I2cRtcBcdToDec(buf[5] & 0x1f);  // 00011111 1-12
	DateTime->Year = DMC_I2cRtcBcdToDec(buf[6]);         // 11111111 0-99

	DateTime->DayOfWeek = DMC_I2cRtcGetDayOfWeek(DateTime->DayOfMonth, DateTime->Month, DateTime->Year);
	DateTime->DayOfYear = DMC_I2cRtcGetDayOfYear(DateTime->DayOfMonth, DateTime->Month, DateTime->Year);
	DateTime->DaysInMonth = DMC_I2cRtcGetNoOfDaysInMonth(DateTime->Year, DateTime->Month);
	DateTime->DaysInYear = DMC_I2cRtcGetNoOfDaysInYear(DateTime->Year);
}

uint8_t DMC_I2cRtcCheckTimeLost(void)
{
	uint8_t buf[8];
	uint8_t year;
	buf[0] = RTC_LOCATION;
	uint8_t w = DMC_I2cRtcRegisterWrite(_address_RTC, buf, 1);
	uint8_t r = DMC_I2cRtcRegisterRead(_address_RTC, buf, 7);

	_error = ((w != 0) || (r != 0));

	// A few of these need masks because certain bits are control bits
//    second     = bcdToDec(buf[0] & 0x7f);  // 01111111 0-59
//    minute     = bcdToDec(buf[1] & 0x7f);  // 01111111 0-59
//    hour       = bcdToDec(buf[2] & 0x3f);  // 00111111 1-23
//    dayOfWeek  = bcdToDec(buf[3] & 0x07);  // 00000111 1-7
//    dayOfMonth = bcdToDec(buf[4] & 0x3f);  // 00111111 1-31
//    month      = bcdToDec(buf[5] & 0x1f);  // 00011111 1-12
	year = DMC_I2cRtcBcdToDec(buf[6]);         // 11111111 0-99
	return (year <= 15) ? TRUE : FALSE;
}

// Enable the clock without changing the date/time
void DMC_I2cRtcEnableClock()
{
	// Get the current seconds value as the enable/disable bit is in the same
	// byte of memory as the seconds value:
	uint8_t buf[2];
	buf[0] = RTC_LOCATION;
	uint8_t w1 = DMC_I2cRtcRegisterWrite(_address_RTC, buf, 1);
	uint8_t r = DMC_I2cRtcRegisterRead(_address_RTC, buf, 1);

	int second = DMC_I2cRtcBcdToDec(buf[0] & 0x7f);  // 01111111

	// Start Clock:
	buf[0] = RTC_LOCATION;
	buf[1] = _BV(ST) | RTC_DecToBcd(second); // set seconds and enable clock (10000000, Bit 7, ST = 1)
	uint8_t w2 = DMC_I2cRtcRegisterWrite(_address_RTC, buf, 2);

	_error = ((w1 != 0) || (r != 0) || (w2 != 0));
}

// Disable the clock without changing the date/time
void DMC_I2cRtcDisableClock()
{
	// Get the current seconds value as the enable/disable bit is in the same
	// uint8_t of memory as the seconds value:
	uint8_t buf[2];
	buf[0] = RTC_LOCATION;
	uint8_t w1 = DMC_I2cRtcRegisterWrite(_address_RTC, buf, 1);
	uint8_t r = DMC_I2cRtcRegisterRead(_address_RTC, buf, 1);

	int second = DMC_I2cRtcBcdToDec(buf[0] & 0x7f);  // 01111111

	// Stop Clock:
	buf[0] = RTC_LOCATION;
	buf[1] = RTC_DecToBcd(second); // set seconds and disable clock (01111111, Bit 7, ST = 0)
	uint8_t w2 = DMC_I2cRtcRegisterWrite(_address_RTC, buf, 2);

	_error = ((w1 != 0) || (r != 0) || (w2 != 0));
}

// Enable the battery
void DMC_I2cRtcEnableBattery()
{
	// Get the current day value as the enable/disable bit is in the same
	// uint8_t of memory as the seconds value:
	uint8_t buf[2];
	buf[0] = DAY_REG;
	uint8_t w1 = DMC_I2cRtcRegisterWrite(_address_RTC, buf, 1);
	uint8_t r = DMC_I2cRtcRegisterRead(_address_RTC, buf, 1);

	int day = DMC_I2cRtcBcdToDec(buf[0] & 0x07);  // 00000111

	// Start Clock:
	buf[0] = DAY_REG;
	buf[1] = _BV(VBATEN) | DMC_I2cRtcDecToBcd(day); // set day and enable battery (00001000)
	uint8_t w2 = DMC_I2cRtcRegisterWrite(_address_RTC, buf, 2);

	_error = ((w1 != 0) || (r != 0) || (w2 != 0));
}

// 64 Bytes SRAM, Battery Backed
// Write a single byte of data to SRAM
void DMC_I2cRtcWriteSramByte(uint8_t location, uint8_t data)
{
	if (location < SRAM_SIZE)
	{
		DMC_I2cRtcWriteSramBytes(location, &data, 1);
	}
}

// 64 Bytes SRAM, Battery Backed
// Write multiple bytes of data to SRAM
uint8_t DMC_I2cRtcWriteSramBytes(uint8_t location, uint8_t *data, uint8_t length)
{
	uint8_t bytesWritten = 0;
	uint8_t buf[1];
	for (uint8_t i = 0; i < length; i++)
	{
		if (location < SRAM_SIZE)
		{
			buf[0] = data[i];
			uint8_t w = DMC_I2cRtcSramWrite(location + SRAM_START_ADDR, buf, 1); // Returns 0 on success (ack), non-0 on failure (nack)
			bytesWritten++;
			if (_error == FALSE)
			{
				_error = (w != 0);
			}
		}
		location++;
	}
	return bytesWritten;
}

// 64 Bytes SRAM, Battery Backed
// Read a single byte of data from SRAM
uint8_t DMC_I2cRtcReadSramByte(uint8_t location)
{
	uint8_t data = 0;
	if (location < SRAM_SIZE)
	{
		DMC_I2cRtcReadSramBytes(location, &data, 1);
	}
	return data;
}

// 64 Bytes SRAM, Battery Backed
// Read multiple bytes of data from SRAM
uint8_t DMC_I2cRtcReadSramBytes(uint8_t location, uint8_t *data, uint8_t length)
{
	uint8_t bytesRead = 0;
	uint8_t buf[1];
	for (uint8_t i = 0; i < length; i++)
	{
		if (location < SRAM_SIZE)
		{
			uint8_t r = DMC_I2cRtcSramRead(location + SRAM_START_ADDR, buf, 1);
			bytesRead++;
			data[i] = buf[0];
			if (_error == FALSE)
			{
				_error = (r != 0);
			}
		}
		location++;
	}
	return bytesRead;
}

// 128 Bytes EEPROM
// Write a single byte of data to EEPROM
void DMC_I2cRtcWriteEepromByte(uint8_t location, uint8_t data)
{
	if (location <= EEPROM_END_ADDR)
	{
		DMC_I2cRtcWriteEepromBytes(location, &data, 1);
	}
}

// 128 Bytes EEPROM
// Unlock the unique id area and write multiple of bytes to EEPROM
uint8_t DMC_I2cRtcWriteEepromBytes(uint8_t location, uint8_t *data, uint8_t length)
{
	uint8_t bytesWritten = 0;
	uint8_t buf[1];
	for (uint8_t i = 0; i < length; i++)
	{
		if (location + i <= EEPROM_END_ADDR)
		{
			// Skip equal values, to safe EEPROM life
			if (data[i] == DMC_I2cRtcReadEepromByte(location + i))
			{
				continue;
			}
			buf[0] = data[i];
			uint8_t w = DMC_I2cRtcEepromWrite(location + i, buf, 1); // Returns 0 on success (ack), non-0 on failure (nack)
			bytesWritten++;
			if (_error == FALSE)
			{
				_error = (w != 0);
			}
//			DMC_I2cRtcResetSysTick();
//			dmc_puts("waiting\n");
			// Wait until the byte is written
			DMC_I2cRtcWaitEepromWriteIsReady();
//			while (data[i] != DMC_I2cRtcReadEepromByte(location + i));	// Takes about 4mS
//			dmc_putintcr(DMC_I2cRtcGetSysTick());
//			HAL_Delay(5);	// 3 just works, took 5
		}
	}
	return bytesWritten;
}

// 128 Bytes EEPROM
// Read a single byte of data from EEPROM
uint8_t DMC_I2cRtcReadEepromByte(uint8_t location)
{
	uint8_t data = 0;
	if (location <= EEPROM_END_ADDR)
	{
		DMC_I2cRtcReadEepromBytes(location, &data, 1);
	}
	return data;
}

// 128 Bytes EEPROM
// Read multiple bytes of data from EEPROM
uint8_t DMC_I2cRtcReadEepromBytes(uint8_t location, uint8_t *data, uint8_t length)
{
	uint8_t bytesRead = 0;
	uint8_t buf[1];
	for (uint8_t i = 0; i < length; i++)
	{
		if (location <= EEPROM_END_ADDR)
		{
			int r = DMC_I2cRtcEepromRead(location, buf, 1);
			bytesRead++;
			data[i] = buf[0];
			if (_error == FALSE)
			{
				_error = (r != 0);
			}
		}
		location++;
	}
	return bytesRead;
}

void DMC_I2cRtcWaitEepromWriteIsReady(void)
{
	// Datasheet, page 41, "ACKNOWLEDGE POLLING"
	// Since the device will not acknowledge an EEPROM control byte during an internal
	// EEPROM write cycle, this can be used to determine when the cycle is complete.
	uint8_t buf[1];
	// Make sure the loop can't hang for ever
	DMC_I2cRtcResetSysTick();
	// Returns 0 on success (ack), non-0 on failure (nack)
	while(DMC_I2cRtcEepromRead(0x00, buf, 1))
	{
		if (DMC_I2cRtcGetSysTick() > 20)	// 20 mS, should have taken about 4 mS only
		{
			// Error may be reported
		    _Error_Handler(__FILE__, __LINE__);
			break;
		}
	}
}

/*----------------------------------------------------------------------*
 * Read the calibration register.                                       *
 * The calibration value is not a twos-complement number. The MSB is    *
 * the sign bit, and the 7 LSBs are an unsigned number, so we convert   *
 * it and return it to the caller as a regular twos-complement integer. *
 *----------------------------------------------------------------------*/
int DMC_I2cRtcCalibrationRead(void)
{
	uint8_t val = DMC_I2cRtcReadRamByte(CALIB_REG);

	if (val & 0x80)
	{
		return -(val & 0x7F);
	}
	return val;
}

/*----------------------------------------------------------------------*
 * Write the calibration register.                                      *
 * Calibration value must be between -127 and 127, others result        *
 * in no action. See note above on the format of the calibration value. *
 *----------------------------------------------------------------------*/
void DMC_I2cRtcCalibrationWrite(int value)
{
	uint8_t calibVal;

	if ((value >= -127) && (value <= 127))
	{
		calibVal = abs(value);
		if (value < 0)
		{
			calibVal += 128;
		}
		DMC_I2cRtcWriteRamByte(CALIB_REG, calibVal);
	}
}

/*----------------------------------------------------------------------*
 * Read the unique ID.                                                  *
 * User or factory programmable, Protected area                         *
 * For the MCP79411 (EUI-48), the first two bytes will contain 0xFF.    *
 * Caller must provide an 8-byte array to contain the results.          *
 *----------------------------------------------------------------------*/
void DMC_I2cRtcReadUniqueId(uint8_t *uniqueID)
{
	uint8_t buf[1];
	buf[0] = UNIQUE_ID_ADDR;
	uint8_t w = DMC_I2cRtcRegisterWrite(_address_EEPROM, buf, 1);
	uint8_t r = DMC_I2cRtcRegisterRead(_address_EEPROM, uniqueID, UNIQUE_ID_SIZE);
	_error = ((w != 0) || (r != 0));
}

/*----------------------------------------------------------------------------*
 * Returns an EUI-64 ID. For an MCP79411, the EUI-48 ID is converted to       *
 * EUI-64. For an MCP79412, calling this function is equivalent to            *
 * calling readUniqueId(). For an MCP79412, if the RTC type is known, calling *
 * readUniqueId() will be a bit more efficient.                               *
 * Caller must provide an 8-byte array to contain the results.                *
 *----------------------------------------------------------------------------*/
void DMC_I2cRtcGetEUI64(uint8_t *uniqueID)
{
	uint8_t rtcID[8];

	DMC_I2cRtcReadUniqueId(rtcID);
	if ((rtcID[0] == 0xFF) && (rtcID[1] == 0xFF))
	{
		rtcID[0] = rtcID[2];
		rtcID[1] = rtcID[3];
		rtcID[2] = rtcID[4];
		rtcID[3] = 0xFF;
		rtcID[4] = 0xFE;
	}
	for (uint8_t i = 0; i < UNIQUE_ID_SIZE; i++)
	{
		uniqueID[i] = rtcID[i];
	}
}

/*----------------------------------------------------------------------*
 * Check to see if a power failure has occurred. If so, returns TRUE    *
 * as the function value, and returns the power down and power up       *
 * timestamps. After returning the time stamps, the RTC's timestamp     *
 * registers are cleared and the VBAT bit which indicates a power       *
 * failure is reset.                                                    *
 *                                                                      *
 * Note that the power down and power up timestamp registers do not     *
 * contain values for seconds or for the year. The returned time stamps *
 * will therefore contain the current year from the RTC. However, there *
 * is a chance that a power outage spans from one year to the next.     *
 * If we find the power down timestamp to be later (larger) than the    *
 * power up timestamp, we will assume this has happened, and well       *
 * subtract one year from the power down timestamp.                     *
 *                                                                      *
 * Still, there is an assumption that the timestamps are being read     *
 * in the same year as that when the power up occurred.                 *
 *                                                                      *
 * Finally, note that once the RTC records a power outage, it must be   *
 * cleared before another will be recorded.                             *
 *----------------------------------------------------------------------*/
uint8_t DMC_I2cRtcCheckPowerFail(time_t *powerDown, time_t *powerUp)
{
//    uint8_t day, yr;                //copies of the RTC Day and Year registers
//    struct tm dn, up;               //power down and power up times
//    uint8_t buf[8];
//
//    rtc_readRamBytes(DAY_REG, &day, 1);
//    rtc_readRamBytes(YEAR_REG, &yr, 1);
//    yr = y2kYearToTm(bcdToDec(yr));
//    if ( day & _BV(VBAT) ) {
//        buf[0] = PWRDWN_TS_REG;
//        int w = rtc_i2c_write(_address_RTC, buf, 1);
//
//        int r = rtc_i2c_read(_address_RTC, buf, 8);       //read both timestamp registers, 8 bytes total
//        dn.tm_sec     = 0;
//        dn.tm_min     = bcdToDec(buf[0]);
//        dn.tm_hour    = bcdToDec(buf[1] & ~_BV(HR1224));     //assumes 24hr clock
//        dn.tm_mday    = bcdToDec(buf[2]);
//        dn.tm_mon     = bcdToDec(buf[3] & 0x1F);             //mask off the day, we don't need it
//        dn.tm_year    = yr;                                 //assume current year
//        up.tm_sec     = 0;
//        up.tm_min     = bcdToDec(buf[4]);
//        up.tm_hour    = bcdToDec(buf[5] & ~_BV(HR1224));     //assumes 24hr clock
//        up.tm_mday    = bcdToDec(buf[6]);
//        up.tm_mon     = bcdToDec(buf[7] & 0x1F);             //mask off the day, we don't need it
//        up.tm_year    = yr;                                 //assume current year
//
//        _error = ((w != 0) || (r != 0));
//
//        *powerDown = mktime(&dn);
//        *powerUp   = mktime(&up);
//
//        //clear the VBAT bit, which causes the RTC hardware to clear the timestamps too.
//        //I suppose there is a risk here that the day has changed since we read it,
//        //but the Day of Week is actually redundant data and the makeTime() function
//        //does not use it. This could be an issue if someone is reading the RTC
//        //registers directly, but as this library is meant to be used with the Time library,
//        //and also because we don't provide a method to read the RTC clock/calendar
//        //registers directly, we won't lose any sleep about it at this point unless
//        //some issue is actually brought to our attention ;-)
//        day &= ~_BV(VBAT);
//        rtc_writeRamBytes(DAY_REG, &day , 1);
//
//        //adjust the powerDown timestamp if needed (see notes above)
//        if (*powerDown > *powerUp) {
//            --dn.tm_year;
////            *powerDown = mktime(&dn);
//        }
//        return TRUE;
//    }
	return FALSE;
}

/*----------------------------------------------------------------------*
 * Enable or disable the square wave output.                            *
 *----------------------------------------------------------------------*/
void DMC_I2cRtcSquareWave(uint8_t freq)
{
	uint8_t ctrlReg;

	DMC_I2cRtcReadSramBytes(CTRL_REG, &ctrlReg, 1);
	if (freq > 3)
	{
		ctrlReg &= ~_BV(SQWE);
	}
	else
	{
		ctrlReg = (ctrlReg & 0xF8) | _BV(SQWE) | freq;
	}
	DMC_I2cRtcWriteSramByte(CTRL_REG, ctrlReg);
}

/*----------------------------------------------------------------------*
 * Set an alarm time. Sets the alarm registers only, does not enable    *
 * the alarm. See enableAlarm().                                        *
 *----------------------------------------------------------------------*/
void DMC_I2cRtcSetAlarm(uint8_t alarmNumber, time_t alarmTime)
{
//    struct tm *t;
//    uint8_t day;                        // Need to preserve bits in the day (of week) register
//
//    alarmNumber &= 0x01;                // Ensure a valid alarm number
//    rtc_readRamBytes(ALM0_DAY + alarmNumber * (ALM1_REG - ALM0_REG) , &day, 1);
//    t = localtime(&alarmTime);            // Put the time_t into the tm structure

//    uint8_t buf[7];
//    buf[0] = ALM0_REG + alarmNumber * (ALM1_REG - ALM0_REG);
//    buf[1] = dec2bcd(t->tm_sec);
//    buf[2] = dec2bcd(t->tm_min);
//    buf[3] = dec2bcd(t->tm_hour);       // Sets 24 hour format (Bit 6 == 0)
//    buf[4] = (day & 0xF8) + t->tm_wday;
//    buf[5] = dec2bcd(t->tm_mday);
//    buf[6] = dec2bcd(t->tm_mon);
//    int w = rtc_i2c_write(_address_RTC, buf, 7);
//    _error = (w != 0);
}

/*----------------------------------------------------------------------*
 * Enable or disable an alarm, and set the trigger criteria,            *
 * e.g. match only seconds, only minutes, entire time and date, etc.    *
 *----------------------------------------------------------------------*/
void DMC_I2cRtcEnableAlarm(uint8_t alarmNumber, uint8_t alarmType)
{
	uint8_t day;                //alarm day register has config & flag bits
	uint8_t ctrl;               //control register has alarm enable bits

	alarmNumber &= 0x01;        //ensure a valid alarm number
	DMC_I2cRtcReadRamBytes(CTRL_REG, &ctrl, 1);
	if (alarmType < ALM_DISABLE)
	{
		DMC_I2cRtcReadRamBytes(ALM0_DAY + alarmNumber * (ALM1_REG - ALM0_REG), &day, 1);
		day = (day & 0x87) | alarmType << 4; //reset interrupt flag, OR in the config bits
		DMC_I2cRtcWriteRamByte(ALM0_DAY + alarmNumber * (ALM1_REG - ALM0_REG), day);
		ctrl |= _BV(ALM0 + alarmNumber);        //enable the alarm
	}
	else
	{
		ctrl &= ~(_BV(ALM0 + alarmNumber));     //disable the alarm
	}
	DMC_I2cRtcWriteRamByte(CTRL_REG, ctrl);
}

/*----------------------------------------------------------------------*
 * Returns TRUE or FALSE depending on whether the given alarm has been  *
 * triggered, and resets the alarm "interrupt" flag. This is not a real *
 * interrupt, just a bit that's set when an alarm is triggered.         *
 *----------------------------------------------------------------------*/
uint8_t DMC_I2cRtcCheckAlarm(uint8_t alarmNumber)
{
	uint8_t day;                //alarm day register has config & flag bits

	alarmNumber &= 0x01;        //ensure a valid alarm number
	DMC_I2cRtcReadRamBytes( ALM0_DAY + alarmNumber * (ALM1_REG - ALM0_REG), &day, 1);
	if (day & _BV(ALMIF))
	{
		day &= ~_BV(ALMIF);     //turn off the alarm "interrupt" flag
		DMC_I2cRtcWriteRamByte(ALM0_DAY + alarmNumber * (ALM1_REG - ALM0_REG), day);
		return TRUE;
	}
	return FALSE;
}

/*----------------------------------------------------------------------*
 * Sets the logic level on the MFP when it's not being used as a        *
 * square wave or alarm output. The default is HIGH.                    *
 *----------------------------------------------------------------------*/
void DMC_I2cRtcSetOutLevelMPF(uint8_t level)
{
	uint8_t ctrlReg;

	DMC_I2cRtcReadRamBytes(CTRL_REG, &ctrlReg, 1);
	if (level)
		ctrlReg |= _BV(OUT);
	else
		ctrlReg &= ~_BV(OUT);
	DMC_I2cRtcWriteRamByte(CTRL_REG, ctrlReg);
}

/*----------------------------------------------------------------------*
 * Specifies the logic level on the Multi-Function Pin (MFP) when an    *
 * alarm is triggered. The default is LOW. When both alarms are         *
 * active, the two are ORed together to determine the level of the MFP. *
 * With alarm polarity set to LOW (the default), this causes the MFP    *
 * to go low only when BOTH alarms are triggered. With alarm polarity   *
 * set to HIGH, the MFP will go high when EITHER alarm is triggered.    *
 *                                                                      *
 * Note that the state of the MFP is independent of the alarm           *
 * "interrupt" flags, and the alarm() function will indicate when an    *
 * alarm is triggered regardless of the polarity.                       *
 *----------------------------------------------------------------------*/
void DMC_I2cRtcSetAlarmPolarityMPF(uint8_t polarity)
{
	uint8_t alm0Day;

	DMC_I2cRtcReadRamBytes(ALM0_DAY, &alm0Day, 1);
	if (polarity)
		alm0Day |= _BV(OUT);
	else
		alm0Day &= ~_BV(OUT);
	DMC_I2cRtcWriteRamByte(ALM0_DAY, alm0Day);
}

/*----------------------------------------------------------------------*
 * Check to see if the RTC's oscillator is started (ST bit in seconds   *
 * register). Returns TRUE if started.                                  *
 *----------------------------------------------------------------------*/
uint8_t DMC_I2cRtcCheckIsRunning(void)
{
	uint8_t buf[1];
	buf[0] = (uint8_t) TIME_REG;
	uint8_t w = DMC_I2cRtcRegisterWrite(_address_RTC, buf, 1);
	uint8_t r = DMC_I2cRtcRegisterRead(_address_RTC, buf, 1);
	_error = ((w != 0) || (r != 0));
	return buf[0] & _BV(ST);
}

/*----------------------------------------------------------------------*
 * Set or clear the VBATEN bit. Setting the bit powers the clock and    *
 * SRAM from the backup battery when Vcc falls. Note that setting the   *
 * time via set() or rtc_i2c_write() sets the VBATEN bit.                       *
 *----------------------------------------------------------------------*/
void DMC_I2cRtcSetVBATENBit(uint8_t enable)
{
	uint8_t day;

	DMC_I2cRtcReadRamBytes(DAY_REG, &day, 1);
	if (enable)
		day |= _BV(VBATEN);
	else
		day &= ~_BV(VBATEN);

	DMC_I2cRtcWriteRamByte(DAY_REG, day);
	return;
}

//uint8_t getSummerTime(void)
//{
//    getDateTime(&t);
//
//    time_t secondsEpoch = mktime(&t);   // seconds since the Epoch
//    t = *localtime(&secondsEpoch);
//    strftime(buffer, 32, "%j", localtime(&secondsEpoch));
//    int dayOfYearC = atoi(buffer);
//
//    strftime(buffer, 32, "%Y", localtime(&secondsEpoch));
//    int year = atoi(buffer);
//
//    int index  = (year - 2011) * 5;
//    if (index < 0)
//        index = 0;
//    if (index > 440)                    // (2099 - 2011) * 5 = 440
//        index = 440;
//
//    int monthS = atoi(SummerTime[index+1]);
//    int dayS   = atoi(SummerTime[index+2]);
//
//    t.tm_mon   = monthS - 1;            // adjust for tm structure required values
//    t.tm_mday  = dayS;
//    secondsEpoch = mktime(&t);   // seconds since the Epoch
//    t = *localtime(&secondsEpoch);
//    strftime(buffer, 32, "%j", localtime(&secondsEpoch));
//    int dayOfYearS = atoi(buffer);
//
//    int monthE = atoi(SummerTime[index+3]);
//    int dayE   = atoi(SummerTime[index+4]);
//
//    t.tm_mon   = monthE - 1;            // adjust for tm structure required values
//    t.tm_mday  = dayE;
//    secondsEpoch = mktime(&t);   // seconds since the Epoch
//    t = *localtime(&secondsEpoch);
//    strftime(buffer, 32, "%j", localtime(&secondsEpoch));
//    int dayOfYearE = atoi(buffer);
//
//    return ((dayOfYearC >= dayOfYearS) && (dayOfYearC < dayOfYearE)) ? TRUE : FALSE;
//}
//
//int dayOfYearC(void)
//{
//    getDateTime(&t);
//
//    time_t secondsEpoch = mktime(&t);   // seconds since the Epoch
//    strftime(buffer, 32, "%j", localtime(&secondsEpoch));
//    return atoi(buffer);
//}
//
//char * getSunRise(void)
//{
//    return (char*) SunRise[dayOfYearC()];
//}
//
//char * getSunSet(void)
//{
//    return (char*) SunSet[dayOfYearC()];
//}
//
//char * getDayLength(void)
//{
//    return (char*) DayLength[dayOfYearC()];
//}
//
//int getSunRiseMinute(void)
//{
//    int doy = dayOfYearC();
//    int h = atoi(substr((char*)SunRise[doy], 0, 2));
//    int m = atoi(substr((char*)SunRise[doy], 3, 2));
//    return h * 60 + m;
//}
//
//int getSunSetMinute(void)
//{
//    int doy = dayOfYearC();
//    int h = atoi(substr((char*)SunSet[doy], 0, 2));
//    int m = atoi(substr((char*)SunSet[doy], 3, 2));
//    return h * 60 + m;
//}
//
//uint8_t checkSunRise(void)
//{
//    int dayOfWeek, mday, month, year, hours, minutes, seconds;
//    readDateTime(&dayOfWeek, &mday, &month, &year, &hours, &minutes, &seconds);
//
//    int absMinute     = hours * 60 + minutes;
//    int SunRiseMinute = getSunRiseMinute();
//    int SunSetMinute  = getSunSetMinute();
//
//    return ((absMinute >= SunRiseMinute) && (absMinute < SunSetMinute)) ? TRUE : FALSE;
//}

struct tm DMC_I2cRtcSetSystemDateTime(uint8_t second,        // 0-59
		uint8_t minute,        // 0-59
		uint8_t hour,          // 1-23
		uint8_t dayOfMonth,    // 1-31
		uint8_t month,         // 1-12
		uint8_t year)          // 0-99
{
	uint8_t dayOfWeek = 1;          // Will be determined
	// Convert to unix time structure
	t->tm_sec = second;           // 0-59
	t->tm_min = minute;           // 0-59
	t->tm_hour = hour;             // 0-23
	t->tm_wday = dayOfWeek - 1;    // 0-6     (0 = Sunday)
	t->tm_mday = dayOfMonth;       // 1-31
	t->tm_mon = month - 1;        // 0-11
	t->tm_year = year + 100;       // 100-199 year since 1900
	t->tm_isdst = 0;

//    printf("-> Debug %d %02d-%02d-%03d %02d:%02d:%02d\n", t->tm_wday, t->tm_mday, t->tm_mon, t->tm_year, t->tm_hour, t->tm_min, t->tm_sec);

//    secondsEpoch = mktime(t);   // seconds since the Epoch
//    set_time(secondsEpoch);

	// Get weekday 0-6, Sunday as 0 for RTC
//    t = localtime(&secondsEpoch);
//    printf("-> Debug %d %02d-%02d-%03d %02d:%02d:%02d\n", t->tm_wday, t->tm_mday, t->tm_mon, t->tm_year, t->tm_hour, t->tm_min, t->tm_sec);
	return *t;
}

void DMC_I2cRtcGetSystemDateTime(uint8_t *second,          // 0-59
		uint8_t *minute,          // 0-59
		uint8_t *hour,            // 0-23
		uint8_t *dayOfWeek,       // 1-7     (1 = Sunday)
		uint8_t *dayOfMonth,      // 1-31
		uint8_t *month,           // 1-12
		uint8_t *year)            // 0-99 year since 2000
{
//    struct tm *t;
//    time_t secondsEpoch;

	// Get system DateTime
//    secondsEpoch = time(NULL);
//    t = localtime(&secondsEpoch);

	// time/date data
	*second = (t->tm_sec);          // 0-59
	*minute = (t->tm_min);          // 0-59
	*hour = (t->tm_hour);         // 0-23
	*dayOfWeek = (t->tm_wday + 1);     // 1-7     (1 = Sunday)
	*dayOfMonth = (t->tm_mday);         // 1-31
	*month = (t->tm_mon + 1);      // 1-12
	*year = (t->tm_year - 100);   // 0-99 year since 2000
}

void DMC_I2cRtcSetRTCToSystemDateTime(void)
{
//    // Get system DateTime
//    secondsEpoch = time(NULL);
//    t = localtime(&secondsEpoch);
//
//    // Convert from unix time structure
//    uint8_t second     = (t->tm_sec);           // 0-59
//    uint8_t minute     = (t->tm_min);           // 0-59
//    uint8_t hour       = (t->tm_hour);          // 0-23
//    uint8_t dayOfWeek  = (t->tm_wday + 1);      // 1-7     (1 = Sunday)
//    uint8_t dayOfMonth = (t->tm_mday);          // 1-31
//    uint8_t month      = (t->tm_mon + 1);       // 1-12
//    uint8_t year       = (t->tm_year - 100);    // 0-99 year since 2000
//
//    // Set RTC DateTime
//    setRtcDateTime(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
}

void DMC_I2cRtcSetSystemToRTCDateTime(void)
{
//    // Get RTC DateTime
//    getRtcDateTime(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
//
//    // Convert to unix time structure
//    t->tm_sec   = second;           // 0-59
//    t->tm_min   = minute;           // 0-59
//    t->tm_hour  = hour;             // 0-23
//    t->tm_wday  = dayOfWeek - 1;    // 0-6     (0 = Sunday)
//    t->tm_mday  = dayOfMonth;       // 1-31
//    t->tm_mon   = month - 1;        // 0-11
//    t->tm_year  = year + 100;       // 100-199 year since 1900
//    t->tm_isdst = 0;
//
//    // Set system DateTime
//    secondsEpoch = mktime(t);   // seconds since the Epoch
//    set_time(secondsEpoch);
}

void RTC_SetRTCFromTm(struct tm *t)
{
//    // Get system DateTime
//    secondsEpoch = time(NULL);
//    t = localtime(&secondsEpoch);
//
//    // Convert from unix time structure
//    uint8_t second     = (t->tm_sec);           // 0-59
//    uint8_t minute     = (t->tm_min);           // 0-59
//    uint8_t hour       = (t->tm_hour);          // 0-23
//    uint8_t dayOfWeek  = (t->tm_wday + 1);      // 1-7     (1 = Sunday)
//    uint8_t dayOfMonth = (t->tm_mday);          // 1-31
//    uint8_t month      = (t->tm_mon + 1);       // 1-12
//    uint8_t year       = (t->tm_year - 100);    // 0-99 year since 2000
//
////    printf("setRtcFromTm %d %02d-%02d-%03d %02d:%02d:%02d\n", dayOfWeek, dayOfMonth, month, year, hour, minute, second);
//
//    // Set RTC DateTime
//    setRtcDateTime(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
}

struct tm RTC_GetTmFromRTC(void)
{
//    // Get RTC DateTime
//    getRtcDateTime(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
//
//    // Convert to unix time structure
//    t->tm_sec   = second;           // 0-59
//    t->tm_min   = minute;           // 0-59
//    t->tm_hour  = hour;             // 0-23
//    t->tm_wday  = dayOfWeek - 1;    // 0-6     (0 = Sunday)
//    t->tm_mday  = dayOfMonth;       // 1-31
//    t->tm_mon   = month - 1;        // 0-11
//    t->tm_year  = year + 100;       // 100-199 year since 1900
//    t->tm_isdst = 0;
//
	return *t;
//    // Set system DateTime
//    secondsEpoch = mktime(t);   // seconds since the Epoch
//    set_time(secondsEpoch);
}

time_t RTC_GetSecondsEpoch(void)
{
//    secondsEpoch = time(NULL);
	return secondsEpoch;
}

void RTC_SetSecondsEpoch(time_t t)
{
//    secondsEpoch = t;
//    set_time(secondsEpoch);
}

void RTC_GetRtcDateTimeAsTm(void)
{
//    // Get RTC DateTime
//    getRtcDateTime(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
//
//    // Convert to unix time structure
//    t->tm_sec   = second;           // 0-59
//    t->tm_min   = minute;           // 0-59
//    t->tm_hour  = hour;             // 0-23
//    t->tm_wday  = dayOfWeek - 1;    // 0-6     (0 = Sunday)
//    t->tm_mday  = dayOfMonth;       // 1-31
//    t->tm_mon   = month - 1;        // 0-11
//    t->tm_year  = year + 100;       // 100-199 year since 1900
//    t->tm_isdst = 0;
//
//    // Set system DateTime
//    secondsEpoch = mktime(t);   // seconds since the Epoch
//    time_t t = time(secondsEpoch);
//
//    set_time(secondsEpoch);
}

time_t DMC_I2cRtcConvertDateTimeToTimestamp(uint8_t second,        // 0-59
		uint8_t minute,        // 0-59
		uint8_t hour,          // 0-23
		uint8_t dayOfMonth,    // 1-31
		uint8_t month,         // 1-12
		uint8_t year)          // 0-99 year since 2000
{
	time_t seconds = 0;
//    // setup time structure for Wed, 28 Oct 2009 11:35:37
//    struct tm t;
//    t.tm_sec  = second;         // 0-59
//    t.tm_min  = minute;         // 0-59
//    t.tm_hour = hour;           // 0-23
//    t.tm_mday = dayOfMonth;     // 1-31
//    t.tm_mon  = month - 1;      // 0-11
//    t.tm_year = year + 100;     // 100-199 year since 1900
//
////    printf("Debug %d %02d-%02d-%03d %02d:%02d:%02d\n", t.tm_wday, t.tm_mday, t.tm_mon, t.tm_year, t.tm_hour, t.tm_min, t.tm_sec);
//
//    // convert to timestamp and display (1256729737)
//    time_t seconds = mktime(&t);
////    printf("Time as seconds since January 1, 1970 = %d\n", seconds);
//
////    char buffer[32];
////    strftime(buffer, 32, "%a %d-%m-%Y %H:%M:%S\n", localtime(&seconds));
////    printf("Time: %s", buffer);
//
//    // Get weekday Sunday as 0 (0-6) for RTC
//    struct tm *t2;
//    t2 = localtime(&seconds);
////    printf("Debug %d %02d-%02d-%03d %02d:%02d:%02d\n", t.tm_wday, t2->tm_mday, t2->tm_mon, t2->tm_year, t2->tm_hour, t2->tm_min, t2->tm_sec);
////    printf("Weekday %d\n", t2->tm_wday);

	return seconds;
}

//double clamp(double v)
//{
//    const double t = v < 0.0f ? 0.0f : v;
//    return t > 1.0f ? 1.0f : t;
//}

//// BCD to decimal conversion
//int RTC_bcd2dec(int bcd)
//{
//	return (((bcd & 0xF0) >> 4) * 10 + (bcd & 0x0F));
//}
//
//// decimal to BCD conversion
//int RTC_dec2bcd(int dec)
//{
//	return ((dec / 10) * 16 + (dec % 10));
//}

// Convert binary coded decimal to normal decimal numbers:
int DMC_I2cRtcBcdToDec(int bcd)
{
	return (((bcd & 0xF0) >> 4) * 10 + (bcd % 16));
}

// Convert normal decimal numbers to binary coded decimal:
int DMC_I2cRtcDecToBcd(int val)
{
	return ((val / 10 * 16) + (val % 10));
}

uint8_t DMC_I2cRtcGetDayOfWeek(uint8_t d, uint8_t m, uint8_t y)  // y > 1752, 1 <= m <= 12
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

char * DMC_I2cRtcGetDayOfWeekString(uint8_t d, uint8_t m, uint8_t y)  // y > 1752, 1 <= m <= 12
{
	return RtcDriverDaysOfWeek[DMC_I2cRtcGetDayOfWeek(d, m, y)];
}

uint8_t DMC_I2cRtcYearYYYYIsLeap(uint16_t year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

uint8_t DMC_I2cRtcYearIsLeap(uint8_t year)
{
    return (year % 4 == 0);
}

uint16_t DMC_I2cRtcGetDayOfYear(uint16_t year, int month, int day)
{
    static const int days[2][13] = {
        {0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
        {0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}
    };

    if (year > 2000)
	{
		year -= 2000;
	}

    uint8_t leap = DMC_I2cRtcYearIsLeap(year);

    return days[leap][month] + day;
}

uint8_t DMC_I2cRtcGetNoOfDaysInMonth(uint16_t year, uint8_t month)
{
    static const int days[2][13] = {
        {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
        {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
    };

    if (year > 2000)
	{
		year -= 2000;
	}

    uint8_t leap = DMC_I2cRtcYearIsLeap(year);

    return days[leap][month];
}

uint16_t DMC_I2cRtcGetNoOfDaysInYear(uint16_t year)
{
    static const int days[2] = {
		365, 366
    };

    if (year > 2000)
	{
		year -= 2000;
	}

    uint8_t leap = DMC_I2cRtcYearIsLeap(year);

    return days[leap];
}

char* DMC_I2cRtcGetDateTimeString(void)
{
	char tmpStr[5];

	struct DMC_I2C_RTC_DATE_TIME DateTime;
//	DateTime.Year = 18;			// 00-99
//	DateTime.Month = 12;		// 01-12
//	DateTime.DayOfMonth = 31;	// 01-31
//	DateTime.Hour = 23;			// 00-23
//	DateTime.Minute = 59;		// 00-59
//	DateTime.Second = 58;		// 00-59
//	DateTime.DayOfWeek = 0;		// 1-7, 1 = Sun

	DMC_I2cRtcGetDateTime(&DateTime);
	strncpy(RtcDriverDateTimeString, "", 24);

	// Weekday
	strncat(RtcDriverDateTimeString, DMC_I2cRtcGetDayOfWeekString(DateTime.DayOfMonth, DateTime.Month, DateTime.Year), 24);
	// ' '
	strncat(RtcDriverDateTimeString, " ", 24);

	// 2018-11-14 10:40:41
	// year
	dmc_itoa_len_0(2000 + DateTime.Year, tmpStr, 10, 4);		// 2018
	strncat(RtcDriverDateTimeString, tmpStr, 24);
	// '-'
	strncat(RtcDriverDateTimeString, "-", 24);			// -
	// month
	dmc_itoa_len_0(DateTime.Month, tmpStr, 10, 2);			// 11
	strncat(RtcDriverDateTimeString, tmpStr, 24);
	// '-'
	strncat(RtcDriverDateTimeString, "-", 24);			// -
	// dayOfMonth
	dmc_itoa_len_0(DateTime.DayOfMonth, tmpStr, 10, 2);		// 14
	strncat(RtcDriverDateTimeString, tmpStr, 24);
	// ' '
	strncat(RtcDriverDateTimeString, " ", 24);
	// hour
	dmc_itoa_len_0(DateTime.Hour, tmpStr, 10, 2);
	strncat(RtcDriverDateTimeString, tmpStr, 24);	// 10
	// ':'
	strncat(RtcDriverDateTimeString, ":", 24);			// :
	// minute
	dmc_itoa_len_0(DateTime.Minute, tmpStr, 10, 2);
	strncat(RtcDriverDateTimeString, tmpStr, 24);	// 49
	// ':'
	strncat(RtcDriverDateTimeString, ":", 24);			// :
	// second
	dmc_itoa_len_0(DateTime.Second, tmpStr, 10, 2);
	strncat(RtcDriverDateTimeString, tmpStr, 24);	// 41

	return RtcDriverDateTimeString;
}

uint8_t DMC_I2cRtcCompareSramBytes(uint8_t location, uint8_t *compare, uint8_t length)
{
	uint8_t dataArray[SRAM_SIZE];
	DMC_I2cRtcReadSramBytes(location, dataArray, length);

	return memcmp(dataArray, compare, length);
}

uint8_t DMC_I2cRtcCompareEepromBytes(uint8_t location, uint8_t *compare, uint8_t length)
{
	uint8_t dataArray[EEPROM_SIZE];
	DMC_I2cRtcReadEepromBytes(location, dataArray, length);

	return memcmp(dataArray, compare, length);
}

void DMC_I2cRtcEepromWriteRequest(uint8_t location, uint8_t *data, uint8_t length)
{
	DMC_I2cRtcEepromDataLocation = location;
	DMC_I2cRtcEepromDataLength = length;
	memcpy(DMC_I2cRtcEepromDataArray, data, length);

	DMC_I2cRtcEepromWriteResetTickCounter();
	DMC_I2cRtcEepromWriteAllowed = FALSE;
	DMC_I2cRtcEepromWriteRequested = TRUE;
}

void DMC_I2cRtcEepromWriteResetTickCounter(void)
{
	DMC_I2cRtcEepromWriteTickCounter = 0;
	DMC_I2cRtcEepromWriteAllowed = FALSE;
}

void DMC_I2cRtcEepromWriteTick(void)
{
	if (!DMC_I2cRtcEepromWriteRequested)
	{
		return;
	}
//	dmc_putc('.');

	// Called every 1 mSec from HAL_TIM_PeriodElapsedCallback (main.c)
	DMC_I2cRtcEepromWriteTickCounter++;
	if (DMC_I2cRtcEepromWriteTickCounter >= 10000)	// 10 Seconds
	{
		DMC_I2cRtcEepromWriteAllowed = TRUE;
		DMC_I2cRtcEepromWriteRequested = FALSE;
	}
}

void DMC_I2cRtcEepromWritePlanned(void)
{
	if (!DMC_I2cRtcEepromWriteAllowed)
	{
		return;
	}

	dmc_puts("EEPROM Programming...\n");
//	dmc_puts("DMC_I2cRtcEepromDataLocation: ");
//	dmc_puthex2cr(DMC_I2cRtcEepromDataLocation);
//	dmc_puts("DMC_I2cRtcEepromDataLength: ");
//	dmc_puthex2cr(DMC_I2cRtcEepromDataLength);
//	dmc_puts("DMC_I2cRtcEepromDataArray: ");
//	for(uint8_t i = 0; i < DMC_I2cRtcEepromDataLength; i++)
//	{
//		if (i > 0)
//		{
//			dmc_puts(" ");
//		}
//		dmc_puthex2(DMC_I2cRtcEepromDataArray[i]);
//	}
//	dmc_puts("\n");

	DMC_I2cRtcWriteEepromBytes(DMC_I2cRtcEepromDataLocation, DMC_I2cRtcEepromDataArray, DMC_I2cRtcEepromDataLength);

	DMC_I2cRtcEepromWriteAllowed = FALSE;
	DMC_I2cRtcEepromWriteRequested = FALSE;

//	uint8_t dataArray[128];
//	DMC_I2cRtcReadEepromBytes(DMC_I2cRtcEepromDataLocation, dataArray, DMC_I2cRtcEepromDataLength);
//	dmc_puts("dataArray: ");
//	for(uint8_t i = 0; i < DMC_I2cRtcEepromDataLength; i++)
//	{
//		if (i > 0)
//		{
//			dmc_puts(" ");
//		}
//		dmc_puthex2(dataArray[i]);
//	}
//	dmc_puts("\n");
}
