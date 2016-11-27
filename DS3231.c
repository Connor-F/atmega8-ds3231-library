#include "DS3231.h"
#include "i2cMaster.h"

/*
   sets up i2c bus and resets any necessary flags. MUST be called before using 
   the ds3231
*/
void initDS3231(void)
{
	initI2C();

	// clear any alarms
	ds3231RemoveAlarm(ALARM_1);
	ds3231RemoveAlarm(ALARM_2);
}

/*
   allows for alarms to be cleared/removed/deleted from the ds3231. This function clears
   the appropriate alarms registers, clears the alarm flag and clears the alarm enable bit.
   Removing an alarm permantely deletes the alarm, unlike the `ds3231ClearAlarmFlag` function 
   which just clears the alarm triggered flag but keeps the alarm
Param: alarm -> the alarm number to clear, e.g. ALARM_2
Returns: DS3231_OPERATION_SUCCESS (0) if everything was ok
		 1 if an invalid alarm number was provided
 */
uint8_t ds3231RemoveAlarm(alarm_number_t alarm)
{
	if(alarm < 0 || alarm >= ALARM_NUMBER_T_MAX)
		return 1;

	// assume alarm 1, if alarm 2 this data gets changed
	uint8_t minutesReg = DS3231_REGISTER_ALARM1_MINUTES;
	uint8_t hoursReg = DS3231_REGISTER_ALARM1_HOURS;
	uint8_t dayDateReg = DS3231_REGISTER_ALARM1_DAY_DATE;
	uint8_t enableInterruptFlag = DS3231_CONTROL_A1IE_BIT;
	uint8_t alarmFlag = DS3231_STATUS_A1F_BIT;

	if(alarm == ALARM_2)
	{
		minutesReg = DS3231_REGISTER_ALARM2_MINUTES;
		hoursReg = DS3231_REGISTER_ALARM2_HOURS;
		dayDateReg = DS3231_REGISTER_ALARM2_DAY_DATE;
		enableInterruptFlag = DS3231_CONTROL_A2IE_BIT;
		alarmFlag = DS3231_STATUS_A2F_BIT;
	}

	if(alarm == ALARM_1) // only alarm1 has seconds register
	{
		i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_ALARM1_SECONDS);
		i2cWriteThenStop(0);
	}

	// clear mins, hours and day/date alarm registers
	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, minutesReg);
	i2cWriteThenStop(0);
	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, hoursReg);
	i2cWriteThenStop(0);
	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, dayDateReg);
	i2cWriteThenStop(0);

	// disable interrupts for alarm
	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_CONTROL);
	i2cRepeatStart(DS3231_ADDRESS_READ);
	uint8_t controlReg = i2cReadNak();
	i2cStop();
	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_CONTROL);
	i2cWriteThenStop(controlReg & ~enableInterruptFlag);

	// clear alarm flag
	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_STATUS);
	i2cRepeatStart(DS3231_ADDRESS_READ);
	uint8_t statusReg = i2cReadNak();
	i2cStop();
	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_STATUS);
	i2cWriteThenStop(statusReg & ~alarmFlag);

	return DS3231_OPERATION_SUCCESS;
}

/*
   sets the global hour mode for the ds3231. The ds3231 offers 2 modes
   for storing the hours value in the timekeeping registers and alarm registers,
   these are AM/PM mode (uses 12 hours and a AM/PM indicator bit) and 24 hour mode.

   This should be called before any other if needing to change the mode to AM/PM, as
   24 hour mode is selected by default
*/
void ds3231Use12HourMode(bool use12HourMode)
{
	is24HourMode = !use12HourMode;
}

