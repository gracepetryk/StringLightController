#!/usr/bin/python3

import serial
import time
import subprocess


class LightControl:

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

    def __init__(self):
        # find arduino serial bit
        dev_list = subprocess.run(["ls", "/dev"], capture_output=True)
        dev_list = str(dev_list.stdout).split("\\n")
        serial_port = "/dev/" + [res for res in dev_list if "ttyUSB" in res][0]

        # open serial port and define command object
        self.ser = serial.Serial(serial_port, 9600)
        self.command = bytearray()

        # get current mode
        self.command.append(LightControl.GET_MODE_BYTE)
        res = self._send_command()
        self.current_mode = res[LightControl.GET_MODE_BYTE]

    def _send_command(self):
        self.ser.reset_input_buffer()
        self.ser.write(self.command)

        time.sleep(0.1)

        status_dict = {}
        current_command = 0
        while self.ser.in_waiting > 0:
            status_dict[self.command[current_command]] = self.ser.read()
            current_command += 1
        return status_dict

    def _clear_command(self):
        self.command.clear()

    def set_mode(self, mode_byte):
        self._clear_command()
        self.command.append(mode_byte)
        self._send_command()
        self.current_mode = mode_byte

    def turn_on(self):
        self._clear_command()
        self.command.append(LightControl.ON_BYTE)
        self._send_command()

    def turn_off(self):
        self._clear_command()
        self.command.append(LightControl.OFF_BYTE)
        self._send_command()

    def set_color(self, r, g, b):
        self._clear_command()

        if self.current_mode != LightControl.MODE_SOLID_BYTE:
            self.set_mode(LightControl.MODE_SOLID_BYTE)

        self.command.append(LightControl.SET_COLOR_BYTE)
        self.command.append(r)
        self.command.append(g)
        self.command.append(b)
        self._send_command()
