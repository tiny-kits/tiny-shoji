#include <Arduino.h>
#ifdef CMAKE
#include <cstring>
#endif 
#include <Wire.h>
#include "src/tinycircuits/VL53L0X.h"    // Time-of-Flight Distance sensor
#include "src/omilli/Thread.h"
#include <Wireling.h>   // For interfacing with Wirelings
#include "Accel3Thread.h"
#include "OLED042Thread.h"
#include "LEDThread.h"
#include "LraThread.h"
#include "RangeThread.h"

using namespace om;

RangeThread rangeThread;
VL53L0X distanceSensor; 

#define VERSION "SightSaber 0.0.1"
#define STEP_CAL_LOOPS 60
#define STEP_CAL_TC 0.5
#define STEP_TC 0.5
#define STEP_LOOPS 5
#define MS_MODELOCK 1000 /* For modes with timeouts */
#define MS_MODELOCK_STARTUP 10000
#define DEG_HORIZONTAL 10
#define STEADY_IDLE_MS 500
#define PITCH_SELFTEST 80
#define PITCH_CAL -80
#define STEADY_DIST 35
#define SLEEP_DIST 50
#define MS_INTERMEASUREMENT 1
#define CAL_FLOOR_DT 400L
#define CALIBRATION_DELTA 30
#define MAX_DIST_ERR 50
#define NOTIFY_STARTUP_DEFAULT 0
#define NOTIFY_STARTUP_SELFTEST 1
#define NOTIFY_STARTUP_CALIBRATE 2


char * modeStr[] = {
  "STARTUP",
  "SLFTST",
  "SWEEP ",
  "CALIB ",
  "SLEEP ",
};

char * notifyStr[] = {
    "STARTUP ",
    "BUSY    ",
    "OK      ",
    "TOUCHING",
    "INCOMING",
    "SWEEPING",
    "ERANGE  ",
    "EACCEL  ",
    "SLEEPING",
};

RangeThread::RangeThread() {}

void RangeThread::setup(uint8_t port, uint16_t msLoop) {
    delay(200);              // Sensor Startup time
    id = 'R';
    Thread::setup();
    this->port = port;
    this->msLoop = msLoop;
    om::print("RangeThread.setup");

    om::setI2CPort(port); 
    distanceSensor.init();
    distanceSensor.setTimeout(500);
    uint32_t msTimingBudget = msLoop-MS_INTERMEASUREMENT;
    distanceSensor.setMeasurementTimingBudget(msTimingBudget*1000);

    setMode(MODE_STARTUP);
}