/*
   ensures the alarm_t struct passed to set an alarm contains valid combinations of values 
		Param: alarm -> pointer to the alarm the user supplied to be passed to the ds3231
		Returns: DS3231_OPERATION_SUCCESS (0) if the alarm was valid
	    1 if an invalid alarmNumber was used
		2 if an invalid trigger was used
		3 if an invalid second value was used with ALARM1 and A1_SEC_MATCH
		4 if an invalid second or minute value was used with ALARM1 and A1_MIN_SEC_MATCH
		5 if an invalid second or minute or hour value was used with ALARM1 and A1_HOUR_MIN_SEC_MATCH
		6 if an invalid second or minute or hour or day/date value was used with ALARM1 and A1_DAY_DATE_HOUR_MIN_SEC_MATCH
		7 if an invalid minute was used with ALARM2 and A2_MIN_MATCH
		8 if an invalid minute or hour was used with ALARM2 and A2_HOUR_MIN_MATCH 
		9 if an invalid minute or hour or day/date was used with ALARM2 and A2_DAY_DATE_HOUR_MIN_MATCH
		10 unknown error occurred processing alarm 1
		11 unknown error occurred processing alarm 2
		12 unknown error occurred after handling alarm 1 or 2
*/
static uint8_t validateAlarm(const alarm_t *alarm)
{
	// invalid alarm number
	if(alarm->alarmNumber < 0 || alarm->alarmNumber >= ALARM_NUMBER_T_MAX)
		return 1;
	// invalid trigger
	if(alarm->trigger < 0 || alarm->trigger >= ALARM_TRIGGER_T_MAX)
		return 2;


	if(alarm->alarmNumber == ALARM_1)
	{
		switch(alarm->trigger)
		{
			case A1_EVERY_SEC: // no values are needed in this case
				return DS3231_OPERATION_SUCCESS;

			case A1_SEC_MATCH:
				if(alarm->second < 60)
					return DS3231_OPERATION_SUCCESS;
				return 3;

			case A1_MIN_SEC_MATCH:
				if(alarm->second < 60 && alarm->minute < 60)
					return DS3231_OPERATION_SUCCESS;
				return 4;

			case A1_HOUR_MIN_SEC_MATCH:
				if(alarm->second < 60 && alarm->minute < 60)
					if((is24HourMode && alarm->hour < 24) || (!is24HourMode && alarm->hour < 13))
						return DS3231_OPERATION_SUCCESS;
				return 5;

			case A1_DAY_DATE_HOUR_MIN_SEC_MATCH:
				if(alarm->second < 60 && alarm->minute < 60)
				{
					if((is24HourMode && alarm->hour < 24) || (!is24HourMode && alarm->hour < 13))
					{
						if(alarm->useDay && (alarm->dayDate > 0 && alarm->dayDate < DAY_T_MAX))
						{
							return DS3231_OPERATION_SUCCESS;
						}
						else if(!alarm->useDay && (alarm->dayDate < 32))
						{
							return DS3231_OPERATION_SUCCESS;
						}
					}
				}
				return 6;

			default: 
				return 10;
		}
	}
	else // check alarm 2 combinations
	{
		switch(alarm->trigger)
		{
			case A2_EVERY_MIN: // no values needed in this case
				return DS3231_OPERATION_SUCCESS;

			case A2_MIN_MATCH:
				if(alarm->minute < 60)
					return DS3231_OPERATION_SUCCESS;
				return 7;

			case A2_HOUR_MIN_MATCH:
				if(alarm->minute < 60)
					if((is24HourMode && alarm->hour < 24) || (!is24HourMode && alarm->hour < 13))
						return DS3231_OPERATION_SUCCESS;
				return 8;

			case A2_DAY_DATE_HOUR_MIN_MATCH:
				if(alarm->minute < 60)
				{
					if((is24HourMode && alarm->hour < 24) || (!is24HourMode && alarm->hour < 13))
					{
						if(alarm->useDay && (alarm->dayDate > 0 && alarm->dayDate < DAY_T_MAX))
						{
							return DS3231_OPERATION_SUCCESS;
						}
						else if(!alarm->useDay && (alarm->dayDate < 32))
						{
							return DS3231_OPERATION_SUCCESS;
						}
					}
				}
				return 9;

			default:
				return 11;
		}
	}

	return 12;
}

