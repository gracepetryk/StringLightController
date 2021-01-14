//
// Created by gracepetryk on 11/15/20.
//

#include "StringLight.h"

StringLight::StringLight(int pin) {
    this->pin = pin;
    degPerUnitSpeed = ((float) (maxSpeed - minSpeed)) / 255.0f;
}

/**
 * initializes the string lights
 */
void StringLight::start(bool startOn) {
    isStarted = true;
    pinMode(pin, OUTPUT);
    setMode(MODE_SOLID);
    setColorRGB(currentR, currentG, currentB, false);
    setSpeed(speed);

    if (startOn) {
        turnOn();
    } else {
        turnOff();
    }

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

    setColorRGB(r, g, b, true);
}

void StringLight::setColorRGB(int r, int g, int b, bool updateGlobals) {

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
    color = color | (uint8_t) currentR;
    color = color << 8u;
    color = color | (uint8_t) currentG;
    color = color << 8u;
    color = color | (uint8_t)currentB;

    return color;
}

void StringLight::setColor(int color) {
    // sends the right number of pulses to switch the light to the desired color no matter what the current color is

    if (!isStarted) return;

    sendPulseInternal(((8 + color - currentColor) % 8));
    currentColor = color;
    digitalWrite(pin, HIGH);
}

void StringLight::selectNextColorSkippingOffAndWhite() {
    if (!isStarted) return;

    if (isAsync()) {
        sendPulseInternal();
    } else {
        if (currentColor == BLUE_GREEN) {
            setColor(RED);
        }  else {
            setColor(++currentColor % 8)      ;
        }
    }

}

void StringLight::loopRGB() {
    if (!isStarted) return;

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
                selectNextColorSkippingOffAndWhite();
            }
            break;

        case MODE_FADE:
            loopRGB();

            if (millis() - timer > msPerDeg) {
                // one degree is approximately and increment/decrement of 4
                timer = millis();
                if (currentR == 255 && currentG < 255 && currentB == 0) {
                    // red to yellow
                    currentG += 4;
                } else if (currentR > 0 && currentG == 255 && currentB == 0) {
                    // yellow to green
                    currentR -= 4;
                } else if (currentR == 0 && currentG == 255 && currentB < 255) {
                    // green to cyan
                    currentB += 4;
                } else if (currentR == 0 && currentG > 0 && currentB == 255) {
                    // cyan to blue
                    currentG -= 4;
                } else if (currentR < 255 && currentG == 0 && currentB == 255) {
                    //blue to magenta
                    currentR += 4;
                } else if (currentR == 255 && currentG == 0 && currentB > 0) {
                    // magenta to red
                    currentB -= 4;
                } else {
                    // color is not on edge of color wheel, subtract lowest color until it is
                    int lowestColor = min(currentR, min(currentG, currentB));
                    if (currentR == lowestColor) {
                        currentR -= 4;
                    } else if (currentG == lowestColor) {
                        currentG -= 4;
                    } else {
                        currentB -= 4;
                    }
                }

                // clamp colors to range of (0, 255)
                int *colors[] = {&currentR, &currentG, &currentB};

                for (int* color : colors) {
                    if (*color > 255) {
                        *color = 255;
                    }
                    if (*color < 0) {
                        *color = 0;
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
    }

    switch (id) {
        case MODE_FADE:
        case MODE_SOLID:
        case MODE_JUMP:
        case MODE_USER:
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

uint8_t StringLight::getSpeed() const {
    return speed;
}

void StringLight::setSpeed(int _speed) {
    this->speed = _speed;

    msPerDeg = 1000 / (minSpeed + (int) ((float) speed * degPerUnitSpeed));
    jumpSpeed = msPerDeg * 60; // convert to ms/60 degrees (6 colors on color wheel = 60 degrees/color)

}
