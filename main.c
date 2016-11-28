#include "DS3231.h"
#include "USART.h"

#include <util/delay.h>
#include <avr/power.h>

int main()
{
	clock_prescale_set(clock_div_1);
	initUSART();
	initDS3231();
	DDRB |= (0 << PB0);

	ds3231Use12HourMode(false);
/*	ds3231SetSecond(58);
	ds3231SetMinute(59);
	ds3231SetHour(14, false);
	ds3231SetDay(THURSDAY);
	ds3231SetDate(28);
	ds3231SetMonth(DECEMBER);
	ds3231SetYear(16);
	ds3231SetCentury(0); // century 0 = year 20xx*/

	ds3231SetTime(14, 59, 58, false);
	ds3231SetFullDate(THURSDAY, 28, DECEMBER, 16, 0);

	alarm_t alarm;
	alarm.alarmNumber = ALARM_2;
	//alarm.second = 2;
	alarm.minute = 40;
	alarm.hour = 13;
	alarm.useDay = false;
	alarm.dayDate = 28;
	alarm.trigger = A2_DAY_DATE_HOUR_MIN_MATCH;

	uint8_t err = ds3231SetAlarm(&alarm);
	if(err)
		usartTransmitByte(err);

	while(1)
	{
		/*uint16_t temp = ds3231GetTemperature();
		usartTransmitByte((uint8_t) (temp >> 8));
		usartTransmitByte((uint8_t) temp);*/
		usartTransmitByte(ds3231GetSecond());
		usartTransmitByte(ds3231GetMinute());
		usartTransmitByte(ds3231GetHour());
		usartTransmitByte((uint8_t) ds3231GetDay());
		usartTransmitByte(ds3231GetDate());
		usartTransmitByte((uint8_t) ds3231GetMonth());
		usartTransmitByte(ds3231GetYear());
		usartTransmitByte(ds3231GetCentury());

		/*ds3231ForceTemperatureUpdate();
		_delay_ms(300);
		temp = ds3231GetTemperature();
		usartTransmitByte((uint8_t) (temp >> 8));
		usartTransmitByte((uint8_t) temp);*/


		/*if(PINB & (1 << PB0))
		{
			continue;
		}
		else // pin is low (therefore ds3231 alarm triggered)
		{
			usartTransmitByte(65);
			ds3231ClearAlarmFlag(ALARM_2);
		}*/
	}

	return 0;
}
