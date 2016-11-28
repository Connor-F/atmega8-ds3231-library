# DS3231 Real Time Clock and Temperature Sensor  Library

DS3231 RTC library for use with AVR microcontrollers. Tested using an ATMega328. 

The DS3231 is a low-cost, extremely accurate I2C real-time clock (RTC) with an integrated temperaturecompensated crystal oscillator (TCXO) and crystal. The device incorporates a battery input, and maintains accurate timekeeping when main power to the device is interrupted. The integration of the crystal resonator enhances the long-term accuracy of the device as well as reduces the piece-part count in a manufacturing line.

The RTC maintains seconds, minutes, hours, day, date, month, and year information. The date at the end of the month is automatically adjusted for months with fewer than 31 days, including corrections for leap year. Temperature is also recorded by the DS3231.

##Quick start guide

The DS3231 should be initialised with data on first use, then the other functions such as setting alarms can be used.

###Initialising the DS3231

1. Before any DS3231 functions are used a call to `initDS3231();` must be made which sets up the `I2C` bus and resets necessary values
2. The hour mode of the DS3231 should be set next. The DS3231 can operate using a 24 hour or a 12 hour mode. By default a 24 hour mode is used.
`ds3231Use12HourMode(false); // use 24 hour mode (default)`
3. The time values of the DS3231 should be initialised next using the `ds3231Set` functions, for example
`ds3231SetSecond(5);` and `ds3231SetDay(MONDAY);` etc.
Instead of calling each set time function seperately, the function `ds3231SetTime(hour, min, second, isPM);` can be called to set the time in a single line. For example, 
`ds3231SetTime(14, 2, 47, false);`. This sets the time to 14:02:47 (24 hour).
4. Day, date, month, year and century should be given values next. This can be done using the appropriate `ds3231Set` functions, e.g. `ds3231SetMonth(DECEMBER);`.

Alternatively the function `ds3231SetFullDate(TUESDAY, 28, NOVEMBER, 16, 0);` can be used to set the full date in a single line

###Using DS3231 alarms

1. Create an `alarm_t` object (referred to as `alarm` in these steps) and provide values to the necessary fields in the object. For example if you wanted to set an alarm to trigger every time the seconds value of the DS3231 hit `3` then the necessary fields in the `alarm_t` object would be
`alarm.second = 3;`
`alarm.trigger = A1_SEC_MATCH; // set the alarm to go off when the seconds value in 
// `alarm` match the seconds value held by the DS3231`
2. Set the alarm using `ds3231SetAlarm(&alarm);`. Making sure to check the return value to see if the alarm provide had a valid combination, e.g. the second alarm of the DS3231 does not have a seconds register, therefore setting the second alarm to trigger on a seconds match is invalid
3. Once an alarm is triggered, the DS3231 pulls the `INTCN/SQW` pin LOW, which can be detected by the AVR. When this happens, a call to `ds3231ClearAlarmFlag(alarm_number_t);` should be called to stop the DS3231 from continually triggering the alarm and holding the line LOW. For example, if the first alarm was triggered then calling

`ds3231ClearAlarmFlag(ALARM_1);` would stop the DS3231 from signalling an alarm for the first alarm

###Reading the DS3231 Temperature Sensor
1. Call the `ds3231GetTemperature();` function to retreive a `uint16_t` encoded temperature value
2. The top 8 bits of the value represent the signed integer part of the temperature
3. The following 2 bits (after the top 8 bits) represent the fractional part of the temperature with the upper bit being the value 0.5 Celcius and the lower bit being 0.25 celcius
4. Combining the integer and fractional parts of the `uint16_t` give the actual temperature reading
5. This can be achieved using the `temperature_reader.py` file, you can send the 2 byte returned value (the `uint16_t`) via a serial port to a device running the above python file. The encoded temperature will then be decoded and printed to `stdout`

##Library Reference

###Important Constants / Enums / Structs

