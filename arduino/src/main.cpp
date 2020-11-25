#include <Arduino.h>
#include "StringLight.h"
#define LED_PIN 3
#define BUTTON_PIN  A5
#define DEBOUNCE_TIME 200

// bytes for setting the light mode over serial, the rest of the mode definitions are set in StringLight
#define MODE_LISTEN 0
#define MODE_SET_COLOR 1
#define ON_BYTE 0xFF
#define OFF_BYTE 0xFE
#define SET_COLOR_BYTE 0xFD
#define GET_MODE_BYTE 0xFC
#define GET_ON_OFF_BYTE 0xFB
#define ACK_BYTE 0xFF
#define FAIL_BYTE 0xF0

bool async = false;
StringLight stringLight = StringLight(LED_PIN);

/**
 * listens for serial input to control lights
 */
byte readMode = MODE_LISTEN;
byte colors[3];
byte currentColor = 0;
void controlOverSerial() {
    if (Serial.available() > 0) {
        byte readByte = Serial.read();
        if (readMode == MODE_LISTEN) {
            switch (readByte) {
                case OFF_BYTE:
                    stringLight.turnOff();
                    readMode = MODE_LISTEN;
                    Serial.write(ACK_BYTE);
                    break;
                case ON_BYTE:
                    stringLight.turnOn();
                    readMode = MODE_LISTEN;
                    Serial.write(ACK_BYTE);
                    break;
                case SET_COLOR_BYTE:
                    readMode = MODE_SET_COLOR;
                    Serial.write(ACK_BYTE);
                    break;
                case GET_MODE_BYTE:
                    // acknowledge and send the current mode over serial
                    Serial.write(ACK_BYTE);
                    Serial.write(stringLight.getMode());
                    break;
                case GET_ON_OFF_BYTE:
                    // acknowledge and send the current on/off state over serial
                    Serial.write(ACK_BYTE);
                    Serial.write(stringLight.getMode() ? 0xFF : 0x00);
                    break;
                default:
                    bool success = stringLight.setMode(readByte);
                    Serial.write(success ? ACK_BYTE : FAIL_BYTE);
                    break;
            }
        } else if (readMode == MODE_SET_COLOR) {
            // read next 3 bytes to set color
            colors[currentColor] = readByte;
            if (currentColor == 2) {
                // last color
                stringLight.setColorRGB(colors[0], colors[1], colors[2]);
                currentColor = 0;
                readMode = MODE_LISTEN;
            } else {
                currentColor++;
            }
        }
    }
}


void setup() {
    Serial.begin(9600);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    stringLight.start(false);
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






