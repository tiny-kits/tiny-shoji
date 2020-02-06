#ifndef LED_THREAD_H
#define LED_THREAD_H

#include "src/omilli/OMilli.h"
#include "src/fastled/FastLED.h"
#include "tiny-shoji.h"

#define LED_FRAMERATE_PLA 40 /* 25 fps */
#define SHOWLED_FADE0 0  /* 0% brightness after first period */
#define SHOWLED_FADE20 20  /* 50% brightness each period */
#define SHOWLED_FADE30 30  /* 50% brightness each period */
#define SHOWLED_FADE40 40  /* 50% brightness each period */
#define SHOWLED_FADE50 50  /* 50% brightness each period */
#define SHOWLED_FADE80 80  /* 80% brightness each period */
#define SHOWLED_FADE85 85  /* 85% brightness each period */
#define SHOWLED_FADE90 90  /* 85% brightness each period */
#define SHOWLED_ON 100  /* 100% brightness each period */

#define NUM_LEDS 1

typedef class LEDThread : om::Thread {
public:
    LEDThread();
    void setup(uint8_t port=I2CPORT_DISPLAY, uint16_t msLoop=LED_FRAMERATE_PLA);
    CRGB leds[NUM_LEDS];
    void show(int16_t effect, int16_t brightness=255, int16_t msPeriod=LED_FRAMERATE_PLA);
    int16_t brightness = 255;

protected:
    void loop();
    uint8_t ledPin;
    int16_t msLoop;
    CRGB lastRgb = CRGB(0,0,0);
    int16_t lastBrightness = 0;
    uint8_t port;
    int16_t msPeriod;
    int16_t effect = SHOWLED_FADE50;
} LEDThread;


inline bool operator == (CRGB &c1, CRGB &c2) {
    return c1.r == c2.r && c1.g == c2.g && c1.b == c2.b;
}

inline bool operator != (CRGB &c1, CRGB &c2) {
    return c1.r != c2.r || c1.g != c2.g || c1.b != c2.b;
}


extern LEDThread ledThread;

#endif
