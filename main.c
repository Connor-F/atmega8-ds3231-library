#include "DS3231.h"
#include "USART.h"

#include <util/delay.h>
#include <avr/power.h>

int main()
{
	clock_prescale_set(clock_div_1);
	initUSART();
	initDS3231();
	ds3231SetSecond(50);
	ds3231SetMinute(59);
	ds3231SetHour(FALSE, FALSE, 23);
	ds3231SetDay(THURSDAY);
	ds3231SetDate(31);
	ds3231SetMonth(DEC);
	ds3231SetYear(99);
	ds3231SetCentury(0); // century 0 = year 20xx


	while(1)
	{
		usartTransmitByte(ds3231GetSecond());
		usartTransmitByte(ds3231GetMinute());
		usartTransmitByte(ds3231GetHour());
		usartTransmitByte((uint8_t) ds3231GetDay());
		usartTransmitByte(ds3231GetDate());
		usartTransmitByte((uint8_t) ds3231GetMonth());
		usartTransmitByte(ds3231GetYear());
		usartTransmitByte(ds3231GetCentury());
		_delay_ms(1000);
	}
}
