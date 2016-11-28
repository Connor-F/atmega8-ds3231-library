# allows the reading of the LM75A temperature values via a USART / UART to USB
# adapter
# requires pyserial to be installed

import serial
import time
import math

# /dev/ttyUSB0 needs to be changed to the port the USART to USB adapter
# is plugged in to
ser = serial.Serial("/dev/ttyUSB0", 9600) # Makefile contains baud #define
ser.flush()

receivedCounter = 0
while True:
	msb = int(ser.read().encode('hex'), 16)
	lsb = int(ser.read().encode('hex'), 16)


	fraction = 0.0 # the 2 fractional places the sensor sends
	if lsb & (1 << 7): # 0.5 bit set
		fraction += 0.5
	if lsb & (1 << 6): # 0.25 bit set
		fraction += 0.25

	fraction = str(fraction).split('.')[1] # don't need leading 0.xxx part
	counterString = ("[{0:<6}] " + str(msb)).format(receivedCounter)
	temperatureString = (".{0:<3} *C" ).format(fraction)

	print counterString + temperatureString
	receivedCounter += 1
	ser.flush()
