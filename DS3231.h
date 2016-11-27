#ifndef GUARD_DS3231_H
#define GUARD_DS3231_H

#include <stdint.h>
#include <stdbool.h>

#define DS3231_ADDRESS_READ 0b11010001
#define DS3231_ADDRESS_WRITE 0b11010000

#define DS3231_OPERATION_SUCCESS 0 // this is returned if a function ran without errors

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

// alarm 1 trigger interval bits
#define DS3231_ALARM1_A1M1_BIT (1 << 7) // used to signal when an alarm should be triggered (e.g. every second etc.)
#define DS3231_ALARM1_A1M2_BIT (1 << 7) // used to signal when an alarm should be triggered (e.g. every second etc.)
#define DS3231_ALARM1_A1M3_BIT (1 << 7) // used to signal when an alarm should be triggered (e.g. every second etc.)
#define DS3231_ALARM1_A1M4_BIT (1 << 7) // used to signal when an alarm should be triggered (e.g. every second etc.)

// alarm 2 registers
#define DS3231_REGISTER_ALARM2_MINUTES 0xb
#define DS3231_REGISTER_ALARM2_HOURS 0xc
#define DS3231_REGISTER_ALARM2_DAY_DATE 0xd

// alarm 2 trigger interval bits
#define DS3231_ALARM2_A2M2_BIT (1 << 7) // used to signal when an alarm should be triggered (e.g. every second etc.)
#define DS3231_ALARM2_A2M3_BIT (1 << 7)
#define DS3231_ALARM2_A2M4_BIT (1 << 7)

// other alarm bits
#define DS3231_ALARM_DAY_BIT (1 << 6) // used to indicate if an alarm is set for a DAY or DATE

// control register
#define DS3231_REGISTER_CONTROL 0xe

// control register bits
#define DS3231_CONTROL_EOSC_BIT (1 << 7)
#define DS3231_CONTROL_BBQSW_BIT (1 << 6)
#define DS3231_CONTROL_CONV_BIT (1 << 5)
#define DS3231_CONTROL_RS2_BIT (1 << 4)
#define DS3231_CONTROL_RS1_BIT (1 << 3)
#define DS3231_CONTROL_INTCN_BIT (1 << 2) // enable interrupts
#define DS3231_CONTROL_A2IE_BIT (1 << 1) // enable alarm 2
#define DS3231_CONTROL_A1IE_BIT (1 << 0) // enable alarm 1

// status register
#define DS3231_REGISTER_STATUS 0xf

// status register bits
#define DS3231_STATUS_OSF_BIT (1 << 7) // oscillator stop
#define DS3231_STATUS_EN32KHZ_BIT (1 << 3) // enable 32KHz square wave output
#define DS3231_STATUS_BSY_BIT (1 << 2) // device busy bit
#define DS3231_STATUS_A2F_BIT (1 << 1) // alarm 2 triggered flag
#define DS3231_STATUS_A1F_BIT (1 << 0) // alarm 1 triggered flag

// aging register
#define DS3231_REGISTER_AGING_OFFSET 0x10

// temperature registers
#define DS3231_REGISTER_TEMPERATURE_MSB 0x11
#define DS3231_REGISTER_TEMPERATURE_LSB 0x12

// special toggle bits
#define DS3231_HOUR_MODE_12_BIT (1 << 6) // this will be 1 in the HOURS register if 12 hour mode is selected. 0 if 24 hour mode selected
#define DS3231_PM_BIT (1 << 5) // if using 12 hr mode, this bit is set in the HOURS register to indicate if the time is AM or PM with PM being indicated by a 1 and AM by a 0
#define DS3231_20_HOUR_BIT (1 << 5) // if using 24 hr mode, this bit is set in the HOURS register to indicate the hours between 20 and 23
#define DS3231_CENTURY_BIT (1 << 7) // in the month/century register bit 7 is set when a new century is entered


// possible days
typedef enum
{
	SUNDAY = 1, // must start from 1 according to datasheet
	MONDAY = 2,
	TUESDAY = 3,
	WEDNESDAY = 4,
	THURSDAY = 5,
	FRIDAY = 6,
	SATURDAY = 7,
	DAY_T_MAX = 8
} day_t;

// possible months
typedef enum
{
	JANUARY = 1,
	FEBRUARY = 2,
	MARCH = 3,
	APRIL = 4,
	MAY = 5,
	JUNE = 6,
	JULY = 7,
	AUGUST = 8,
	SEPTEMBER = 9,
	OCTOBER = 10,
	NOVEMBER = 11,
	DECEMBER = 12,
	MONTH_T_MAX = 13
} month_t;

// the available alarms of the ds3231
typedef enum
{
	ALARM_1,
	ALARM_2,
	ALARM_NUMBER_T_MAX
} alarm_number_t;