`typedef enum
{
	...
} day_t;` 
**represents valid days of the week**

`typedef enum
{
	...
} month_t;`
**represents valid months of the year**

`typedef enum
{
	...
} alarm_number_t;`
**represents the 2 available alarms of the ds3231**

`typedef enum
{
	...
} alarm_trigger_t;`
**represents the conditions in which an alarm can be triggered, e.g.
   when the hours, minutes and seconds match the hour, minutes and seconds
   of the ds3231**

`typedef struct
{
	...
} alarm_t;`
**represents an alarm and all its necessary information**

`typedef enum
{
	...
} bbsqw_frequency_t;`
**represents the valid frequencies the battery backed square wave
   can output**

###Functions Overview

`void initDS3231(void);`
**sets up i2c bus and resets any necessary flags. MUST be called before using 
   the ds3231**

`uint8_t setRegisterPointer(uint8_t reg);`
**utility function to set the register pointer on
   the ds3231
	Param: reg -> the register to be pointed at by the register pointer
	Returns: DS3231_OPERATION_SUCCESS (0) on success
			 1 if the register provided was invalid**

`uint8_t getRegisterValue(uint8_t reg);`
   **utility function to get a registers value (single byte)
	Param: reg -> the register to read
	Returns: the value of the register (1 byte)**

`uint8_t writeValueThenStop(uint8_t value, uint8_t reg);`
   **writes the value to the provided register of the
   ds3231
	Param: value -> the value to write to the register
		   reg -> the register to write the value to
    Returns: DS3231_OPERATION_SUCCESS (0) on success
	         1 if the register provided was out of range**

`uint8_t ds3231RemoveAlarm(alarm_number_t alarm);`
   **allows for alarms to be cleared/removed/deleted from the ds3231. This function clears
   the appropriate alarms registers, clears the alarm flag and clears the alarm enable bit.
   Removing an alarm permantely deletes the alarm, unlike the `ds3231ClearAlarmFlag` function 
   which just clears the alarm triggered flag but keeps the alarm
Param: alarm -> the alarm number to clear, e.g. ALARM_2
Returns: DS3231_OPERATION_SUCCESS (0) if everything was ok
		 1 if an invalid alarm number was provided**

`void ds3231Use12HourMode(bool use12HourMode);`
   **sets the global hour mode for the ds3231. The ds3231 offers 2 modes
   for storing the hours value in the timekeeping registers and alarm registers,
   these are AM/PM mode (uses 12 hours and a AM/PM indicator bit) and 24 hour mode. This should be called before any other if needing to change the mode to AM/PM, as
   24 hour mode is selected by default**

`static uint8_t validateAlarm(const alarm_t *alarm);`
   **ensures the alarm_t struct passed to set an alarm contains valid combinations of values 
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
		12 unknown error occurred after handling alarm 1 or 2**


`uint8_t ds3231SetAlarm(const alarm_t *alarm);`
   **sets an alarm on the ds3231. Also ensures INTCN and A1IE / A2IE is set so alarms will function
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
		12 unknown error occurred after handling alarm 1 or 2**

`uint8_t ds3231ClearAlarmFlag(alarm_number_t alarm);`
   **used to reset an alarms flag (that indicates the alarm was triggered). This function does
   NOT delete the alarm, to delete an alarm see the `ds3231RemoveAlarm` function
		Param: alarm -> the alarm number flag to be reset, e.g. ALARM_1
		Returns: DS3231_OPERATION_SUCCESS (0)**

`uint8_t ds3231SetTime(uint8_t hour, uint8_t minute, uint8_t second, bool isPM)`
   **convenience function to set the ds3231 time using a single function
	Param: hour -> the hour to set the ds3231 to
	       minute -> the minute to set the ds3231 to
	       second -> the second to set the ds3231 to
	   isPM -> used to indicate whether the hours value should be treated as a PM value
	   		   this should only be true if 12 hour AM/PM mode is activated and it is a PM value. 
			   By default 12 hour AM/PM mode is NOT enabled**