/*
   sets an alarm on the ds3231. Also ensures INTCN and A1IE / A2IE is set so alarms will function
		Param: alarm -> pointer to the alarm struct that contains all the info needed to 
		set the alarm
		Returns: DS3231_OPERATION_SUCCESS (0) if the alarm was valid
	    1 if an invalid alarmNumber was used
		2 if an invalid trigger was used
		3 if an invalid second value was used with ALARM1 and A1_SEC_MATCH
		4 if an invalid second or minute value was used with ALARM1 and A1_MIN_SEC_MATCH
		5 if an invalid second or minute or hour value was used with ALARM1 and A1_HOUR_MIN_SEC_MATCH
		6 if an invalid second or minute or hour or day/date value was used with ALARM1 and A1_DAY_DATE_HOUR_MIN_SEC_MATCH
		7 if an invalid minute was used with ALARM2 and A2_MIN_MATCH
		8 if an invalid minute or hour was used with ALARM2 and A2_HOUR_MIN_MATCH 
		9 if an invalid minute or hour or day/date was used with ALARM2 and A2_DAY_DATE_HOUR_MIN_MATCH
		10 unknown error occurred processing alarm 1
		11 unknown error occurred processing alarm 2
		12 unknown error occurred after handling alarm 1 or 2
*/
uint8_t ds3231SetAlarm(const alarm_t *alarm)
{
	uint8_t error = validateAlarm(alarm);
	if(error)
		return error;

	// enable alarm interrupts
	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_CONTROL);
	i2cRepeatStart(DS3231_ADDRESS_READ);
	uint8_t controlReg = i2cReadNak();
	i2cStop();

	// ensure INTCN is set for alarms to trigger an interrupt on INTCN/SQW pin
	// ensure A1IE / A2IE is enabled for alarm1/2 interrupts
	controlReg |= DS3231_CONTROL_INTCN_BIT;
	if(alarm->alarmNumber == ALARM_1)
		controlReg |= DS3231_CONTROL_A1IE_BIT;
	else
		controlReg |= DS3231_CONTROL_A2IE_BIT;
	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_CONTROL);
	i2cWriteThenStop(controlReg);

	if(alarm->alarmNumber == ALARM_1)
	{
		switch(alarm->trigger)
		{
			case A1_EVERY_SEC: // needs a1m1, a1m2, a1m3 and a1m4 all set
				// set a1m1
				i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_ALARM1_SECONDS);
				i2cWriteThenStop(decToBcd(DS3231_ALARM1_A1M1_BIT));
				// set a1m2
				i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_ALARM1_MINUTES);
				i2cWriteThenStop(decToBcd(DS3231_ALARM1_A1M2_BIT));
				// set a1m3
				i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_ALARM1_HOURS);
				i2cWriteThenStop(decToBcd(DS3231_ALARM1_A1M3_BIT));
				// set a1m4
				i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_ALARM1_DAY_DATE);
				i2cWriteThenStop(decToBcd(DS3231_ALARM1_A1M4_BIT));
				break;

			case A1_SEC_MATCH: // needs a1m2, a1m3 and a1m4 set
				// set seconds
				i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_ALARM1_SECONDS);
				i2cWriteThenStop(decToBcd(alarm->second));
				// set a1m2
				i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_ALARM1_MINUTES);
				i2cWriteThenStop(decToBcd(DS3231_ALARM1_A1M2_BIT));
				// set a1m3
				i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_ALARM1_HOURS);
				i2cWriteThenStop(decToBcd(DS3231_ALARM1_A1M3_BIT));
				// set a1m4
				i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_ALARM1_DAY_DATE);
				i2cWriteThenStop(decToBcd(DS3231_ALARM1_A1M4_BIT));
				break;

			case A1_MIN_SEC_MATCH: // needs a1m3 and a1m4 set
				// set seconds
				i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_ALARM1_SECONDS);
				i2cWriteThenStop(decToBcd(alarm->second));
				// set mins
				i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_ALARM1_MINUTES);
				i2cWriteThenStop(decToBcd(alarm->minute));
				// set a1m3
				i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_ALARM1_HOURS);
				i2cWriteThenStop(decToBcd(DS3231_ALARM1_A1M3_BIT));
				// set a1m4
				i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_ALARM1_DAY_DATE);
				i2cWriteThenStop(decToBcd(DS3231_ALARM1_A1M4_BIT));
				break;

			case A1_HOUR_MIN_SEC_MATCH: // needs a1m4 set
				// set seconds
				i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_ALARM1_SECONDS);
				i2cWriteThenStop(decToBcd(alarm->second));
				// set mins
				i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_ALARM1_MINUTES);
				i2cWriteThenStop(decToBcd(alarm->minute));
				// set hours
				i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_ALARM1_HOURS);
				i2cWriteThenStop(decToBcd(alarm->hour));
				// set a1m4
				i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_ALARM1_DAY_DATE);
				i2cWriteThenStop(decToBcd(DS3231_ALARM1_A1M4_BIT));
				break;

			case A1_DAY_DATE_HOUR_MIN_SEC_MATCH: // no a1m* bits needed
				// set seconds
				i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_ALARM1_SECONDS);
				i2cWriteThenStop(decToBcd(alarm->second));
				// set mins
				i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_ALARM1_MINUTES);
				i2cWriteThenStop(decToBcd(alarm->minute));
				// set hours
				i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_ALARM1_HOURS);
				i2cWriteThenStop(decToBcd(alarm->hour));

				i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_ALARM1_DAY_DATE);
				// set day/date
				if(alarm->useDay)
					i2cWriteThenStop(alarm->dayDate | DS3231_ALARM_DAY_BIT);
				else
					i2cWriteThenStop(decToBcd(alarm->dayDate));

				break;

			default: // this should never be reached, here to stop compiler warnings
				break;
		}
	}
	else // alarm 2
	{
		switch(alarm->trigger)
		{
			case A2_EVERY_MIN: // needs a2m2, a2m3 and a2m4 set
				// set a2m2
				i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_ALARM2_MINUTES);
				i2cWriteThenStop(decToBcd(DS3231_ALARM2_A2M2_BIT));
				// set a2m3
				i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_ALARM2_HOURS);
				i2cWriteThenStop(decToBcd(DS3231_ALARM2_A2M3_BIT));
				// set a2m4
				i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_ALARM2_DAY_DATE);
				i2cWriteThenStop(decToBcd(DS3231_ALARM2_A2M4_BIT));
				break;

			case A2_MIN_MATCH: // a2m3 and a2m4 set
				// set a2m3
				i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_ALARM2_HOURS);
				i2cWriteThenStop(decToBcd(DS3231_ALARM2_A2M3_BIT));
				// set a2m4
				i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_ALARM2_DAY_DATE);
				i2cWriteThenStop(decToBcd(DS3231_ALARM2_A2M4_BIT));
				// set minutes
				i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_ALARM2_MINUTES);
				i2cWriteThenStop(decToBcd(alarm->minute));
				break;

			case A2_HOUR_MIN_MATCH: // a2m4 set
				// set a2m4
				i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_ALARM2_DAY_DATE);
				i2cWriteThenStop(decToBcd(DS3231_ALARM2_A2M4_BIT));
				// set minutes
				i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_ALARM2_MINUTES);
				i2cWriteThenStop(decToBcd(alarm->minute));
				// set hours
				i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_ALARM2_HOURS);
				i2cWriteThenStop(decToBcd(alarm->hour));
				break;

			case A2_DAY_DATE_HOUR_MIN_MATCH: // no a2m* bits needed, does need day/date bit set
				// set minutes
				i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_ALARM2_MINUTES);
				i2cWriteThenStop(decToBcd(alarm->minute));
				// set hours
				i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_ALARM2_HOURS);
				i2cWriteThenStop(decToBcd(alarm->hour));

				i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_ALARM2_DAY_DATE);
				if(alarm->useDay)
					i2cWriteThenStop(alarm->dayDate | DS3231_ALARM_DAY_BIT);
				else
					i2cWriteThenStop(decToBcd(alarm->dayDate));

				break;

			default: // this should never be reached, here to stop compiler warnings
				break;
		}
	}

	ds3231ClearAlarmFlag(alarm->alarmNumber);
	return DS3231_OPERATION_SUCCESS;
}

