#include "DS3231.h"
#include "i2cMaster.h"

/*
   sets up i2c bus
*/
void initDS3231(void)
{
	initI2C();
}

uint8_t ds3231GetCentury(void)
{
	return century;
}

uint8_t ds3231SetYear(uint8_t year)
{
	if(year > 99)
		return 1;

	i2cStart(DS3231_ADDRESS_WRITE); // set register pointer
	i2cWrite(DS3231_REGISTER_YEAR);
	i2cWrite(decToBcd(year));
	i2cStop();

	return 0;
}

uint8_t ds3231GetYear(void)
{
	i2cStart(DS3231_ADDRESS_WRITE); // set register pointer
	i2cWrite(DS3231_REGISTER_YEAR);
	i2cRepeatStart(DS3231_ADDRESS_READ);
	uint8_t year = i2cReadNak();

	i2cStop();
	return bcdToDec(year);
}

uint8_t ds3231SetMonth(month_t month)
{
	i2cStart(DS3231_ADDRESS_WRITE); // set register pointer
	i2cWrite(DS3231_REGISTER_MONTH_CENTURY);
	i2cWrite(decToBcd((uint8_t) month)); // this also sets the century bit to 0
	i2cStop();

	return 0; // not needed
}

month_t ds3231GetMonth(void)
{
	i2cStart(DS3231_ADDRESS_WRITE); // set register pointer
	i2cWrite(DS3231_REGISTER_MONTH_CENTURY);
	i2cRepeatStart(DS3231_ADDRESS_READ);

	uint8_t month = i2cReadNak();
	i2cStop();

	if(month & DS3231_CENTURY_INDICATOR) // entered a new century
	{
		century++;
		month &= ~(DS3231_CENTURY_INDICATOR); // this forces the century bit clear whilst keeping the correct month
		ds3231SetMonth((month_t) month); 
	}

	return (month_t) bcdToDec(month);
}

uint8_t ds3231SetDate(uint8_t date)
{
	i2cStart(DS3231_ADDRESS_WRITE); // set register pointer
	i2cWrite(DS3231_REGISTER_DATE);
	i2cWrite(decToBcd(date));
	i2cStop();

	return 0; // not needed
}

uint8_t ds3231GetDate(void)
{
	i2cStart(DS3231_ADDRESS_WRITE); // set register pointer
	i2cWrite(DS3231_REGISTER_DATE);
	i2cRepeatStart(DS3231_ADDRESS_READ);

	uint8_t date = i2cReadNak();
	i2cStop();

	return bcdToDec(date);
}


uint8_t ds3231SetDay(day_t day)
{
	i2cStart(DS3231_ADDRESS_WRITE); // set register pointer
	i2cWrite(DS3231_REGISTER_DAY);
	i2cWrite(decToBcd((uint8_t) day));
	i2cStop();

	return 0; // not needed
}

day_t ds3231GetDay(void)
{
	i2cStart(DS3231_ADDRESS_WRITE); // set register pointer
	i2cWrite(DS3231_REGISTER_DAY);
	i2cRepeatStart(DS3231_ADDRESS_READ);

	uint8_t day = i2cReadNak();

	i2cStop();
	return (day_t) bcdToDec(day);
}


// isPM should be TRUE only if 12 hour mode is being used and the time is AM
// isPM should be FALSE if 24 hour mode is being used or it is PM and 12 hour mode is used
uint8_t ds3231SetHour(bool_t is12HourMode, bool_t isPM, uint8_t hours)
{
	if(is12HourMode == TRUE && hours > 12) // invalid condition
		return 1;
	if(is12HourMode == FALSE && hours > 23) // invalid condition
		return 2;

	uint8_t hoursValue = 0; // used to build the byte to send
	if(is12HourMode == TRUE) // set special bits for 12 hr mode
	{
		hoursValue |= DS3231_HOUR_MODE_12; // bit 6 indicates the mode
		if(isPM == TRUE)
			hoursValue |= DS3231_PM_INDICATOR;
	}

	hoursValue |= decToBcd(hours);

	i2cStart(DS3231_ADDRESS_WRITE); // set register pointer
	i2cWrite(DS3231_REGISTER_HOURS);

	i2cWrite(hoursValue); // write value
	i2cStop();

	return 0;
}

uint8_t ds3231GetHour(void)
{
	i2cStart(DS3231_ADDRESS_WRITE); // set the register pointer
	i2cWrite(DS3231_REGISTER_HOURS);
	i2cRepeatStart(DS3231_ADDRESS_READ); // now read the hours register

	uint8_t hours = i2cReadNak();
	i2cStop();
	return bcdToDec(hours);
}

uint8_t ds3231SetMinute(uint8_t minutes)
{
	if(minutes > 59) // invalid condition
		return 1;

	i2cStart(DS3231_ADDRESS_WRITE); // set register pointer
	i2cWrite(DS3231_REGISTER_MINUTES);
	i2cWrite(decToBcd(minutes)); // write to register
	i2cStop();

	return 0;
}

uint8_t ds3231GetMinute(void)
{
	i2cStart(DS3231_ADDRESS_WRITE); // set register pointer
	i2cWrite(DS3231_REGISTER_MINUTES);

	i2cRepeatStart(DS3231_ADDRESS_READ);
	uint8_t minutes = i2cReadNak(); // read from register
	i2cStop();

	return bcdToDec(minutes);
}

uint8_t ds3231SetSecond(uint8_t seconds)
{
	if(seconds > 59)
		return 1; // invalid condition

	i2cStart(DS3231_ADDRESS_WRITE); // set register pointer
	i2cWrite(DS3231_REGISTER_SECONDS);
	i2cWrite(decToBcd(seconds)); // write to register
	i2cStop();

	return 0;
}

uint8_t ds3231GetSecond()
{
	i2cStart(DS3231_ADDRESS_WRITE); // set register pointer
	i2cWrite(DS3231_REGISTER_SECONDS);

	i2cRepeatStart(DS3231_ADDRESS_READ);
	uint8_t seconds = i2cReadNak(); // read from register
	i2cStop();

	return bcdToDec(seconds);
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
uint8_t decToBcd(uint8_t val)
{
  return (val / 10 * 16) + (val % 10);
}

// Convert binary coded decimal to normal decimal numbers

uint8_t bcdToDec(uint8_t val)
{
  return (val / 16 * 10) + (val % 16);
}
