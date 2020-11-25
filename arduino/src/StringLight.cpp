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
    startOn ? turnOn() : turnOff();
    currentColor = WHITE;
}

void StringLight::setColorRGB(int r, int g, int b) {
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
            // display current color
            digitalWrite(pin, HIGH);
            delayMicroseconds(fade_currentPulseTime);

            // display next color
            selectNextColorSkippingOff();
            delayMicroseconds(fade_nextPulseTime);
            selectNextColorSkippingOff(6); // go back to current color

            if (millis() - timer > fadeSpeed) {
                timer = millis();
                if (fade_nextPulseTime >= fade_maxPulseTime) {
                    selectNextColorSkippingOff();
                    fade_currentPulseTime = fade_nextPulseTime;
                    fade_nextPulseTime = 0;
                }

                if (fade_currentPulseTime > fade_step) {
                    // don't allow negative time
                    fade_currentPulseTime -= fade_step;
                }

                fade_nextPulseTime += fade_step;
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
        case MODE_SOLID:
        case MODE_FADE:
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
    digitalWrite(pin, LOW);
    lightsOn = false;
}

void StringLight::turnOn() {
    digitalWrite(pin, HIGH);
    delay(1);
    currentColor = WHITE;
    lightsOn = true;
}

bool StringLight::isOn() const {
    return lightsOn;
}




