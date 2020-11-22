//
// Created by gracepetryk on 11/15/20.
//

#ifndef READ_STRINGLIGHTS_STRINGLIGHT_H
#define READ_STRINGLIGHTS_STRINGLIGHT_H

#include <Arduino.h>

// color order definition
#define WHITE 0
#define OFF 1
#define RED 2
#define GREEN 3
#define RED_GREEN 4
#define BLUE 5
#define RED_BLUE 6
#define BLUE_GREEN 7

// light mode definitions
#define MODE_SOLID 0
#define MODE_JUMP 1
#define MODE_FADE 2
#define MODE_JUMP_ASYNC 3
#define MODE_FADE_ASYNC 4
#define MODE_USER 6


class StringLight {
public:
    explicit StringLight(int pin);

    void start();

    /**
     * sets the color of the lights
     * @param r
     * @param g
     * @param b
     */
    void setColorRGB(int r, int g, int b);

    /**
    * sends a pulse to the led, only works if set to mode MODE_USER
    */
    void sendPulse(int numPulses=1, int pulseTimeMicros=5) const;

    /**
     * include this function in loop() to maintain light functionality
     */
    void loopLight();

    /**
     * make LED color asynchronous with one another
     */
    void startAsync();

    /**
     * makes LED colors synchronous with one another
     */
    void stopAsync();

    void turnOff();

    void turnOn();

    bool isOn() const;

    void selectNextColorSkippingOff(int numSkips = 1);

    /**
     * changes the mode of the lights.
     *
     * @param id the mode to set the lights to, detailed below
     * @returns whether or not the mode change was successful
     *
     * @id MODE_SOLID: display a solid color*
     * @id MODE_JUMP: jump between colors
     * @id MODE_FADE: fade in and out smoothly between colors
     * @id MODE_JUMP_ASYNC: same as MODE_JUMP but each light is a different color
     * @id MODE_FADE_ASYNC: same as MODE_FADE but each light is a different color
     */
    bool setMode(int id);
    int getMode() const;

    bool isAsync() const;


private:

    int pin;

    const static int INCREMENT_TIME = 25; // amount of delay per color in milliseconds

    bool lightsOn = true;

    int currentColor = 0;
    int redDelay = 0;
    int greenDelay = 0;
    int blueDelay = 0;

    /**
     * used for timing operations for certain light modes, reset frequently
     */
    unsigned long timer = 0;

    int jumpSpeed = 100; // interval in ms for jump modes
    int fadeSpeed = 50; // interval in ms for fade modes

    // variables for managing fade modes
    int fade_currentPulseTime = 6000;
    int fade_nextPulseTime = 0;
    int fade_step = 100;
    int fade_maxPulseTime = 6000;

    bool async = false;

    int lightMode = MODE_SOLID;
public:

private:

    /**
     * sends pulses until light is set to color specified
     */
     void setColor(int color);

    /**
    * sends a pulse to the led, pulse functionality is broken out into this private method so that a client cannot
    * send pulses to the LEDs without setting the proper mode first. This ensures that we can always keep track of
    * what the current color is
    */
    void sendPulseInternal(int numPulses=1, int pulseTimeMicros=5) const;
};


#endif //READ_STRINGLIGHTS_STRINGLIGHT_H