/*
   used to reset an alarms flag (that indicates the alarm was triggered). This function does
   NOT delete the alarm, to delete an alarm see the `ds3231RemoveAlarm` function
		Param: alarm -> the alarm number flag to be reset, e.g. ALARM_1
		Returns: DS3231_OPERATION_SUCCESS (0)
*/
uint8_t ds3231ClearAlarmFlag(alarm_number_t alarm)
{
	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_STATUS);
	i2cRepeatStart(DS3231_ADDRESS_READ);
	uint8_t statusReg = i2cReadNak();
	i2cStop();

	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_STATUS);
	if(alarm == ALARM_1)
		i2cWriteThenStop(statusReg & ~(DS3231_STATUS_A1F_BIT));
	else 
		i2cWriteThenStop(statusReg & ~(DS3231_STATUS_A2F_BIT));

	return DS3231_OPERATION_SUCCESS;
}

/*
   convenience function to set the ds3231 time using a single function
Param: hour -> the hour to set the ds3231 to
	   minute -> the minute to set the ds3231 to
	   second -> the second to set the ds3231 to
	   isPM -> used to indicate whether the hours value should be treated as a PM value
	   		   this should only be true if 12 hour AM/PM mode is activated and it is a PM value. 
			   By default 12 hour AM/PM mode is NOT enabled
*/
uint8_t ds3231SetTime(uint8_t hour, uint8_t minute, uint8_t second, bool isPM)
{
	uint8_t error = DS3231_OPERATION_SUCCESS;
	error |= ds3231SetHour(hour, isPM); // fix
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

/*
   checks to see if the CENTURY_BIT bit is set in the MONTH register. If it is then a new century has been entered so the currentCentury counter is incremented.
   This function should be called at the start / end of every other function that interacts with the DS3231, otherwise turning a century will be missed. However if it is unlikely that the DS3231 will experience a change in century, this function can be ignored and removed from the rest of the library code 
 */
static void checkCentury(void)
{
	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_MONTH_CENTURY);
	i2cRepeatStart(DS3231_ADDRESS_READ);

	uint8_t month = i2cReadNak();
	i2cStop();
	if(month & DS3231_CENTURY_BIT) // entered a new century
	{
		century++;
		month &= ~(DS3231_CENTURY_BIT); // this forces the century bit clear whilst keeping the correct month
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
uint8_t ds3231SetCentury(uint8_t cent)
{
	century = cent;
}

/*
   allows the year to be set on the ds3231
	Param: the year to set on the ds3231
	Returns: DS3231_OPERATION_SUCCESS (0) on success
			 1 if the year is invalid (> 99)
*/
uint8_t ds3231SetYear(uint8_t year)
{
	if(year > 99)
		return 1;

	checkCentury();

	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_YEAR);
	i2cWriteThenStop(decToBcd(year));

	return DS3231_OPERATION_SUCCESS;
}

/*
   allows the retreival of the year held by the ds3231
	Returns: the year held by the ds3231
*/
uint8_t ds3231GetYear(void)
{
	checkCentury();

	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_YEAR);
	i2cRepeatStart(DS3231_ADDRESS_READ);
	uint8_t year = i2cReadNak();

	i2cStop();
	return bcdToDec(year);
}