void RangeThread::notify(NotifyType value, int8_t level) {
    if (lastNotify != value) {
        loopsNotify = loops;
    }
    uint16_t diffLoops = loops - loopsNotify;
    uint16_t mod16 = diffLoops % 16;
    uint16_t mod24 = diffLoops % 24;
    uint16_t mod32 = diffLoops % 32;
    uint16_t mod48 = diffLoops % 48;
    uint16_t mod64 = diffLoops % 64;
    int16_t showLed = SHOWLED_FADE85;
    CRGB led = ledThread.leds[0];
    uint8_t brightness = ledThread.brightness;
    
    switch (value) {
    case NOTIFY_STARTUP:
        if (loopsNotify == loops) {
            lraThread.setEffect(
                DRV2605_TRANSITION_RAMP_DOWN_LONG_SMOOTH_1); 
        }
        if (mod16 == 0) {
            brightness = 0xff;
        }
        switch (level) {
        default:
        case NOTIFY_STARTUP_DEFAULT:
            led = CRGB(0xff,0xff,0xff);
            break;
        case NOTIFY_STARTUP_SELFTEST:
            led = CRGB(0,0xff,0);
            break;
        case NOTIFY_STARTUP_CALIBRATE:
            led = CRGB(0,0,0xff);
            break;
        }
        break;
    case NOTIFY_SWEEP:
        if (mod16 % 0 == 0) {
            lraThread.setEffect(0); 
            led = CRGB(0,0,0);
            brightness = 0;
        }
        break;
    case NOTIFY_SLEEP:
        showLed = SHOWLED_FADE50;
        if (loopsNotify == loops) {
            lraThread.setEffect(    
                DRV2605_TRANSITION_RAMP_DOWN_LONG_SMOOTH_1); 
            led = CRGB(0xff,0xff,0xff);
            brightness = 0xff;
        } else if (loops - loopsNotify == 1) {
            led = CRGB(0xff,0x0,0xff);
            brightness = 0xff;
        } else if (loops - loopsNotify == 2) {
            led = CRGB(0x0,0xff,0xff);
            brightness = 0xaa;
        } else if (loops - loopsNotify == 3) {
            led = CRGB(0,0xff,0  );
            brightness = 0x66;
        } else if (loops - loopsNotify == 4) {
            led = CRGB(0,0x0,0xff);
            brightness = 0x66;
        }
        break;
    case NOTIFY_INCOMING:
        if (mod16 == 0) {
            lraThread.setEffect(DRV2605_STRONG_CLICK_100); 
            led = CRGB(0xff,0,0);
            brightness = 0xff;
        }
        break;
    case NOTIFY_TOUCHING:
        if (mod24 == 0) {
            showLed = SHOWLED_FADE85;
            lraThread.setEffect(DRV2605_STRONG_CLICK_100); 
            led = CRGB(0xff,0xff,0);
            brightness = 0xff;
        } else if (level!=1 && level!=3 && mod24 % 12 == 0) { // 2/4
            lraThread.setEffect(DRV2605_SHARP_TICK_1); 
            led = CRGB(0xff,0,0);
            brightness = 0x80;
        } else if (level == 3 && mod24 % 8 == 0) {  // 3/4
            lraThread.setEffect(DRV2605_SHARP_TICK_3); 
            led = CRGB(0xff,0,0);
            brightness = 0x80;
        } else if (level == 4 && mod24 % 6 == 0) {  // 4/4
            lraThread.setEffect(DRV2605_SHARP_TICK_3); 
            led = CRGB(0xff,0,0);
            brightness = 0x80;
        } else if (level == 5 && mod24 % 4 == 0) {  // 6/8
            lraThread.setEffect(DRV2605_SHARP_TICK_3); 
            led = CRGB(0xff,0,0);
            brightness = 0x80;
        }
        break;
    case NOTIFY_BUSY:
        if (mod24 == 0) {
            showLed = SHOWLED_FADE85;
            lraThread.setEffect(DRV2605_STRONG_CLICK_100); 
            led = CRGB(0xff,0,0xff);
            brightness = 0xff;
        } else if (mod24 % 6 == 0) {
            lraThread.setEffect(DRV2605_SHARP_TICK_3); 
            led = CRGB(0,0,0xff);
            brightness = 0x80;
        }
        break;
    case NOTIFY_OK:
        if (mod48 == 0) {
            lraThread.setEffect(DRV2605_SOFT_FUZZ_60); 
            led = CRGB(0,0xff,0);
            brightness = 0xff;
        }
        break;
    default:
    case NOTIFY_EACCEL: // SOSOSOSOS...
    case NOTIFY_ERANGE: // SOSOSOSOS...
        showLed = SHOWLED_FADE50;
        if (mod64==0 || mod64==4 || mod64==8) {
            lraThread.setEffect(DRV2605_SHARP_TICK_1); 
            led = CRGB(0xff,0,0);
            brightness = 0xff;
        } else if (mod64==22 || mod64==34 || mod64==46) {
            lraThread.setEffect(DRV2605_SHARP_TICK_1); 
            led = CRGB(0xff,0,0);
            brightness = 0xff;
        }
        break;
    }

    bool updateDisplay = led != ledThread.leds[0] || 
        brightness != ledThread.brightness;
    if (updateDisplay) {
        ledThread.brightness = brightness;
        ledThread.leds[0] = led;
        ledThread.show(showLed);
    }
    lastNotify = value;
}

