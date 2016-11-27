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
	ds3231SetSecond(7);
	ds3231SetMinute(59);
	ds3231SetHour(23, false);
	ds3231SetDay(THURSDAY);
	ds3231SetDate(31);
	ds3231SetMonth(DECEMBER);
	ds3231SetYear(99);
	ds3231SetCentury(0); // century 0 = year 20xx

	alarm_t alarm;
	alarm.alarmNumber = ALARM_1;
	alarm.second = 9;
	alarm.trigger = A1_SEC_MATCH;
	ds3231SetAlarm(alarm);

	while(1)
	{
		uint16_t temp = ds3231GetTemperature();
		usartTransmitByte((uint8_t) (temp >> 8));
		usartTransmitByte((uint8_t) temp);
		//usartTransmitByte(ds3231GetSecond());
		/*usartTransmitByte(ds3231GetMinute());
		usartTransmitByte(ds3231GetHour());
		usartTransmitByte((uint8_t) ds3231GetDay());
		usartTransmitByte(ds3231GetDate());
		usartTransmitByte((uint8_t) ds3231GetMonth());
		usartTransmitByte(ds3231GetYear());
		usartTransmitByte(ds3231GetCentury());*/

		ds3231ForceTemperatureUpdate();
		_delay_ms(300);
		temp = ds3231GetTemperature();
		usartTransmitByte((uint8_t) (temp >> 8));
		usartTransmitByte((uint8_t) temp);


		/*if(PINB & (1 << PB0))
		{
			continue;
		}
		else // pin is low (therefore ds3231 alarm triggered)
		{
			usartTransmitByte(65);
			ds3231ClearAlarmFlag(ALARM_1);
		}*/

	}
}