/*
   sets the month on the ds3231
	Param: month -> the month to set the ds3231 to
	Returns: DS3231_OPERATION_SUCCESS (0) on success
		     1 if the month provided was out of range
*/
uint8_t ds3231SetMonth(month_t month)
{
	if(month < 0 || month >= MONTH_T_MAX)
		return 1;

	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_MONTH_CENTURY);
	i2cWriteThenStop(decToBcd((uint8_t) month)); // this also sets the century bit to 0

	return DS3231_OPERATION_SUCCESS;
}

/*
   allows the retreival of the current date held by the ds3231
	Returns: the month held by the ds3231
*/
month_t ds3231GetMonth(void)
{
	checkCentury();
	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_MONTH_CENTURY);
	i2cRepeatStart(DS3231_ADDRESS_READ);

	uint8_t month = i2cReadNak();
	i2cStop();

	return (month_t) bcdToDec(month);
}

/*
   allows the date to be set on the ds3231
	Param: date -> the date to set the ds3231 to
	Returns: DS3231_OPERATION_SUCCESS (0) on success
			 1 if the date provided was out of range (> 31)
*/
uint8_t ds3231SetDate(uint8_t date)
{
	if(date > 31)
		return 1;

	checkCentury();
	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_DATE);
	i2cWriteThenStop(decToBcd(date));

	return DS3231_OPERATION_SUCCESS;
}