`uint8_t ds3231SetFullDate(day_t day, uint8_t date, month_t month, uint8_t year, uint8_t century);`
   **allows the day, date, month, year and century to be set with a single function call
		Param: day -> the desired day the DS3231 will be set to
			   date -> the desired date the DS3231 will be set to
			   month -> the desired month the DS3231 will be set to
			   year -> the desired year the DS3231 will be set to
			   century -> the desired century the DS3231 will be set to
		Returns: DS3231_OPERATION_SUCCESS (0) if everything worked, otherwise a non-zero error**

`static void checkCentury(void);`
   **checks to see if the CENTURY_BIT bit is set in the MONTH register. If it is then a new century has been entered so the currentCentury counter is incremented.
   This function should be called at the start / end of every other function that interacts with the DS3231, otherwise turning a century will be missed. However if it is unlikely that the DS3231 will experience a change in century, this function can be ignored and removed from the rest of the library code**

`uint8_t ds3231GetCentury(void);`
	**Returns: the current century of the DS3231, e.g.
		     0 = 20xx, 1 = 21xx etc.**

`void ds3231SetCentury(uint8_t cent);`
   **Sets the starting century for the DS3231. The DS3231 doesn't store the century
   itself, so the library handles it**

`uint8_t ds3231SetYear(uint8_t year);`
   **allows the year to be set on the ds3231
	Param: the year to set on the ds3231
	Returns: DS3231_OPERATION_SUCCESS (0) on success
			 1 if the year is invalid (> 99)**

`uint8_t ds3231GetYear(void);`
   **allows the retreival of the year held by the ds3231
	Returns: the year held by the ds3231**

`uint8_t ds3231SetMonth(month_t month);`
   **sets the month on the ds3231
	Param: month -> the month to set the ds3231 to
	Returns: DS3231_OPERATION_SUCCESS (0) on success
		     1 if the month provided was out of range**

`month_t ds3231GetMonth(void);`
   **allows the retreival of the current date held by the ds3231
	Returns: the month held by the ds3231**

`uint8_t ds3231SetDate(uint8_t date);`
   **allows the date to be set on the ds3231
	Param: date -> the date to set the ds3231 to
	Returns: DS3231_OPERATION_SUCCESS (0) on success
			 1 if the date provided was out of range (> 31)**

`uint8_t ds3231GetDate(void);`
   **allows the date to be retreived from the ds3231
	Returns: the date held by the ds3231**

`uint8_t ds3231SetDay(day_t day);`
   **allows the day value to be set for the ds3231
	Param: day -> the day to set the ds3231 to
	Returns: DS3231_OPERATION_SUCCESS (0) on success
			 1 if the day provided was invalid**

`day_t ds3231GetDay(void);`
   **allows the ds3231 day value to be retreived
	Returns: the current day value of the ds3231**

`uint8_t ds3231SetHour(uint8_t hours, bool isPM);`
   **sets the hours register on the ds3231
   Param: hours -> the hours value to set the ds3231 to
   		  isPM -> if using 12 hour mode isPM states whether the hours value
		  		  represents a PM time (if true) or AM time (if false).
				  isPM is ignored if 24 hour mode is being used
   Return: DS3231_OPERATION_SUCCESS (0) if setting the hours worked
   		   1 if an invalid hours value was supplied for 24 hour mode
   		   2 if an invalid hours value was supplied for 12 hour mode**

`uint8_t ds3231GetHour(void);`
   **allows the ds3231 hour value to be retreived
	Returns: the hours value the ds3231 has currently stored**

`uint8_t ds3231SetMinute(uint8_t minutes);`
   **allows the minutes value of the ds3231 to be set
	Param: minutes -> the minutes value to set on the ds3231
	Returns: DS3231_OPERATION_SUCCESS (0) on success
			 1 if the minutes value was invalid (> 59)**

