#include <Arduino.h>
#include "Thread.h"

using namespace om;

#define MAX_THREADS 32

namespace om {
    ThreadRunner 	threadRunner;
    struct Thread *	pThreadList;
    int 			nThreads;
    int32_t 		nLoops;
    int32_t 		nTardies;
    int16_t			leastFreeRam = 32767;
};

void Thread::setup() {
    bool active = false;
    for (ThreadPtr pThread = pThreadList; pThread; pThread = pThread->pNext) {
        if (pThread == this) {
            active = true;
            break;
        }
    }

    if (!active) {
        pNext = pThreadList;
        pThreadList = this;
        if (id == 0) {
            id = 'a' + nThreads;
        }
        nThreads++;
        if (nThreads >= MAX_THREADS) {
            Error("SC", MAX_THREADS);
        }
    }
}

void PulseThread::setup(Ticks period, Ticks pulseWidth) {
    Thread::setup();
    if (pulseWidth == 0) {
        m_HighPeriod = period / 2;
    } else {
        m_HighPeriod = pulseWidth;
    }
    m_LowPeriod = period - m_HighPeriod;
}

void PulseThread::loop() {
    isHigh = !isHigh;
    if (isHigh) {
        nextLoop.ticks = ticks() + m_HighPeriod;
    } else {
        nextLoop.ticks = ticks() + m_LowPeriod;
    }
}

void MonitorThread::setup(int pinLED) {
    id = 'Z';
    // set monitor interval to not coincide with timer overflow
    PulseThread::setup(MS_TICKS(1000), MS_TICKS(250));
    this->pinLED = pinLED;
    verbose = false;
    om::println("MonitorThread::setup()");
    if (pinLED != NOPIN) {
        om::pinMode(pinLED, OUTPUT);
    }
    blinkLED = true;
}

void MonitorThread::quiet(bool q) {
    blinkLED = (q == false);
}

void MonitorThread::LED(uint8_t value) {
    if (pinLED != NOPIN) {
        om::digitalWrite(pinLED, value ? HIGH : LOW);
    }
}

void MonitorThread::Error(const char *msg, int value) {
    LED(HIGH);
    for (int i = 0; i < 20; i++) {
        om::print('>');
    }
    om::print(msg);
    om::println((int16_t) value);
}

void MonitorThread::loop() {
    PulseThread::loop();
#define MONITOR
#ifdef MONITOR
    ThreadEnable(false);
    if (isHigh) {
        LED(blinkLED);
    } else {
        LED(LOW);
    }
    if (nTardies > 50) {
        Error("T", nTardies);
        for (ThreadPtr pThread = pThreadList; pThread; pThread = pThread->pNext) {
            om::print(pThread->id);
            om::print(":");
            om::print(pThread->tardies, DEC);
            pThread->tardies = 0;
            om::print(" ");
        }
        om::println('!');
    } else if (nTardies > 20) {
        LED(HIGH);
        verbose = true;
    }
    if (isHigh) {
        if (verbose) {
            om::print(".");
        }
        nTardies = 0;
    }
    ThreadEnable(true);
#endif
}

MonitorThread om::monitor;

void om::Error(const char *msg, int value) {
    monitor.Error(msg, value);
}

ThreadRunner::ThreadRunner() {
    clear();
}

void ThreadRunner::clear() {
    pThreadList = NULL;
    nThreads = 0;
    nLoops = 0;
    nTardies = 0;
    nHB = 0;
    testTardies = 0;
    fast = 255;
}

void ThreadRunner::setup(int pinLED) {
    monitor.setup(pinLED);
    ThreadEnable(true);
}

void om::ThreadEnable(bool enable) {
#ifdef DEBUG_ThreadENABLE
    for (ThreadPtr pThread = pThreadList; pThread; pThread = pThread->pNext) {
        om::print(pThread->id);
        om::print(":");
        om::print(pThread->nextLoop.ticks, DEC);
        om::print(" ");
    }
#endif
}

om::Ticks om::ticks() {
#if defined(TEST)
    arduino.timer64us(1);
#endif
    Ticks result = threadRunner.ticks();

    if (result == 0) {
        result = threadRunner.ticks();
    }
    return result;
}