/*
   allows the date to be retreived from the ds3231
	Returns: the date held by the ds3231
*/
uint8_t ds3231GetDate(void)
{
	checkCentury();
	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_DATE);
	i2cRepeatStart(DS3231_ADDRESS_READ);

	uint8_t date = i2cReadNak();
	i2cStop();

	return bcdToDec(date);
}

/*
   allows the day value to be set for the ds3231
	Param: day -> the day to set the ds3231 to
	Returns: DS3231_OPERATION_SUCCESS (0) on success
			 1 if the day provided was invalid
 */
uint8_t ds3231SetDay(day_t day)
{
	if(day < 0 || day >= DAY_T_MAX)
		return 1;

	checkCentury();
	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_DAY);
	i2cWriteThenStop(decToBcd((uint8_t) day));

	return DS3231_OPERATION_SUCCESS;
}

/*
   allows the ds3231 day value to be retreived
	Returns: the current day value of the ds3231
*/
day_t ds3231GetDay(void)
{
	checkCentury();
	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_DAY);
	i2cRepeatStart(DS3231_ADDRESS_READ);

	uint8_t day = i2cReadNak();

	i2cStop();
	return (day_t) bcdToDec(day);
}


/*
   sets the hours register on the ds3231
   Param: hours -> the hours value to set the ds3231 to
   		  isPM -> if using 12 hour mode isPM states whether the hours value
		  		  represents a PM time (if true) or AM time (if false).
				  isPM is ignored if 24 hour mode is being used
   Return: DS3231_OPERATION_SUCCESS (0) if setting the hours worked
   		   1 if an invalid hours value was supplied for 24 hour mode
   		   2 if an invalid hours value was supplied for 12 hour mode
*/
uint8_t ds3231SetHour(uint8_t hours, bool isPM)
{
	if(is24HourMode && hours > 23)
		return 1;
	if(!is24HourMode && hours > 12)
		return 2;

	checkCentury();
	uint8_t hoursValue = 0; // used to build the byte to send
	if(!is24HourMode) // set special bits for 12 hr mode
	{
		hoursValue |= DS3231_HOUR_MODE_12_BIT; // bit 6 indicates the mode
		if(isPM)
			hoursValue |= DS3231_PM_BIT;
	}

	hoursValue |= decToBcd(hours);

	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_HOURS);
	i2cWriteThenStop(hoursValue); // write value

	return DS3231_OPERATION_SUCCESS;
}

/*
   allows the ds3231 hour value to be retreived
	Returns: the hours value the ds3231 has currently stored
*/
uint8_t ds3231GetHour(void)
{
	checkCentury();

	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_HOURS);
	i2cRepeatStart(DS3231_ADDRESS_READ); // now read the hours register

	uint8_t hours = i2cReadNak();
	i2cStop();
	return bcdToDec(hours);
}

/*
   allows the minutes value of the ds3231 to be set
	Param: minutes -> the minutes value to set on the ds3231
	Returns: DS3231_OPERATION_SUCCESS (0) on success
			 1 if the minutes value was invalid (> 59)
*/
uint8_t ds3231SetMinute(uint8_t minutes)
{
	if(minutes > 59) // invalid condition
		return 1;

	checkCentury();

	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_MINUTES);
	i2cWriteThenStop(decToBcd(minutes)); // write to register

	return DS3231_OPERATION_SUCCESS;
}

/*
   allows the minutes value of the ds3231 to be returned
	Returns: the minutes value held by the ds3231
*/
uint8_t ds3231GetMinute(void)
{
	checkCentury();

	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_MINUTES);

	i2cRepeatStart(DS3231_ADDRESS_READ);
	uint8_t minutes = i2cReadNak(); // read from register
	i2cStop();

	return bcdToDec(minutes);
}

/*
   allows the seconds value of the ds3231 to be set
	Param: seconds -> the seconds value to pass to the ds3231
	Returns: DS3231_OPERATION_SUCCESS (0) on success
			 1 if the seconds value was invalid (> 59)
 */
uint8_t ds3231SetSecond(uint8_t seconds)
{
	if(seconds > 59)
		return 1; // invalid condition

	checkCentury();

	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_SECONDS);
	i2cWriteThenStop(decToBcd(seconds)); // write to register

	return DS3231_OPERATION_SUCCESS;
}

