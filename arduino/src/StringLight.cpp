//
// Created by gracepetryk on 11/15/20.
//

#include "StringLight.h"

StringLight::StringLight(uint8_t pin) {
    this->pin = pin;
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

void StringLight::sendPulse(uint8_t numPulses, int pulseTimeMicros) const {
    if (lightMode != MODE_USER || !isStarted) {
        return;
    }

    sendPulseInternal(numPulses, pulseTimeMicros);
}

void StringLight::sendPulseInternal(uint8_t numPulses, int pulseTimeMicros) const {
    if (!isStarted) return;

    for (int i = 0; i < numPulses; i++) {
        digitalWrite(pin, LOW);
        delayMicroseconds(pulseTimeMicros);
        digitalWrite(pin, HIGH);
    }
}

void StringLight::setColorRGB(uint8_t r, uint8_t g, uint8_t b) {

    setColorRGB(r, g, b, true);
}

void StringLight::setColorRGB(uint8_t r, uint8_t g, uint8_t b, bool updateGlobals) {

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

void StringLight::setColor(uint8_t color) {
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

        uint8_t nextColor = (currentColor + 1) % 8;
        if (nextColor == WHITE) {
            setColor(RED);
        }  else {
            setColor(nextColor);
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

    static unsigned int timer = 0;


    switch (lightMode) {
        case MODE_SOLID:
            loopRGB();
            break;

        case MODE_JUMP: {
            loopRGB();
            unsigned int timeSinceLastCycle = millis() - timer;
            unsigned int jumpSpeed = msPerDeg * 60;
            static size_t currentColor = 0;
            if (timeSinceLastCycle > jumpSpeed) {
                timer = millis();
                setColorRGB(loopColors[currentColor][0], loopColors[currentColor][1], loopColors[currentColor][2]);
                currentColor = (currentColor + 1) % (sizeof(loopColors) / sizeof(loopColors[0]));
            }
            break;
        }
        case MODE_FADE: {
            loopRGB();

            unsigned int timeSinceLastCycle = millis() - timer;
            if (timeSinceLastCycle >= 50) {

                uint8_t fadeIncrement = 4 * (timeSinceLastCycle / msPerDeg);
                fadeIncrement = max(1, fadeIncrement);


                uint8_t maxColor = 255;
                uint8_t minColor = 0;

                timer = millis();

                if (currentR >= maxColor && currentG < maxColor && currentB <= minColor) {
                    // red to yellow
                    currentG += fadeIncrement;
                } else if (currentR > minColor && currentG >= maxColor && currentB <= minColor) {
                    // yellow to green
                    currentR -= fadeIncrement;
                } else if (currentR <= minColor && currentG >= maxColor && currentB < maxColor) {
                    // green to cyan
                    currentB += fadeIncrement;
                } else if (currentR <= minColor && currentG > minColor && currentB >= maxColor) {
                    // cyan to blue
                    currentG -= fadeIncrement;
                } else if (currentR < maxColor && currentG <= minColor && currentB >= maxColor) {
                    //blue to magenta
                    currentR += fadeIncrement;
                } else if (currentR >= maxColor && currentG <= minColor && currentB > minColor) {
                    // magenta to red
                    currentB -= fadeIncrement;
                } else {
                    // color is not on edge of color wheel, subtract lowest color and add highest until it is
                    int lowestColor = min(currentR, min(currentG, currentB));
                    if (currentR == lowestColor) {
                        currentR -= fadeIncrement;
                    } else if (currentG == lowestColor) {
                        currentG -= fadeIncrement;
                    } else {
                        currentB -= fadeIncrement;
                    }


                    int highestColor = max(currentR, max(currentG, currentB));

                    if (currentR == highestColor) {
                        currentR += fadeIncrement;
                    } else if (currentG == highestColor) {
                        currentG += fadeIncrement;
                    } else {
                        currentB += fadeIncrement;
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


bool StringLight::setMode(uint8_t id) {
    if (!isStarted) return false;

    // reset the lights if mode is switched away from user
    if (lightMode == MODE_USER) {
        turnOff();
        delay(1000);
        turnOn();
        currentColor = WHITE;
    }

    switch (id) {
        case MODE_JUMP:
            setColor(RED);
        case MODE_FADE:
        case MODE_SOLID:
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

void StringLight::setSpeed(uint8_t _speed) {
    this->speed = _speed;

    msPerDeg = 1000 / (minSpeed + (uint8_t) ((float) speed * degPerSecondPerUnitSpeed));
}
