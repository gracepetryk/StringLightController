#include <Arduino.h>
#include "StringLight.h"
#define LED_PIN 3
#define BUTTON_PIN  A5
#define DEBOUNCE_TIME 200


// bytes for controlling the lights over serial

#define GET_ON_OFF_BYTE 0x04
#define OFF_BYTE 0x00
#define ON_BYTE 0x01

#define SET_COLOR_BYTE 0x02
#define GET_COLOR_BYTE 0x05

#define GET_MODE_BYTE 0x03
#define SET_MODE_BYTE 0x08

#define GET_ASYNC_BYTE 0x06
#define SET_ASYNC_BYTE 0x09

#define GET_STATUS_BYTE 0x07

#define GET_SPEED_BYTE 0x0A
#define SET_SPEED_BYTE 0x0B

#define ACK_BYTE 0xFF
#define FAIL_BYTE 0x00

#define TIMEOUT_MS 1000

#define WAITING_FOR_INPUT 0
#define WAITING_FOR_COLOR 1

bool async = false;
StringLight stringLight = StringLight(LED_PIN);

void sendColor() {
    unsigned long color = stringLight.getColorRGB();
    Serial.write(0xFFu & (color >> 16u)); // send red byte
    Serial.write(0xFFu & (color >> 8u)); // send green byte
    Serial.write(0xFFu & color); // send blue byte
}

void sendOnOff() {
    Serial.write(stringLight.isOn() ? 0xFF : 0x00);
}

void sendMode() {
    Serial.write(stringLight.getMode());
}

void sendAsync() {
    Serial.write(stringLight.isAsync() ? 0xFF : 0x00);
}

void sendSpeed() {
    Serial.write(stringLight.getSpeed());
}

/**
 * listens for serial input to control lights
 */
void controlOverSerial() {
    static uint8_t read_mode = WAITING_FOR_INPUT;
    static uint8_t timeout_start = millis();
    if (read_mode == WAITING_FOR_INPUT) {
        timeout_start = millis();
        if (Serial.available() > 0) {
            uint8_t readByte = Serial.read();
            switch (readByte) {
                case GET_ON_OFF_BYTE:
                    sendOnOff();
                    break;

                case OFF_BYTE:
                    Serial.write(ACK_BYTE);
                    stringLight.turnOff();
                    break;

                case ON_BYTE:
                    Serial.write(ACK_BYTE);
                    stringLight.turnOn();
                    break;

                case SET_COLOR_BYTE: {
                    read_mode = WAITING_FOR_COLOR;
                    Serial.write(ACK_BYTE);

                    break;
                }
                case GET_COLOR_BYTE:
                    sendColor();
                    break;

                case SET_MODE_BYTE: {
                    while (Serial.available() == 0) {
                        delayMicroseconds(100);
                        if (millis() - timeout_start > TIMEOUT_MS) {
                            Serial.write(FAIL_BYTE);
                            Serial.flush();
                            return;
                        }
                    }
                    bool success = stringLight.setMode(Serial.read());
                    Serial.write(success ? ACK_BYTE : FAIL_BYTE);
                    break;
                }

                case GET_MODE_BYTE:
                    Serial.write(ACK_BYTE);
                    Serial.write(stringLight.getMode());
                    break;

                case GET_ASYNC_BYTE:
                    Serial.write(ACK_BYTE);
                    sendAsync();
                    break;

                case SET_ASYNC_BYTE: {
                    while (Serial.available() == 0) {
                        delayMicroseconds(100);
                        if (millis() - timeout_start > TIMEOUT_MS) {
                            Serial.write(FAIL_BYTE);
                            Serial.flush();
                            return;
                        }
                    }
                    readByte = Serial.read();
                    switch (readByte) {
                        case ON_BYTE:
                            Serial.write(ACK_BYTE);
                            stringLight.startAsync();
                            break;
                        case OFF_BYTE:
                            Serial.write(ACK_BYTE);
                            stringLight.stopAsync();
                            break;
                        default:
                            Serial.write(FAIL_BYTE);
                            break;
                    }

                    break;
                }
                case GET_SPEED_BYTE:
                    sendSpeed();
                    break;

                case SET_SPEED_BYTE:
                    Serial.write(ACK_BYTE);
                    stringLight.setSpeed(Serial.read());
                    break;

                case GET_STATUS_BYTE:
                    Serial.write(ACK_BYTE);
                    sendMode();
                    sendColor();
                    sendOnOff();
                    sendAsync();
                    sendSpeed();
                    break;
                default:
                    Serial.write(FAIL_BYTE); // invalid command
                    break;
            }
        }
    } else if (read_mode == WAITING_FOR_COLOR) {
        static int i = 0;
        static uint8_t colors[3];
        if (Serial.available() != 0) {
            timeout_start = millis();
            colors[i] = Serial.read();
            if (i == 2) {
                stringLight.setColorRGB(colors[0], colors[1], colors[2]);
                i = 0;
                read_mode = WAITING_FOR_INPUT;
            } else {
                i++;
            }

        } else {
            if (millis() - timeout_start > TIMEOUT_MS) {
                Serial.write(FAIL_BYTE);
                read_mode = WAITING_FOR_INPUT;
                timeout_start = millis();
            }
        }
    }
}

void setup() {
    Serial.begin(19200);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    stringLight.start(true);
    stringLight.setMode(MODE_FADE);
}

bool buttonPressed = false;
unsigned long debounceTimer = millis();
void loop() {
    controlOverSerial();
    stringLight.loopLight();

    if (digitalRead(BUTTON_PIN) == LOW && !buttonPressed) {
        debounceTimer = millis();
        if (stringLight.isOn()) {
            stringLight.turnOff();
        } else {
            stringLight.turnOn();
        }
        buttonPressed = true;
    }

    if (digitalRead(BUTTON_PIN) == HIGH) {
        if (millis() - debounceTimer > DEBOUNCE_TIME) {
            buttonPressed = false;
        }
    }
}

