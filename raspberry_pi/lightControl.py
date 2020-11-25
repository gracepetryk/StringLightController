#!/usr/bin/python3

import serial
import time
import glob

# define communication bytes
ON_BYTE = 0xFF
OFF_BYTE = 0xFE
SET_COLOR_BYTE = 0xFD
GET_MODE_BYTE = 0xFC
GET_ON_OFF_BYTE = 0xFB
ACK_BYTE = 0xFF
FAIL_BYTE = 0xF0
MODE_SOLID_BYTE = 0x00
MODE_JUMP_BYTE = 0x01
MODE_FADE_BYTE = 0x02
MODE_JUMP_ASYNC_BYTE = 0x03
MODE_FADE_ASYNC_BYTE = 0x04
MODE_USER_BYTE = 0x06


ser = serial.Serial('/dev/ttyUSB0', 9600)
time.sleep(2) # give arduino time to init


command = bytearray()
command.append(ON_BYTE)
command.append(MODE_SOLID_BYTE)
command.append(SET_COLOR_BYTE)
command.append(0xFF)
command.append(0x00)
command.append(0xFF)

ser.write(command)

time.sleep(0.1)
while ser.in_waiting:
    print(ser.read())
