#!/usr/bin/python3

import serial
import time
import subprocess
from typing import Union


class LightControlStatus:

    def __init__(self, mode: int, color: bytes, speed: int, power_state: bool, is_async: bool):
        """

        :param mode: The current mode of the lights
        :param color: The current color of the lights, 3 bytes
        :param speed: The current speed of light fade and jump modes
        :param power_state: Whether the lights are on or off
        """
        self.mode = mode
        self.color = color
        self.speed = speed
        self.power_state = power_state
        self.is_async = is_async


class LightControlSerialException(Exception):
    pass


class LightControl:
    # bytes for controlling the lights over serial
    GET_ON_OFF_BYTE = 0x04

    OFF_BYTE = 0x00
    ON_BYTE = 0x01

    SET_COLOR_BYTE = 0x02
    GET_COLOR_BYTE = 0x05

    GET_MODE_BYTE = 0x03
    SET_MODE_BYTE = 0x08

    GET_ASYNC_BYTE = 0x06
    SET_ASYNC_BYTE = 0x09

    GET_STATUS_BYTE = 0x07

    ACK_BYTE = 0xFF
    FAIL_BYTE = 0x00

    # bytes defining the different light modes
    MODE_SOLID_BYTE = 0x00
    MODE_JUMP_BYTE = 0x01
    MODE_FADE_BYTE = 0x02
    MODE_USER_BYTE = 0x06

    VALID_COMMAND_BYTES = (
        SET_COLOR_BYTE,
        GET_MODE_BYTE,
        GET_ON_OFF_BYTE,
        MODE_SOLID_BYTE,
        MODE_JUMP_BYTE,
        MODE_FADE_BYTE,
        MODE_USER_BYTE
    )
    
    def __init__(self):
        # find arduino serial port
        dev_list = subprocess.run(["ls", "/dev"], capture_output=True)
        dev_list = str(dev_list.stdout).split("\\n")
        serial_port = "/dev/" + [res for res in dev_list if "ttyUSB" in res][0]

        # open serial port and define command object
        self.ser = serial.Serial(serial_port, 9600)
        self.command = bytearray()

        # get current mode
        self._send_command(bytes(self.GET_MODE_BYTE))

    def _send_command(self, command: Union[bytes, int]) -> bytes:
        if type(command) is int:
            command = bytes(command)

        if command[0] not in LightControl.VALID_COMMAND_BYTES:
            raise LightControlSerialException("Invalid command byte {:#X}".format(command[0]))

        self.ser.reset_input_buffer()
        self.ser.write(self.command)

        time.sleep(50/1000)  # wait 50ms for response

        res = bytearray()
        while self.ser.in_waiting > 0:
            res.append(self.ser.read()[0])
        self.command.clear()

        if len(res) == 1 and res[0] == self.FAIL_BYTE:
            raise LightControlSerialException("Arduino could not process command")

        return bytes(res)

    def get_status(self) -> LightControlStatus:
        status_bytes = self._send_command(LightControl.GET_STATUS_BYTE)
        return LightControlStatus(status_bytes[0], bytes(status_bytes[1:4]), 0, status_bytes[5] == b'\xff', status_bytes[3] == b'\xff')

    def set_mode(self, mode_byte):
        command = bytes([self.SET_MODE_BYTE,  mode_byte])
        self._send_command(command)

    def get_on_off(self):
        res = self._send_command(self.GET_ON_OFF_BYTE)
        return res[0] == b'\xff'

    def turn_on(self):
        self._send_command(LightControl.ON_BYTE)

    def turn_off(self):
        self._send_command(LightControl.OFF_BYTE)

    def set_color(self, r, g, b):
        self.set_mode(LightControl.MODE_SOLID_BYTE)
        command = bytes([LightControl.SET_COLOR_BYTE, r, g, b])
        self._send_command(command)
