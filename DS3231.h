#ifndef GUARD_DS3231_H
#define GUARD_DS3231_H

#include <stdint.h>

#define DS3231_ADDRESS_READ 0b11010001
#define DS3231_ADDRESS_WRITE 0b11010000

// general time keeping registers
#define DS3231_REGISTER_SECONDS 0
#define DS3231_REGISTER_MINUTES 0x1
#define DS3231_REGISTER_HOURS 0x2
#define DS3231_REGISTER_DAY 0x3
#define DS3231_REGISTER_DATE 0x4
#define DS3231_REGISTER_MONTH_CENTURY 0x5
#define DS3231_REGISTER_YEAR 0x6 

// alarm 1 registers
#define DS3231_REGISTER_ALARM1_SECONDS 0x7
#define DS3231_REGISTER_ALARM1_MINUTES 0x8
#define DS3231_REGISTER_ALARM1_HOURS 0x9
#define DS3231_REGISTER_ALARM1_DAY_DATE 0xa

// alarm 2 registers
#define DS3231_REGISTER_ALARM2_MINUTES 0xb
#define DS3231_REGISTER_ALARM2_HOURS 0xc
#define DS3231_REGISTER_ALARM2_DAY_DATE 0xd

// special registers
#define DS3231_REGISTER_CONTROL 0xe
#define DS3231_REGISTER_STATUS 0xf
#define DS3231_REGISTER_AGING_OFFSET 0x10

// temperature registers
#define DS3231_REGISTER_TEMPERATURE_MSB 0x11
#define DS3231_REGISTER_TEMPERATURE_LSB 0x12

// special toggle bits
#define DS3231_HOUR_MODE_12 (1 << 6) // this will be 1 in the HOURS register if 12 hour mode is selected. 0 if 24 hour mode selected
#define DS3231_PM_INDICATOR (1 << 5) // if using 12 hr mode, this bit is set in the HOURS register to indicate if the time is AM or PM with PM being indicated by a 1 and AM by a 0
#define DS3231_20_HOUR_INDICATOR (1 << 5) // if using 24 hr mode, this bit is set in the HOURS register to indicate the hours between 20 and 23
#define DS3231_CENTURY_INDICATOR (1 << 7) // in the month/century register bit 7 is set when a new century is entered

// possible days
typedef enum
{
	SUNDAY = 1, // must start from 1 according to datasheet
	MONDAY = 2,
	TUESDAY = 3,
	WEDNESDAY = 4,
	THURSDAY = 5,
	FRIDAY = 6,
	SATURDAY = 7
} day_t;

// possible months
typedef enum
{
	JAN = 1,
	FEB = 2,
	MAR = 3,
	APR = 4,
	MAY = 5,
	JUN = 6,
	JUL = 7,
	AUG = 8,
	SEP = 9,
	OCT = 10,
	NOV = 11,
	DEC = 12
} month_t;

// for readability
typedef enum
{
	FALSE,
	TRUE
} bool_t;

// used to track the century
static uint8_t century = 0; // year 20xx has a century of 0

void initDS3231(void);

// time setting / getting functions
uint8_t ds3231SetSecond(uint8_t);
uint8_t ds3231GetSecond(void);

uint8_t ds3231SetMinute(uint8_t);
uint8_t ds3231GetMinute(void);

uint8_t ds3231SetHour(bool_t, bool_t, uint8_t);
uint8_t ds3231GetHour(void);

uint8_t ds3231SetDay(day_t);
day_t ds3231GetDay(void);

uint8_t ds3231SetDate(uint8_t);
uint8_t ds3231GetDate(void);

uint8_t ds3231SetMonth(month_t);
month_t ds3231GetMonth(void);

uint8_t ds3231SetYear(uint8_t);
uint8_t ds3231GetYear(void);

uint8_t ds3231GetCentury(void);

// utility functions
uint8_t decToBcd(uint8_t);
uint8_t bcdToDec(uint8_t);

#endif
