# allows the reading of the LM75A temperature values via a USART / UART to USB
# adapter
# requires pyserial to be installed

import serial
import time
import math

# /dev/ttyUSB0 needs to be changed to the port the USART to USB adapter
# is plugged in to
ser = serial.Serial("/dev/ttyUSB0", 9600) # C code defaults to 9600 baud
ser.flush()

receivedCounter = 0
while True:
	msb = int(ser.read().encode('hex'), 16)
	counterString = ("[{0:<6}] " + str(msb)).format(receivedCounter)

	print counterString
	receivedCounter += 1
	ser.flush()
