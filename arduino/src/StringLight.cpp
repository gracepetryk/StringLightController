//
// Created by gracepetryk on 11/15/20.
//

#include "StringLight.h"

StringLight::StringLight(int pin) {
    this->pin = pin;
}

/**
 * initializes the string lights
 */
void StringLight::start(bool startOn) {
    pinMode(pin, OUTPUT);
    setColorRGB(currentR, currentG, currentB, false);
    startOn ? turnOn() : turnOff();
    currentColor = WHITE;
    isStarted = true;
}

void StringLight::sendPulse(int numPulses, int pulseTimeMicros) const {
    if (lightMode != MODE_USER || !isStarted) {
        return;
    }

    sendPulseInternal(numPulses, pulseTimeMicros);
}

void StringLight::sendPulseInternal(int numPulses, int pulseTimeMicros) const {
    if (!isStarted) return;

    for (int i = 0; i < numPulses; i++) {
        digitalWrite(pin, LOW);
        delayMicroseconds(pulseTimeMicros);
        digitalWrite(pin, HIGH);
    }
}

void StringLight::setColorRGB(int r, int g, int b) {
    if (!isStarted) return;

    setColorRGB(r, g, b, true);
}

void StringLight::setColorRGB(int r, int g, int b, bool updateGlobals) {
    if (!isStarted) return;

    if (updateGlobals) {
        currentR = r;
        currentG = g;
        currentB = b;
    }

    redDelay = r * INCREMENT_TIME;
    greenDelay = g * INCREMENT_TIME;
    blueDelay = b * INCREMENT_TIME;
}

unsigned long StringLight::getColorRGB() const {
    unsigned long color = 0;
    color = color | currentR;
    color = color << 8u;
    color = color | currentG;
    color = color << 8u;
    color = color | currentB;

    return color;
}

void StringLight::setColor(int color) {
    // sends the right number of pulses to switch the light to the desired color no matter what the current color is

    if (!isStarted) return;

    sendPulseInternal(((8 + color - currentColor) % 8));
    currentColor = color;
    digitalWrite(pin, HIGH);
}

void StringLight::selectNextColorSkippingOff(int numSkips) {
    if (!isStarted) return;

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

void StringLight::loopRGB() {
    setColor(RED);
    delayMicroseconds(redDelay);

    setColor(GREEN);
    delayMicroseconds(greenDelay);

    setColor(BLUE);
    delayMicroseconds(blueDelay);
}

void StringLight::loopLight() {
    if (!isStarted) return;

    if (!lightsOn) {
        digitalWrite(pin, LOW);
        return;
    }

    switch (lightMode) {
        case MODE_SOLID:
            loopRGB();
            break;

        case MODE_JUMP:
            if (millis() - timer > jumpSpeed) {
                timer = millis();
                selectNextColorSkippingOff();
            }
            break;

        case MODE_FADE:
            loopRGB();

            if (millis() - timer > fadeSpeedMillis / 4) {
                // each step is ~0.25 degrees on the hsv color wheel, so dividing fadeSpeedMillis / 4 gives us time per step
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
                } else {
                    // color is not on edge of color wheel, subtract lowest color until it is
                    int lowestColor = min(currentR, min(currentG, currentB));
                    if (currentR == lowestColor) {
                        currentR--;
                    } else if (currentG == lowestColor) {
                        currentG--;
                    } else {
                        currentB--;
                    }
                }
                setColorRGB(currentR, currentG, currentB, false);
            }
        default:
            break;
    }
}

void StringLight::startAsync() {
    if (isAsync() || !isStarted) return;

    // send decreasing pulses to LED's, each led will start detecting pulses at slightly different pulse lengths
    for (int i = 6000; i > 100; i -= 20) {
        sendPulseInternal(5, i);
        delay(5);
    }
    async = true;
}

void StringLight::stopAsync() {
    if (!isAsync() || !isStarted) return;

    digitalWrite(pin, LOW);
    delay(100);
    digitalWrite(pin, HIGH);
    delay(10);
    async = false;
    currentColor = WHITE;
}


bool StringLight::setMode(int id) {
    if (!isStarted) return false;

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
        case MODE_SOLID:
        case MODE_JUMP:
            lightMode = id;
            return true;
        default:
            return false;
    }
}

int StringLight::getMode() const {
    return lightMode;
}


bool StringLight::isAsync() const {
    return async;
}

void StringLight::turnOff() {
    if (!isStarted) return;

    if (isOn()) {
        digitalWrite(pin, LOW);
        lightsOn = false;
    }
}

void StringLight::turnOn() {
    if (!isStarted) return;

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

float StringLight::getFadeSpeed() const {
    return fadeSpeed;
}

void StringLight::setFadeSpeed(float speed) {
    if (!isStarted) return;

    fadeSpeed = speed;
    fadeSpeedMillis = (int) (1000 / speed);
}