/* 
   allows the seconds value of the ds3231 to be retreived
	Returns: the seconds value the ds3231 is at currently
*/
uint8_t ds3231GetSecond()
{
	checkCentury();

	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_SECONDS);
	i2cRepeatStart(DS3231_ADDRESS_READ);
	uint8_t seconds = i2cReadNak(); // read from register
	i2cStop();

	return bcdToDec(seconds);
}

/*
   sets the oscillator enable bit in the control register to 1, which indicates that when the
   ds3231 switches to the battery power supply that the oscillator should stop (therefore 
   saving the power). This does mean however that no new data is put into the time registers,
   essentially disables the time functions of the ds3231
		Returns: DS3231_OPERATION_SUCCESS (0)
*/
uint8_t ds3231DisableOscillatorOnBattery(void)
{
	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_CONTROL);
	i2cRepeatStart(DS3231_ADDRESS_READ);
	uint8_t controlReg = i2cReadNak();
	i2cRepeatStart(DS3231_ADDRESS_WRITE);
	i2cWriteThenStop(controlReg | DS3231_CONTROL_EOSC_BIT);

	return DS3231_OPERATION_SUCCESS;
}

/*
   enables the oscillator to run when the ds3231 switches to battery mode. By default this is already the case and therefore there is no need to call this function if "ds3231DisableOscillatorOnBattery(void)" has not been called
		Returns: DS3231_OPERATION_SUCCESS (0)
*/
uint8_t ds3231EnableOscillatorOnBattery(void)
{
	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_CONTROL);
	i2cRepeatStart(DS3231_ADDRESS_READ);
	uint8_t controlReg = i2cReadNak();
	i2cRepeatStart(DS3231_ADDRESS_WRITE);
	i2cWriteThenStop(controlReg & ~(DS3231_CONTROL_EOSC_BIT));

	return DS3231_OPERATION_SUCCESS;
}

/*
   enables the battery backed square wave output. Enabling this clears the INTCN bit and 
   therefore means alarms will not trigger
		Returns: DS3231_OPERATION_SUCCESS (0) if everything was ok
		        1 if an invalid frequency was provided
*/
uint8_t ds3231EnableBBSQW(bbsqw_frequency_t freq)
{
	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_CONTROL);
	i2cRepeatStart(DS3231_ADDRESS_READ);
	uint8_t controlReg = i2cReadNak();
	controlReg &= ~(DS3231_CONTROL_INTCN_BIT); // clear intc otherwise bbsqw will not work
	controlReg |= DS3231_CONTROL_BBQSW_BIT;
	switch(freq)
	{
		case HZ_1: // rs1 and rs2 both clear
			controlReg &= ~(DS3231_CONTROL_RS1_BIT);
			controlReg &= ~(DS3231_CONTROL_RS2_BIT);
			break;
		case KHZ_1_024: // rs1 set, rs2 clear
			controlReg |= DS3231_CONTROL_RS1_BIT;
			controlReg &= ~(DS3231_CONTROL_RS2_BIT);
			break;
		case KHZ_4_096: // rs1 clear, rs2 set
			controlReg |= DS3231_CONTROL_RS2_BIT;
			controlReg &= ~(DS3231_CONTROL_RS1_BIT);
			break;
		case KHZ_8_192: // rs1 and rs2 set
			controlReg |= (DS3231_CONTROL_RS1_BIT | DS3231_CONTROL_RS2_BIT);
			break;
		default:
			return 1;
	}

	i2cRepeatStart(DS3231_ADDRESS_WRITE);

	i2cWriteThenStop(controlReg);
	return DS3231_OPERATION_SUCCESS;
}

