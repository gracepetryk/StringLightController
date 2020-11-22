#include <Arduino.h>
#include "StringLight.h"
#define LED_PIN 5
#define BUTTON_PIN  3

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

// starting colors
int red = 255;
int green = 0;
int blue = 150;

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
            if (currentColor < 3) {
                colors[currentColor] = readByte;
            } else {
                stringLight.setColorRGB(colors[0], colors[1], colors[2]);
                currentColor = 0;
                readMode = MODE_SET_COLOR;
            }
        }
    }
}


void setup() {
    Serial.begin(9600);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    stringLight.start();
    stringLight.setColorRGB(red, green, blue);
    digitalWrite(LED_PIN, HIGH);

    stringLight.setColorRGB(255, 50, 0);
}

bool buttonPressed = false;
void loop() {
    controlOverSerial();
    stringLight.loopLight();

    if (digitalRead(BUTTON_PIN) == HIGH && !buttonPressed) {
        if (stringLight.isOn()) {
            stringLight.turnOff();
        } else {
            stringLight.turnOn();
        }
        digitalWrite(LED_PIN, LOW);
        buttonPressed = true;
    }

    if (digitalRead(BUTTON_PIN) == LOW) {
        buttonPressed = false;
    }
}






