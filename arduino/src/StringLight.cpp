//
// Created by gracepetryk on 11/15/20.
//

#include "StringLight.h"

StringLight::StringLight(int pin) {
    this->pin = pin;
    currentColor = WHITE;
}

void StringLight::sendPulseInternal(int numPulses, int pulseTimeMicros) const {
    for (int i = 0; i < numPulses; i++) {
        digitalWrite(pin, LOW);
        delayMicroseconds(pulseTimeMicros);
        digitalWrite(pin, HIGH);
    }
}

void StringLight::sendPulse(int numPulses, int pulseTimeMicros) const {
    if (lightMode != MODE_USER) {
        return;
    }

    sendPulseInternal(numPulses, pulseTimeMicros);
}

/**
 * initializes the string lights
 */
void StringLight::start(bool startOn) {
    pinMode(pin, OUTPUT);
    setColorRGB(150, 150, 150);
    startOn ? turnOn() : turnOff();
    currentColor = WHITE;
}

void StringLight::setColorRGB(int r, int g, int b) {
    currentR = r;
    currentG = g;
    currentB = b;

    redDelay = r * INCREMENT_TIME;
    greenDelay = g * INCREMENT_TIME;
    blueDelay = b * INCREMENT_TIME;

}

void StringLight::setColor(int color) {
    // sends the right number of pulses to switch the light to the desired color no matter what the current color is
    sendPulseInternal(((8 + color - currentColor) % 8));
    currentColor = color;
    digitalWrite(pin, HIGH);
}

void StringLight::selectNextColorSkippingOff(int numSkips) {
    for (int i = 0; i < numSkips; i++) {
        sendPulseInternal();
        if (currentColor == WHITE) {
            if (isAsync()) {
                i--; // we want to count off for async
            } else {
                sendPulseInternal();
                currentColor++;
            }
        }
        currentColor++;
        currentColor = currentColor % 8;
    }
}

void StringLight::loopLight() {
    if (!lightsOn) {
        digitalWrite(pin, LOW);
        return;
    }

    switch (lightMode) {
        case MODE_SOLID:
            setColor(RED);
            delayMicroseconds(redDelay);

            setColor(GREEN);
            delayMicroseconds(greenDelay);

            setColor(BLUE);
            delayMicroseconds(blueDelay);
            break;

        case MODE_JUMP_ASYNC:
        case MODE_JUMP:
            if (millis() - timer > jumpSpeed) {
                timer = millis();
                selectNextColorSkippingOff();
            }
            break;

        case MODE_FADE_ASYNC:
        case MODE_FADE:
            setColor(RED);
            delayMicroseconds(redDelay);

            setColor(GREEN);
            delayMicroseconds(greenDelay);

            setColor(BLUE);
            delayMicroseconds(blueDelay);

            if (millis() - timer > fadeSpeed) {
                timer = millis();
                if (currentR == 255 && currentG < 255 && currentB == 0) {
                    // red to yellow
                    currentG++;
                } else if (currentR > 0 && currentG == 255 && currentB == 0) {
                    // yellow to green
                    currentR--;
                } else if (currentR == 0 && currentG == 255 && currentB < 255) {
                    // green to cyan
                    currentB++;
                } else if (currentR == 0 && currentG > 0 && currentB == 255) {
                    // cyan to blue
                    currentG--;
                } else if (currentR < 255 && currentG == 0 && currentB == 255) {
                    //blue to magenta
                    currentR++;
                } else if (currentR == 255 && currentG == 0 && currentB > 0) {
                    // magenta to red
                    currentB--;
                }
            }
        default:
            break;
    }
}

void StringLight::startAsync() {
    if (isAsync()) return;

    // send decreasing pulses to LED's, each led will start detecting pulses at slightly different pulse lengths
    for (int i = 6000; i > 100; i -= 20) {
        sendPulseInternal(5, i);
        delay(5);
    }
    async = true;
}

void StringLight::stopAsync() {
    if (!isAsync()) return;

    digitalWrite(pin, LOW);
    delay(1000);
    digitalWrite(pin, HIGH);
    delay(1000);
    async = false;
    currentColor = WHITE;
}


bool StringLight::setMode(int id) {
    // reset the lights if mode is switched away from user
    if (lightMode == MODE_USER) {
        turnOff();
        delay(1000);
        turnOn();
        currentColor = WHITE;
        return true;
    }

    switch (id) {
        case MODE_FADE:
            setColorRGB(255, 0, 0);
        case MODE_SOLID:
        case MODE_JUMP:
            stopAsync();
            break;
        case MODE_FADE_ASYNC:
        case MODE_JUMP_ASYNC:
            startAsync();
            break;
        default:
            return false;
    }
    lightMode = id;
    return true;
}

int StringLight::getMode() const {
    return lightMode;
}


bool StringLight::isAsync() const {
    return async;
}

void StringLight::turnOff() {
    if (isOn()) {
        digitalWrite(pin, LOW);
        lightsOn = false;
    }
}

void StringLight::turnOn() {
    if (!isOn()) {
        digitalWrite(pin, HIGH);
        delay(10);
        currentColor = WHITE;
        if (isAsync()) {
            async = false; // lost async when turning off
            startAsync();
        }
        lightsOn = true;
    }
}

bool StringLight::isOn() const {
    return lightsOn;
}