/*
   the ds3231 updates the temperature values every 64 seconds, however a user can force
   a new temperature reading by setting the CONV bit in the CONTROL register
*/
void ds3231ForceTemperatureUpdate(void)
{
	bool busy = true;
	do // loop until BSY is clear (we can start our conversion)
	{
		i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_STATUS);
		i2cRepeatStart(DS3231_ADDRESS_READ);
		uint8_t statusReg = i2cReadNak();
		i2cStop();

		if(!(statusReg & DS3231_STATUS_BSY_BIT))
			busy = false;
	} while(busy);

	// set the CONV bit to start a new conversion
	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_CONTROL);
	i2cRepeatStart(DS3231_ADDRESS_READ);
	uint8_t controlReg = i2cReadNak();
	i2cStop();

	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_CONTROL);
	i2cWriteThenStop(controlReg | DS3231_CONTROL_CONV_BIT);

	busy = true;
	do // loop until CONV becomes clear (conversion complete)
	{
		i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_CONTROL);
		i2cRepeatStart(DS3231_ADDRESS_READ);
		uint8_t controlReg = i2cReadNak();
		i2cStop();

		if(!(controlReg & DS3231_CONTROL_CONV_BIT))
			busy = false;
	} while(busy);
}

/*
   reads the temperature sensor of the ds3231. The temperature is encoded in a uint16_t with
   bits 15 to 8 (0 indexed) representing a SIGNED integer temperature and bits 7 to 6 
   representing the decimal part of the temperature. For example, if the function
   returned...
   6464 which is 0001100101000000
   Then the top 8 bits (00011001) are the SIGNED integer representation of the temperature,
   which is +25
   And the following 2 bits (01) are the fractional part of the temperature, which is 0.25
   So the tempreature read was +25.25
		Returns: encoded 10 bit temperature value
*/
uint16_t ds3231GetTemperature(void)
{
	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_TEMPERATURE_MSB);
	i2cRepeatStart(DS3231_ADDRESS_READ);
	uint8_t temperatureUpper = i2cReadNak();
	i2cStop();

	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_TEMPERATURE_LSB);
	i2cRepeatStart(DS3231_ADDRESS_READ);
	uint8_t temperatureLower = i2cReadNak();
	i2cStop();

	return (temperatureUpper << 8) | temperatureLower;
}

/*
   checks if the OSCILLATOR STOPPED FLAG (OSF) is set, if so the oscillator was stopped at some point, therefore the validity of the data held in the ds3231's registers may be at risk.
   Also, clears the OSF flag if it was set
		Returns: true if the oscillator has stopped at some point, false if it hasn't
*/
bool ds3231HasOscillatorStopped(void)
{
	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_STATUS);
	i2cRepeatStart(DS3231_ADDRESS_READ);
	uint8_t statusReg = i2cReadNak();
	i2cStop();

	bool didStop = false;
	if(statusReg & DS3231_STATUS_OSF_BIT) // oscillator stopped flag set
	{
		didStop = true;
		// now reset the flag
		i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_STATUS);
		i2cWriteThenStop(statusReg & ~(DS3231_STATUS_OSF_BIT));
	}

	return didStop;
}

/*
   enables the 32KHz square wave output signal. The oscillator must be running for this
   wave to be output
		Returns: DS3231_OPERATION_SUCCESS (0)
*/
uint8_t ds3231Enable32KHzOutput(void)
{
	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_STATUS);
	i2cRepeatStart(DS3231_ADDRESS_READ);
	uint8_t statusReg = i2cReadNak();
	i2cStop();
	if(statusReg & ~DS3231_STATUS_EN32KHZ_BIT) // wasn't enabled, so enable it
	{
		i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_STATUS);
		i2cWriteThenStop(statusReg | DS3231_STATUS_EN32KHZ_BIT);
	}

	return DS3231_OPERATION_SUCCESS;
}

/*
   disables the 32KHz square wave output signal
		Returns: DS3231_OPERATION_SUCCESS (0)
*/
uint8_t ds3231Disable32KhzOutput(void)
{
	i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_STATUS);
	i2cRepeatStart(DS3231_ADDRESS_READ);
	uint8_t statusReg = i2cReadNak();
	i2cStop();
	if(statusReg & DS3231_STATUS_EN32KHZ_BIT) // is enabled, so disable it
	{
		i2cSetRegisterPointer(DS3231_ADDRESS_WRITE, DS3231_REGISTER_STATUS);
		i2cWriteThenStop(statusReg & ~DS3231_STATUS_EN32KHZ_BIT);
	}

	return DS3231_OPERATION_SUCCESS;
}

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
