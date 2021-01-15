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
#define MODE_USER 3


class StringLight {
public:
    explicit StringLight(uint8_t pin);

    /**
     * Initialize the lights, must be called in setup() before lights will function
     * @param startOn whether or not to turn on the lights at startup
     */
    void start(bool startOn);

    /**
     * sets the color of the lights
     * @param r
     * @param g
     * @param b
     */
    void setColorRGB(uint8_t r, uint8_t g, uint8_t b);

    /**
     * get the current RGB color of the lights
     * @return a long representation of the color with a bit pattern of 0x00RRGGBB
     */
    unsigned long getColorRGB() const;

    /**
    * sends a pulse to the led, only works if set to mode MODE_USER
    */
    void sendPulse(uint8_t numPulses=1, int pulseTimeMicros=5) const;

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

    void turnOn();

    void turnOff();

    bool isOn() const;

    void selectNextColorSkippingOffAndWhite();

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
    bool setMode(uint8_t id);

    int getMode() const;

    bool isAsync() const;

    uint8_t getSpeed() const;

    void setSpeed(uint8_t speed);

private:

    uint8_t pin;

    const static uint8_t INCREMENT_TIME = 25; // amount of delay per color in microseconds

    bool lightsOn = false;

    uint8_t currentColor = WHITE;

    int currentR = 255;
    int currentG = 255;
    int currentB = 255;

    // timing delays for setting an rgb color, initialized on startup
    int redDelay = 0;
    int greenDelay = 0;
    int blueDelay = 0;

    // keeps track of initialization state of lights
    bool isStarted = false;

    // expressed in degrees per second
    int minSpeed = 3;
    int maxSpeed = 180;

    uint8_t speed = 50; // control variable for speed, 0 = minSpeed, 255 = maxSpeed

    const float degPerSecondPerUnitSpeed = ((float) (maxSpeed - minSpeed)) / 255.0f; // number of degrees represented by a speed increase of one
    uint8_t msPerDeg = 0; // fade speed expressed in ms/deg


    uint8_t loopColors[6][3] = {
            {255, 0, 0},
            {255, 255, 0},
            {0, 255, 0},
            {0, 255, 255},
            {0, 0, 255},
            {255, 0, 255}
    };

    bool async = false;

    uint8_t lightMode = MODE_SOLID;

    /**
     * sends pulses until light is set to color specified
     */
     void setColor(uint8_t color);


    /**
    * sends a pulse to the led, pulse functionality is broken out into this private method so that a client cannot
    * send pulses to the LEDs without setting the proper mode first. This ensures that we can always keep track of
    * what the current color is
    */
    void sendPulseInternal(uint8_t numPulses=1, int pulseTimeMicros=50) const;


    /**
     * sets the color of the lights, private function for faster updating the color in a loop when doing direct manipulation
     * on currentR, currentG, amd currentB
     */
    void setColorRGB(uint8_t r, uint8_t g, uint8_t b, bool updateGlobals);

    void loopRGB();
};


#endif //READ_STRINGLIGHTS_STRINGLIGHT_H