void RangeThread::calibrateLength(uint16_t d){
    int32_t diffCal = absval(d - eaDistSlow);
    distCal = expAvg(d, distCal, EATC_2);
    if (diffCal <= CALIBRATION_DELTA) {
        distStick = distCal + CALIBRATION_DELTA;
        msModeLock = 0;
        setMode(MODE_SWEEP);
    } else {
        notify(NOTIFY_BUSY);
        msModeLock = om::millis() + MS_MODELOCK;
    }
}

void RangeThread::sweep(uint16_t d){
    if (d > distStick) {
        notify(NOTIFY_SWEEP);
    } else if (d < distStick*1/5) {
        notify(NOTIFY_TOUCHING, 5);
    } else if (d < distStick*2/5) {
        notify(NOTIFY_TOUCHING, 4);
    } else if (d < distStick*3/5) {
        notify(NOTIFY_TOUCHING, 3);
    } else if (d < distStick*4/5) {
        notify(NOTIFY_TOUCHING, 2);
    } else {
        notify(NOTIFY_TOUCHING, 1);
    }
}

void RangeThread::selftest(uint16_t d){
    bool okRange = d == 65535 ? false : true;
    bool okAccel = -90 <= pitch && pitch <= 90;

    if (okRange && okAccel) {
        notify(NOTIFY_OK);
    } else {
        if (!okRange) {
            notify(NOTIFY_ERANGE);
        } else if (!okAccel) {
            notify(NOTIFY_EACCEL);
        }
    }
}

void RangeThread::setMode(ModeType newMode, bool force) {
    if (mode == newMode && !force) {
        return;
    }

    if (mode != newMode) { // End existing mode
        switch (mode) {
        case MODE_SLEEP:
            monitor.quiet(false);
            distanceSensor.startContinuous(MS_INTERMEASUREMENT); // 19mA
            break;
        case MODE_SELFTEST:
            om:;println(VERSION);
            break;
        }
    }

    uint32_t msNow = om::millis();

    switch (newMode) {  // Begin new mode
    case MODE_STARTUP:
        msModeLock = msNow + MS_MODELOCK_STARTUP;
        break;
    case MODE_SELFTEST:
        msModeLock = msNow + MS_MODELOCK;
        break;
    case MODE_SLEEP:
        // stopContinuous() can't be restarted?
        // so just slow down ranging
        distanceSensor.startContinuous(msLoop*100L); 
        monitor.quiet(true);
        msModeLock = msNow + MS_MODELOCK;
        break;
    case MODE_CALIBRATE: 
        msModeLock = msNow + MS_MODELOCK;
        break;
    case MODE_SWEEP:
        break;
    }
    mode = newMode;
}

void RangeThread::updateOledPosition() {
    // Update OLED position display
    strcpy(oledThread.lines[1], "");
    px->headingToString(oledThread.lines[2]);
    py->headingToString(oledThread.lines[3]);
    pz->headingToString(oledThread.lines[4]);
}

void RangeThread::startup() {
    if (pitch > PITCH_SELFTEST) {
        notify(NOTIFY_STARTUP, NOTIFY_STARTUP_SELFTEST);
    } else if (pitch < PITCH_CAL) {
        notify(NOTIFY_STARTUP, NOTIFY_STARTUP_CALIBRATE);
    } else {
        notify(NOTIFY_STARTUP, NOTIFY_STARTUP_DEFAULT);
    }
}

