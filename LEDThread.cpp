#include <Arduino.h>
#ifdef CMAKE
#include <cstring>
#endif 
#include <Wire.h>                   // For using I2C communication
#include <Wireling.h>               // For interfacing with Wirelings
#include "src/omilli/Thread.h"
#include "src/omilli/OMilli.h"
#include "LEDThread.h"

LEDThread ledThread; 

LEDThread::LEDThread() {}

void LEDThread::show(int16_t effect, int16_t brightness, int16_t msPeriod) {
    this->brightness = brightness;
    this->msPeriod = msPeriod;
    this->effect = effect;
    FastLED.setBrightness(brightness);
    FastLED.show();
}

void LEDThread::setup(uint8_t port, uint16_t msLoop) {
    id = 'D';
    this->msLoop = msLoop;
    this->port = port;
    Thread::setup();
    Wireling.begin();
    Wireling.selectPort(port);  // 
    switch (port) {
    case 0: FastLED.addLeds<WS2812, A0, GRB>(leds,NUM_LEDS); break;
    case 1: FastLED.addLeds<WS2812, A1, GRB>(leds, NUM_LEDS); break;
    case 2: FastLED.addLeds<WS2812, A2, GRB>(leds, NUM_LEDS); break;
    case 3: FastLED.addLeds<WS2812, A3, GRB>(leds, NUM_LEDS); break;
    default: FastLED.addLeds<WS2812, A0, GRB>(leds, NUM_LEDS); break;
    }

    // Start dark
    FastLED.setBrightness(brightness);
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB(0,0,0);
    }
    msPeriod = 150; // LED frame rate 

    pinMode(ledPin, OUTPUT);
    FastLED.show();
    om::println("LEDThread.setup()");
}

void LEDThread::loop() {
    Wireling.selectPort(port);  
    if (brightness) {
        brightness = (brightness * effect)/100;
        nextLoop.ticks = om::ticks() + MS_TICKS(msPeriod);
        if (brightness != lastBrightness || 
            lastRgb.r != leds[0].r ||
            lastRgb.g != leds[0].g ||
            lastRgb.b != leds[0].b) {
            FastLED.setBrightness(brightness);
            FastLED.show();
            lastRgb = leds[0];
            lastBrightness = brightness;
        }
    } else {
        nextLoop.ticks = om::ticks() + MS_TICKS(msLoop);
    }
}