// defines when alarms should be triggered, e.g. when seconds match
typedef enum
{
	//////////////////////////
	// ALARM1 (A1) triggers //
	//////////////////////////
	A1_EVERY_SEC, // triggers every second
	A1_SEC_MATCH, // triggers when seconds value in ALARM1 seconds register match ds3231 seconds register
	A1_MIN_SEC_MATCH, // triggers when seconds & min value the ALARM1 seconds & min registers match ds3231 seconds & min registers
	A1_HOUR_MIN_SEC_MATCH, // triggers when hours, min & seconds value in ALARM1 respective registers match ds3231 hour, min & seconds registers
	A1_DAY_DATE_HOUR_MIN_SEC_MATCH, // triggers when day/date, hours, min & seconds in ALARM1 register match the values in the ds3231 time & date registers
	// depending on what is "useDay" is true in the alarm_t used selects whether the alarm triggers on a day or date

	//////////////////////////
	// ALARM2 (A2) triggers //
	//////////////////////////
	A2_EVERY_MIN, // triggers every minute (at 0 seconds of the new minute)
	A2_MIN_MATCH, // triggers when ALARM2 minute register matches the ds3231 minute register
	A2_HOUR_MIN_MATCH, // triggers when ALARM2 minute & hour register matches the ds3231 minute & hour registers
	A2_DAY_DATE_HOUR_MIN_MATCH, // triggers when ALARM2 day/date, hour & min register match the ds3231 registers
	// depending on what is "useDay" is true in the alarm_t used selects whether the alarm triggers on a day or date
	ALARM_TRIGGER_T_MAX
} alarm_trigger_t;

// used to set alarms
typedef struct
{
	alarm_number_t alarmNumber; // which alarm to set
	uint8_t second; // the seconds value of the alarm, this is only valid for ALARM_1
	uint8_t minute; // minutes value of alarm
	uint8_t hour; // hour value of alarm
	bool useDay; // if TRUE, then the value in the "dayDate" field is interpreted as the day_t to trigger the alarm
				   // e.g. THURSDAY.
				   // if FALSE, then the value in "dayDate" is interpreted as a numerical date value to trigger the alarm
				   // e.g. 12 (the 12th of the month)
	uint8_t dayDate; // the day OR date of the alarm (depends on useDay);
	alarm_trigger_t trigger; // when the alarm will trigger, e.g. on match on mins & seconds
} alarm_t;

// the frequencies the bbsqw can output
typedef enum
{
	HZ_1, // 1 Hz
	KHZ_1_024, // 1.024 KHz
	KHZ_4_096, // 4.096 KHz
	KHZ_8_192, // 8.192 KHz
	BBSQW_FREQUENCY_MAX	
} bbsqw_frequency_t;


////////////////////////////////////////////////////////////////
// Global variables                                           //
////////////////////////////////////////////////////////////////
// used to track the century
static uint8_t century = 0; // year 20xx has a century of 0
// used to indicate the hour storing mode, either AM/PM (12 hour mode) or 24 hour mode
static bool is24HourMode = true;

////////////////////////////////////////////////////////////////
// Function prototypes                                        //
////////////////////////////////////////////////////////////////
void initDS3231(void);

// time setting / getting functions
static void checkCentury(void);
void ds3231Use12HourMode(bool);

uint8_t ds3231SetSecond(uint8_t);
uint8_t ds3231GetSecond(void);

uint8_t ds3231SetMinute(uint8_t);
uint8_t ds3231GetMinute(void);

uint8_t ds3231SetHour(uint8_t, bool);
uint8_t ds3231GetHour(void);

uint8_t ds3231SetDay(day_t);
day_t ds3231GetDay(void);

uint8_t ds3231SetDate(uint8_t);
uint8_t ds3231GetDate(void);

uint8_t ds3231SetMonth(month_t);
month_t ds3231GetMonth(void);

uint8_t ds3231SetYear(uint8_t);
uint8_t ds3231GetYear(void);

void ds3231SetCentury(uint8_t);
uint8_t ds3231GetCentury(void);

uint8_t ds3231SetFullDate(day_t, uint8_t, month_t, uint8_t, uint8_t);
uint8_t ds3231SetTime(uint8_t, uint8_t, uint8_t, bool);

// alarm functions
static uint8_t validateAlarm(alarm_t alarm);
uint8_t ds3231SetAlarm(alarm_t);
uint8_t ds3231ClearAlarmFlag(alarm_number_t);
uint8_t ds3231RemoveAlarm(alarm_number_t);

// temperature functions
void ds3231ForceTemperatureUpdate(void);
uint16_t ds3231GetTemperature(void);

// oscillator functions
uint8_t ds3231DisableOscillatorOnBattery(void);
uint8_t ds3231EnableOscillatorOnBattery(void);
bool ds3231HasOscillatorStopped(void);

// 32KHz output pin functions
uint8_t ds3231Enable32KHzOutput(void);
uint8_t ds3231Disable32KhzOutput(void);

// other functions
uint8_t ds3231EnableBBSQW(bbsqw_frequency_t freq);

// utility functions
uint8_t decToBcd(uint8_t);
uint8_t bcdToDec(uint8_t);

#endif
