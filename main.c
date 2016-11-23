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

	ds3231SetSecond(7);
	ds3231SetMinute(59);
	ds3231SetHour(FALSE, FALSE, 23);
	ds3231SetDay(THURSDAY);
	ds3231SetDate(31);
	ds3231SetMonth(DEC);
	ds3231SetYear(99);
	ds3231SetCentury(0); // century 0 = year 20xx

	ds3231SetAlarm1(1,1,1,1,5);


	while(1)
	{
		//usartTransmitByte(ds3231GetSecond());
		/*usartTransmitByte(ds3231GetMinute());
		usartTransmitByte(ds3231GetHour());
		usartTransmitByte((uint8_t) ds3231GetDay());
		usartTransmitByte(ds3231GetDate());
		usartTransmitByte((uint8_t) ds3231GetMonth());
		usartTransmitByte(ds3231GetYear());
		usartTransmitByte(ds3231GetCentury());*/

		//_delay_ms(1000);

		if(PINB & (1 << PB0))
		{
			continue;
		}
		else // pin is low (therefore ds3231 alarm triggered)
		{
			usartTransmitByte(65);
			ds3231ClearAlarmFlag(ALARM_1);
		}

	}
}