void RangeThread::loop() {
    nextLoop.ticks = om::ticks() + MS_TICKS(msLoop);
    om::setI2CPort(port); 
    uint32_t msNow = om::millis();
    double az = absval((double) pz->valFast); // either flat side up
    double ay = py->valFast;
    pitch = round(90-atan2(az, -ay) * 180 / PI);
    bool steady = px->heading==HEADING_STEADY && 
        py->heading==HEADING_STEADY && 
        pz->heading==HEADING_STEADY;
    if (!steady) { msUnsteady = msNow; }
    uint16_t d = distanceSensor.readRangeContinuousMillimeters();
    if (d < minRange) {
        d = maxRange;
    }
    eaDistFast = expAvg(d, eaDistFast, EATC_0);
    eaDistSlow = expAvg(d, eaDistSlow, EATC_4);
    float errFast = abs(d - eaDistFast);
    float errSlow = abs(d - eaDistSlow);
    if (errSlow > MAX_DIST_ERR) { // noise possible
        d = maxRange;
    }
    if (maxRange < d) { // reduce average bias
        d = maxRange;
    }
    bool horizontal = -DEG_HORIZONTAL <= pitch && pitch <= DEG_HORIZONTAL;
    eaDistSleep = expAvg(d, eaDistSleep, EATC_6);
    bool still = msNow - msUnsteady > STEADY_IDLE_MS;
    bool flatStill = horizontal && still;
    bool modeLock = om::millis() <= msModeLock;
    bool startingUp = mode == MODE_STARTUP;
    bool lockStartup = startingUp && modeLock;
    bool testing = mode == MODE_SELFTEST;
    bool lockSelftest = testing && modeLock;
    bool startTesting = startingUp && pitch >= PITCH_SELFTEST;
    bool lockCalibrate = mode == MODE_CALIBRATE && modeLock;
    bool startCalibrating = startingUp && pitch <= PITCH_CAL;
    bool sleeping = mode == MODE_SLEEP;
    bool sleepLock = sleeping && modeLock;
    //bool startSleep = eaDistSleep < SLEEP_DIST;
    bool startSleep = false;

    // Chose mode of operation
    if (lockStartup) {
        setMode(MODE_STARTUP);
    } else if (startSleep || sleeping && (horizontal || modeLock)) {
        setMode(MODE_SLEEP);
        if (startSleep || horizontal) {
            msModeLock = om::millis() + MS_MODELOCK;
        }
    } else if (startTesting || lockSelftest ) {
        setMode(MODE_SELFTEST, !lockSelftest);
        if (startTesting) {
            msModeLock = om::millis() + MS_MODELOCK;
        }
    } else if (startCalibrating || lockCalibrate) {
        setMode(MODE_CALIBRATE, !lockCalibrate);
        if (pitch < PITCH_CAL) {
            msModeLock = om::millis() + MS_MODELOCK;
        }
    } else {
        setMode(MODE_SWEEP);
    }

    if (loops % 16 == 0) {
        om::print(modeStr[(int8_t) mode]);
        om::print(" ");
        om::print(notifyStr[(uint8_t)lastNotify]);
        om::print(" d");
        om::print(d);
        om::print(" errFast:");
        om::print(errFast);
        om::print(" errSlow:");
        om::print(errSlow);
        om::print(" distStick:");
        om::print(distStick);
        om::print(" pitch:");
        om::print(pitch);
        for (int ix = 0; ix < HEADING_COUNT; ix++) {
            om::print(" "); 
            if (ix == (int) px->heading+2) {
                om::print(px->dir > 0.5 ? ">" : "<");
            } else {
                om::print("-");
            }
            
        }
        om::println();
    }

    if (mode == MODE_STARTUP) {
        startup();
    } else if (mode == MODE_SELFTEST) {
        selftest(d);
    } else if (mode == MODE_SWEEP) {
        sweep(eaDistSlow);
        updateOledPosition();
    } else if (mode == MODE_CALIBRATE) {
        calibrateLength(d);
    } else if (mode == MODE_SLEEP) {
        notify(NOTIFY_SLEEP);
    } else {
        om::print("UNKNOWN MODE");
        om::print((uint8_t) mode);
        om::println();
    }
}
