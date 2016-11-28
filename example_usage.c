#include "DS3231.h"
#include <avr/power.h> // 8MHz cpu

/*
   this example program shows how to set the time, day, date, month, year and century of the ds3231

   it also shows how to set an alarm on the ds3231 and how to naively check if an alarm has been triggered
*/

int main()
{
	clock_prescale_set(clock_div_1);

	// 1. must initialise the ds3231
	initDS3231();

	// 2. set hour mode
	ds3231Use12HourMode(false);
	// 3. set time values
	ds3231SetSecond(58);
	ds3231SetMinute(59);
	ds3231SetHour(14, false);
	// alternatively the above 3 calls can be replaced by the following line
	ds3231SetTime(14, 59, 58, false);

	// 4. set day, date, month, year and century values
	ds3231SetDay(THURSDAY);
	ds3231SetDate(28);
	ds3231SetMonth(DECEMBER);
	ds3231SetYear(16);
	ds3231SetCentury(0); // century 0 = year 20xx
	// alternatively the above 4 calls can be replaced by the following line
	ds3231SetFullDate(THURSDAY, 28, DECEMBER, 16, 0);

	// create alarm object and initialise appropriate values
	alarm_t alarm;
	alarm.alarmNumber = ALARM_1;
	alarm.second = 2;
	alarm.minute = 40;
	alarm.trigger = A1_MIN_SEC_MATCH; // trigger alarm at 2 seconds into
	// the 40th minute of every hour
	//alarm.hour = 13; // not needed in for A1_MIN_SEC_MATCH
	//alarm.useDay = true;
	//alarm.dayDate = WEDNESDAY;

	uint8_t err = ds3231SetAlarm(&alarm);
	//if(err)
		// handle invalid alarm error

	while(1)
	{
		if(PINB & (1 << PB0)) // the pin we are monitoring for alarms is HIGH so no alarm has been triggered
		{
			continue;
		}
		else // pin is low (therefore ds3231 alarm triggered)
		{
			ds3231ClearAlarmFlag(ALARM_1); // clear correct flag (so
			// alarm doesn't keep triggering)
		}
		
		// a better way to check for triggered alarms would be to use
		// a pin change interrupt routine
	}

	return 0;
}
