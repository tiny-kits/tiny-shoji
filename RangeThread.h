#ifndef RANGE_THREAD_H
#define RANGE_THREAD_H

#include "src/omilli/Thread.h"
#include "om-ranger.h"
#include "Accel3Thread.h"
#include "src/fastled/FastLED.h"

#define VL53L0X_PERIOD 33 /* Maximum range */

typedef enum RangeType {
  RNG_UNKNOWN = 0,   // Center flash white
  RNG_TOUCH = 1,     // Steady red
  RNG_CLOSE = 2,     // Quick flash red
  RNG_BODY = 3,      // Slow flash red
  RNG_NEAR = 4,      // Steady green
  RNG_FAR = 5,       // Slow flash green
} RangeType;

typedef enum NotifyType {
    NOTIFY_STARTUP = 0,   // Power up
    NOTIFY_BUSY = 1,      // Action in progress
    NOTIFY_OK = 2,        // Action completed successfully
    NOTIFY_TOUCHING = 3,  // Within distStick range
    NOTIFY_INCOMING = 4,  // Rapidly closing, within distStick range
    NOTIFY_SWEEP = 5,     // Sweeping for contact
    NOTIFY_ERANGE = 6,    // Ranging error
    NOTIFY_EACCEL = 7,    // Accelerometer error
    NOTIFY_SLEEP = 8,
} NotifyType;

typedef enum ModeType {
    MODE_STARTUP = 0,   // Power up
    MODE_SELFTEST = 1,  // Triggered by startup
    MODE_SWEEP = 2,     // Sweep for objects in range
    MODE_CALIBRATE = 3, // Calibrate floor height
    MODE_SLEEP = 4,     // Inactive
} ModeType;

typedef class RangeThread : om::Thread {
public:
    RangeThread();
    void setup(uint8_t port=2, uint16_t msLoop=1+VL53L0X_PERIOD);
    
protected:
    uint8_t port;
    uint16_t msLoop;
    void loop();
    RangeType rng = RNG_UNKNOWN;
    uint32_t minRange = 50L;   // Disregard close noise
    uint32_t maxRange = 2500L; // Max ranging white target indoors + 500
    int32_t eaDistFast = 0;    
    int32_t eaDistSlow = 0;
    int32_t eaDistSleep = 2000L;
    int32_t eaDistErr = 0; 
    uint32_t msModeLock = 0;
    uint32_t msUnsteady = 0;
    uint32_t loopsNotify = 0;
    int32_t distStick = 800;
    int32_t distCal = 0;
    int32_t pitch;
    ModeType mode = MODE_SLEEP;
    NotifyType lastNotify = NOTIFY_SLEEP;
    AxisState * px = &accelThread.xState;
    AxisState * py = &accelThread.yState;
    AxisState * pz = &accelThread.zState;

    void sweep(uint16_t dist);
    void selftest(uint16_t d);
    void calibrateLength(uint16_t dist);
    void startup();
    void setMode(ModeType mode, bool force=false);
    void updateOledPosition();
    void notify(NotifyType value, int8_t level=0);
} RangeThread;

extern RangeThread rangeThread;


#endif
