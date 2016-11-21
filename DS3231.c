#include "DS3231.h"
#include "i2cMaster.h"

/*
   sets up i2c bus
*/
void initDS3231(void)
{
	initI2C();
}

void ds3231SetAlarm1(day_t day, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second)
{
}

uint8_t ds3231SetTime(uint8_t hour, uint8_t minute, uint8_t second)
{
	uint8_t error = DS3231_OPERATION_SUCCESS;
	error |= ds3231SetHour(hour);
	error |= ds3231SetMinute(minute);
	error |= ds3231SetSecond(second);

	return error == DS3231_OPERATION_SUCCESS ? DS3231_OPERATION_SUCCESS : error;
}

/*
   allows the day, date, month, year and century to be set with a single function call
Param: day -> the desired day the DS3231 will be set to
       date -> the desired date the DS3231 will be set to
       month -> the desired month the DS3231 will be set to
       year -> the desired year the DS3231 will be set to
       century -> the desired century the DS3231 will be set to
Returns: DS3231_OPERATION_SUCCESS (0) if everything worked, otherwise a non-zero error
*/
uint8_t ds3231SetFullDate(day_t day, uint8_t date, month_t month, uint8_t year, uint8_t century)
{
	uint8_t error = DS3231_OPERATION_SUCCESS;
	error |= ds3231SetDay(day);
	error |= ds3231SetDate(date);
	error |= ds3231SetMonth(month);
	error |= ds3231SetYear(year);
	ds3231SetCentury(century);

	return error == DS3231_OPERATION_SUCCESS ? DS3231_OPERATION_SUCCESS : error;
}

static void setRegisterPointer(uint8_t reg)
{
	i2cStart(DS3231_ADDRESS_WRITE);
	i2cWrite(reg);
}

/*
   checks to see if the CENTURY_INDICATOR bit is set in the MONTH register. If it is then a new century has been entered so the currentCentury counter is incremented.
   This function should be called at the start / end of every other function that interacts with the DS3231, otherwise turning a century will be missed. However if it is unlikely that the DS3231 will experience a change in century, this function can be ignored and removed from the rest of the library code 
 */
static void checkCentury(void)
{
	setRegisterPointer(DS3231_REGISTER_MONTH_CENTURY);
	i2cRepeatStart(DS3231_ADDRESS_READ);

	uint8_t month = i2cReadNak();
	i2cStop();
	if(month & DS3231_CENTURY_INDICATOR) // entered a new century
	{
		century++;
		month &= ~(DS3231_CENTURY_INDICATOR); // this forces the century bit clear whilst keeping the correct month
		ds3231SetMonth((month_t) month); 
	}
}

/*
	Returns: the current century of the DS3231, e.g.
		     0 = 20xx, 1 = 21xx etc.
*/
uint8_t ds3231GetCentury(void)
{
	checkCentury();
	return century;
}

/*
   Sets the starting century for the DS3231. The DS3231 doesn't store the century
   itself, so the library handles it
*/
void ds3231SetCentury(uint8_t cent)
{
	century = cent;
}

uint8_t ds3231SetYear(uint8_t year)
{
	if(year > 99)
		return 1;

	checkCentury();

	setRegisterPointer(DS3231_REGISTER_YEAR);
	i2cWrite(decToBcd(year));
	i2cStop();

	return DS3231_OPERATION_SUCCESS;
}

uint8_t ds3231GetYear(void)
{
	checkCentury();

	setRegisterPointer(DS3231_REGISTER_YEAR);
	i2cRepeatStart(DS3231_ADDRESS_READ);
	uint8_t year = i2cReadNak();

	i2cStop();
	return bcdToDec(year);
}

uint8_t ds3231SetMonth(month_t month)
{
	setRegisterPointer(DS3231_REGISTER_MONTH_CENTURY);
	i2cWrite(decToBcd((uint8_t) month)); // this also sets the century bit to 0
	i2cStop();

	return DS3231_OPERATION_SUCCESS;
}

