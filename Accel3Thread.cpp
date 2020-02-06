#include <Arduino.h>
#ifdef CMAKE
#include <cstring>
#endif 
#include "src/omilli/OMilli.h"
#include "src/tinycircuits/BMA250.h"       
#include "Accel3Thread.h"

/////////////// AxisState /////////////////

#define MAX_CYCLE_TICKS 1500

AxisState::AxisState(char id, bool invert) 
    :   id(id), invert(invert), valFast(0), valSlow(0), 
        maxVal(0), minVal(0)
{
    for (int i = 0; i < ACCEL_SAMPLES; i++) {
        data[i] = 0;
    }
}

void AxisState::addData(int16_t value, int16_t index, int16_t damping) {
    index = index % ACCEL_SAMPLES;
    int16_t curValue = data[index];
    data[index] = value;

    int16_t rank = 0;
    valFast = expAvg(value, valFast, EATC_1);
    valSlow = expAvg(value, valSlow, EATC_2);
    dir = valFast < valSlow ? -1 : 1;
    if (valFast < valSlow) {
        dir = 0;
    } else {
        dir = 1;
    }
    maxVal = minVal = value;
    for (int i = 0; i < ACCEL_SAMPLES; i++) {
        if (data[i] < minVal) { minVal = data[i]; }
        if (maxVal < data[i]) { maxVal = data[i]; }
        if (value >= data[i]) { rank++; }
    }

    rank = (100*rank)/ACCEL_SAMPLES;
    int range = maxVal - minVal;
    setHeading(rank, range <= damping);
}

#define SWEEP_END 20
void AxisState::setHeading(int16_t rank, bool damped) {
    Heading h;
    if (damped) {
        h = HEADING_STEADY;
    } else if (rank <= SWEEP_END) {
        h = HEADING_LFT;
    } else if (rank <= 50) {
        h = HEADING_CTR_LFT;
    } else if (rank <= 100 - SWEEP_END) {
        h = HEADING_CTR_RHT;
    } else {
        h = HEADING_RHT;
    }
    if (invert) {
        h = -h;
    }
    if (h*heading < 0) {
      center = true;
    }

    if (h == nextHeading) {
        om::Ticks now = om::ticks();
        if (now - lastState > MAX_CYCLE_TICKS) {
            h = HEADING_STEADY;
        }
        nextHeading = nextHeading == HEADING_RHT 
            ? HEADING_LFT : HEADING_RHT;
        lastState = now;
    }
    if (heading != h) {
        lastHeading = heading;
        heading = h;
    }
}

void AxisState::print() {
    char buf[6];
    this->headingToString(buf);
    om::print(buf);
    om::print(" ");
    om::print(nextHeading);
}

void AxisState::headingToString(char *buf) {
    switch (heading) {
    case HEADING_LFT: sprintf(buf, "%c-:--", id); break;
    case HEADING_CTR_LFT: sprintf(buf, "-%c:--", id); break;
    case HEADING_STEADY: sprintf(buf, "--%c--", id); break;
    case HEADING_CTR_RHT: sprintf(buf, "--:%c-", id); break;
    case HEADING_RHT: sprintf(buf, "--:-%c", id); break;
    default: sprintf(buf, "??%c??", id); break;
    }
}

//////////////////// Accel3Thread ////////////////////

Accel3Thread accelThread; // Acceleromoter tracker

// Accelerometer sensor variables for the sensor and its values
BMA250 accel_sensor;

Accel3Thread::Accel3Thread(uint16_t msLoop, int16_t damping)
    : msLoop(msLoop), damping(damping)
{}

void Accel3Thread::setup() {
    id = 'M';
    Thread::setup();
    accel_sensor.begin(BMA250_range_2g, BMA250_update_time_32ms); 
    om::print("Accel3Thread.setup");
}

void Accel3Thread::loop() {
    nextLoop.ticks = om::ticks() + MS_TICKS(msLoop);
    accel_sensor.read();
    double temp = ((accel_sensor.rawTemp * 0.5) + 24.0);
    int x = accel_sensor.X;
    int y = accel_sensor.Y;
    int z = accel_sensor.Z;

    if (x == -1 && y == -1 && z == -1) {
        om::print("ERROR! NO BMA250 DETECTED!");
    } else {
        iSample = (iSample+1) % ACCEL_SAMPLES;
        xState.addData(x, iSample, damping);
        yState.addData(y, iSample, damping);
        zState.addData(z, iSample, damping);
    }
}
