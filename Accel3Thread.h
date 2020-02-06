#ifndef ACCEL3_THREAD_H
#define ACCEL3_THREAD_H

#include "src/omilli/Thread.h"
#include "tiny-shoji.h"

/* Heading resolution is quite coarse, which provides robust
 * resistance to noise from many sources.
 */
typedef int16_t Heading;
#define HEADING_LFT     -2
#define HEADING_CTR_LFT -1
#define HEADING_STEADY     0
#define HEADING_CTR_RHT  1
#define HEADING_RHT      2
#define HEADING_COUNT    5 

/* The heading is determined by ranking current sample
 * with respect to recent samples. The recent sample
 * window size assumes a side-to-side sweep period of 
 * about one second.
 */
#define ACCEL_SAMPLES 32 /* 960 ms */

typedef class AxisState {
public:
    bool invert;
    char id;
    uint16_t cycles = 0;
    om::Ticks lastState = 0;
    Heading nextHeading = HEADING_RHT;
    Heading heading = HEADING_STEADY;
    Heading lastHeading = HEADING_STEADY;
    bool center = false;
    int16_t data[ACCEL_SAMPLES];
    int16_t maxVal;
    int16_t minVal;
    float valFast;
    float valSlow;
    float dir = 0;

    AxisState(char id, bool invert=false);
    void addData(int16_t value, int16_t index, int16_t damping);
    void setHeading(int16_t rank, bool damped);
    void headingToString(char * buf);
    void print();
} AxisState;

typedef class Accel3Thread : om::Thread {
public:
    Accel3Thread(uint16_t msLoop=32, int16_t damping=10);
    void setup();
    void loop();
    AxisState xState = AxisState('x', false); // positive right
    AxisState yState = AxisState('y', false); // positive forward
    AxisState zState = AxisState('z', true);  // positive up

protected:
    int16_t damping;
    uint16_t msLoop;
    int iSample = 0;
} Accel3Thread;


extern Accel3Thread accelThread;


#endif