month_t ds3231GetMonth(void)
{
	checkCentury();
	setRegisterPointer(DS3231_REGISTER_MONTH_CENTURY);
	i2cRepeatStart(DS3231_ADDRESS_READ);

	uint8_t month = i2cReadNak();
	i2cStop();

	return (month_t) bcdToDec(month);
}

uint8_t ds3231SetDate(uint8_t date)
{
	checkCentury();
	setRegisterPointer(DS3231_REGISTER_DATE);
	i2cWrite(decToBcd(date));
	i2cStop();

	return DS3231_OPERATION_SUCCESS;
}

uint8_t ds3231GetDate(void)
{
	checkCentury();
	setRegisterPointer(DS3231_REGISTER_DATE);
	i2cRepeatStart(DS3231_ADDRESS_READ);

	uint8_t date = i2cReadNak();
	i2cStop();

	return bcdToDec(date);
}


uint8_t ds3231SetDay(day_t day)
{
	checkCentury();
	setRegisterPointer(DS3231_REGISTER_DAY);
	i2cWrite(decToBcd((uint8_t) day));
	i2cStop();

	return DS3231_OPERATION_SUCCESS;
}

day_t ds3231GetDay(void)
{
	checkCentury();
	setRegisterPointer(DS3231_REGISTER_DAY);
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

	checkCentury();
	uint8_t hoursValue = 0; // used to build the byte to send
	if(is12HourMode == TRUE) // set special bits for 12 hr mode
	{
		hoursValue |= DS3231_HOUR_MODE_12; // bit 6 indicates the mode
		if(isPM == TRUE)
			hoursValue |= DS3231_PM_INDICATOR;
	}

	hoursValue |= decToBcd(hours);

	setRegisterPointer(DS3231_REGISTER_HOURS);
	i2cWrite(hoursValue); // write value
	i2cStop();

	return DS3231_OPERATION_SUCCESS;
}

uint8_t ds3231GetHour(void)
{
	checkCentury();

	setRegisterPointer(DS3231_REGISTER_HOURS);
	i2cRepeatStart(DS3231_ADDRESS_READ); // now read the hours register

	uint8_t hours = i2cReadNak();
	i2cStop();
	return bcdToDec(hours);
}

uint8_t ds3231SetMinute(uint8_t minutes)
{
	if(minutes > 59) // invalid condition
		return 1;

	checkCentury();

	setRegisterPointer(DS3231_REGISTER_MINUTES);
	i2cWrite(decToBcd(minutes)); // write to register
	i2cStop();

	return DS3231_OPERATION_SUCCESS;
}

uint8_t ds3231GetMinute(void)
{
	checkCentury();

	setRegisterPointer(DS3231_REGISTER_MINUTES);

	i2cRepeatStart(DS3231_ADDRESS_READ);
	uint8_t minutes = i2cReadNak(); // read from register
	i2cStop();

	return bcdToDec(minutes);
}

uint8_t ds3231SetSecond(uint8_t seconds)
{
	if(seconds > 59)
		return 1; // invalid condition

	checkCentury();

	setRegisterPointer(DS3231_REGISTER_SECONDS);
	i2cWrite(decToBcd(seconds)); // write to register
	i2cStop();

	return DS3231_OPERATION_SUCCESS;
}

uint8_t ds3231GetSecond()
{
	checkCentury();

	setRegisterPointer(DS3231_REGISTER_SECONDS);

	i2cRepeatStart(DS3231_ADDRESS_READ);
	uint8_t seconds = i2cReadNak(); // read from register
	i2cStop();

	return bcdToDec(seconds);
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

/*
   used to convert normal decimal numbers to BCD numbers
	Param: val -> the decimal value
	Returns: the BCD representation of val
*/
uint8_t decToBcd(uint8_t val)
{
  return (val / 10 * 16) + (val % 10);
}

/*
   used to convery binary coded decimal to standard
   decimal numbers
	Param: val -> the BCD value
	Returns: the standard decimal representation of val
*/
uint8_t bcdToDec(uint8_t val)
{
  return (val / 16 * 10) + (val % 16);
}
