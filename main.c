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
	ds3231SetHour(FALSE, TRUE, 23);
	ds3231SetDay(WEDNESDAY);

	while(1)
	{
		usartTransmitByte(ds3231GetSecond());
		usartTransmitByte(ds3231GetMinute());
		usartTransmitByte(ds3231GetHour());
		usartTransmitByte((uint8_t) ds3231GetDay());
		_delay_ms(1000);
	}
}