`uint8_t ds3231GetMinute(void);`
   **allows the minutes value of the ds3231 to be returned
	Returns: the minutes value held by the ds3231**

`uint8_t ds3231SetSecond(uint8_t seconds);`
   **allows the seconds value of the ds3231 to be set
	Param: seconds -> the seconds value to pass to the ds3231
	Returns: DS3231_OPERATION_SUCCESS (0) on success
			 1 if the seconds value was invalid (> 59)**

`uint8_t ds3231GetSecond(void);`
   **allows the seconds value of the ds3231 to be retreived
	Returns: the seconds value the ds3231 is at currently**

`uint8_t ds3231DisableOscillatorOnBattery(void);`
   **sets the oscillator enable bit in the control register to 1, which indicates that when the
   ds3231 switches to the battery power supply that the oscillator should stop (therefore 
   saving the power). This does mean however that no new data is put into the time registers,
   essentially disables the time functions of the ds3231
		Returns: DS3231_OPERATION_SUCCESS (0)**

`uint8_t ds3231EnableOscillatorOnBattery(void);`
   **enables the oscillator to run when the ds3231 switches to battery mode. By default this is already the case and therefore there is no need to call this function if "ds3231DisableOscillatorOnBattery(void)" has not been called
		Returns: DS3231_OPERATION_SUCCESS (0)**

`uint8_t ds3231EnableBBSQW(bbsqw_frequency_t freq);`
   **enables the battery backed square wave output. Enabling this clears the INTCN bit and 
   therefore means alarms will not trigger
		Returns: DS3231_OPERATION_SUCCESS (0) if everything was ok
		        1 if an invalid frequency was provided**

`void ds3231ForceTemperatureUpdate(void);`
   **the ds3231 updates the temperature values every 64 seconds, however a user can force
   a new temperature reading by setting the CONV bit in the CONTROL register**

`uint16_t ds3231GetTemperature(void);`
   **reads the temperature sensor of the ds3231. The temperature is encoded in a uint16_t with
   bits 15 to 8 (0 indexed) representing a SIGNED integer temperature and bits 7 to 6 
   representing the decimal part of the temperature. For example, if the function
   returned...
   6464 which is 0001100101000000
   Then the top 8 bits (00011001) are the SIGNED integer representation of the temperature,
   which is +25
   And the following 2 bits (01) are the fractional part of the temperature, which is 0.25
   So the tempreature read was +25.25
		Returns: encoded 10 bit temperature value**

`bool ds3231HasOscillatorStopped(void);`
   **checks if the OSCILLATOR STOPPED FLAG (OSF) is set, if so the oscillator was stopped at some point, therefore the validity of the data held in the ds3231's registers may be at risk.
   Also, clears the OSF flag if it was set
		Returns: true if the oscillator has stopped at some point, false if it hasn't**

`uint8_t ds3231Enable32KHzOutput(void);`
   **enables the 32KHz square wave output signal. The oscillator must be running for this
   wave to be output
		Returns: DS3231_OPERATION_SUCCESS (0)**

`uint8_t ds3231Disable32KhzOutput(void);`
   **disables the 32KHz square wave output signal
		Returns: DS3231_OPERATION_SUCCESS (0)**

`uint8_t ds3231SetAgingOffset(int8_t offset);`
   **allows the aging offset register value to be set
	Param: offset -> the value to set the aging offset register to
	Returns: DS3231_OPERATION_SUCCESS (0)**

`int8_t ds3231GetAgingOffset(void);`
   **allows the aging offset register to be read
	Returns: the signed value stored in the aging offset register**

`static uint8_t decToBcd(uint8_t val);`
   **used to convert normal decimal numbers to BCD numbers
	Param: val -> the decimal value
	Returns: the BCD representation of val**

**`static uint8_t bcdToDec(uint8_t val);`**
   used to convery binary coded decimal to standard
   decimal numbers
	Param: val -> the BCD value
	Returns: the standard decimal representation of val
