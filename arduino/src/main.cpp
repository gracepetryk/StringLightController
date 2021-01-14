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
byte colors[3];
void controlOverSerial() {
    while (Serial.available() > 0) {
        uint8_t readByte = Serial.read();
        switch (readByte) {
            case GET_ON_OFF_BYTE:
                sendOnOff();
                break;

            case OFF_BYTE:
                stringLight.turnOff();
                Serial.write(ACK_BYTE);
                break;

            case ON_BYTE:
                stringLight.turnOn();
                Serial.write(ACK_BYTE);
                break;

            case SET_COLOR_BYTE: {
                Serial.write(ACK_BYTE);
                for (int i = 0; i <= 2; i++) {
                    colors[i] = Serial.read();
                    if (i == 2) {
                        // last color
                        stringLight.setColorRGB(colors[0], colors[1], colors[2]);
                    }
                }
                break;
            }
            case GET_COLOR_BYTE:
                sendColor();
                break;

            case SET_MODE_BYTE: {
                bool success = stringLight.setMode(Serial.read());
                Serial.write(success ? ACK_BYTE : FAIL_BYTE);
                break;
            }

            case GET_MODE_BYTE:
                //  send the current mode over serial
                Serial.write(stringLight.getMode());
                break;

            case GET_ASYNC_BYTE:
                sendAsync();
                break;

            case SET_ASYNC_BYTE: {
                readByte = Serial.read();
                switch (readByte) {
                    case ON_BYTE:
                        stringLight.startAsync();
                        Serial.write(ACK_BYTE);
                    case OFF_BYTE:
                        stringLight.stopAsync();
                        Serial.write(ACK_BYTE);
                    default:
                        Serial.write(FAIL_BYTE);
                }

                break;
            }
            case GET_SPEED_BYTE:
                sendSpeed();
                break;

            case SET_SPEED_BYTE:
                stringLight.setSpeed(Serial.read());
                Serial.write(ACK_BYTE);
                break;

            case GET_STATUS_BYTE:
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
}

void setup() {
    Serial.begin(9600);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    stringLight.start(true);
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






