#!/usr/bin/python3

import serial
import time
import subprocess
from typing import Union
import json


class LightControlStatus:
    _mode_dict = {
        0: "solid",
        1: "jump",
        2: "fade"
    }

    def __init__(self, mode: int, color: bytes, power_state: bool, is_async: bool, speed: int):
        """

        :param mode: The current mode of the lights
        :param color: The current color of the lights, 3 bytes
        :param speed: The current speed of light fade and jump modes
        :param power_state: Whether the lights are on or off
        """
        self.mode = self._mode_dict[mode]
        self.color = {'r': int(color[0]), 'g': int(color[1]), 'b': int(color[2])}
        self.speed = speed
        self.power_state = power_state
        self.is_async = is_async

    def __str__(self):
        # r = int(self.color[0])
        # g = int(self.color[1])
        # b = int(self.color[2])
        # return("LightControlStatus[mode: {mode}, color: ({0}, {1}, {2}), speed: {speed}, power_state: {power_state}, "
        #        "is_async: {is_async}]").format(r, g, b, **self.__dict__)
        return json.dumps(self.__dict__)


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

    GET_SPEED_BYTE = 0x0A
    SET_SPEED_BYTE = 0x0B

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
        GET_STATUS_BYTE,
        GET_ON_OFF_BYTE,
        MODE_SOLID_BYTE,
        MODE_JUMP_BYTE,
        MODE_FADE_BYTE,
        MODE_USER_BYTE,
        GET_SPEED_BYTE,
        SET_SPEED_BYTE
    )
    
    def __init__(self):
        # find arduino serial port
        dev_list = subprocess.run(["ls", "/dev"], capture_output=True)
        dev_list = str(dev_list.stdout).split("\\n")
        serial_port = "/dev/" + [res for res in dev_list if "ttyUSB" in res][0]

        # open serial port and define command object
        self.ser = serial.Serial(serial_port, 9600)

    def _send_command(self, command: Union[bytes, int]) -> bytes:
        self.ser.reset_input_buffer()
        self.ser.reset_input_buffer()

        if type(command) is int:
            command = command.to_bytes(1, byteorder='big')

        if command[0] not in LightControl.VALID_COMMAND_BYTES:
            raise LightControlSerialException("Invalid command byte {:#x}".format(command[0]))

        self.ser.reset_input_buffer()
        self.ser.write(command)

        time.sleep(50/1000)  # wait 50ms for response

        res = bytearray()
        while self.ser.in_waiting > 0:
            res.append(self.ser.read()[0])

        if (len(res) == 1 and res[0] == self.FAIL_BYTE) or len(res) == 0:
            raise LightControlSerialException("Arduino could not process command")

        return bytes(res)

    def get_status(self) -> LightControlStatus:
        status_bytes = self._send_command(LightControl.GET_STATUS_BYTE)
        hex_str = status_bytes.hex()

        # bytes.hex() isn't working with arguments for some reason
        print(":".join([h1 + h2 for h1, h2 in zip(hex_str[::2], hex_str[1::2])]))
        print(status_bytes[4])
        return LightControlStatus(status_bytes[0],
                                  bytes(status_bytes[1:4]),
                                  status_bytes[4] == 255,
                                  status_bytes[5] == 255,
                                  status_bytes[6])

    def set_mode(self, mode_byte):
        command = bytes([self.SET_MODE_BYTE,  mode_byte])
        self._send_command(command)

    def turn_on(self):
        self._send_command(LightControl.ON_BYTE)

    def turn_off(self):
        self._send_command(LightControl.OFF_BYTE)

    def set_color(self, r, g, b):
        self.set_mode(LightControl.MODE_SOLID_BYTE)
        command = bytes([LightControl.SET_COLOR_BYTE, r, g, b])
        self._send_command(command)

    def set_speed(self, speed):
        if speed < 0 or speed > 255:
            raise ValueError("Speed must be between 0 and 255")

        self._send_command(bytes([LightControl.SET_SPEED_BYTE, speed]))


if __name__ == "__main__":
    lc = LightControl()
    print(lc.get_status())
