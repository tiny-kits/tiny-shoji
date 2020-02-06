#ifndef OMILLI_H
#define OMILLI_H

#define NOPIN -1
#define minval(a,b) ((a)<(b)?(a):(b))
#define maxval(a,b) ((a)>(b)?(a):(b))
#define absval(x) ((x)>0?(x):-(x))
#define roundval(x)     ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
#ifndef radians
#define radians(deg) ((deg)*DEG_TO_RAD)
#define degrees(rad) ((rad)*RAD_TO_DEG)
#endif
#ifndef PI
#define PI 3.1415926535897932384626433832795
#endif

/* Exponential Averaging 
 * =====================
 * Exponential averaging provides a cheap way to analyze
 * series of time values. They require only a single memory
 * location to store the exponentially weighted average of all
 * preceding values. Exponential averages are computed using
 * an Exponential Average Time Constant (EATC) that 
 * controls the stability vs. sensitivity of the average. 
 * For convenience, we arbitrarily choose a 0-to-1 transition
 * to determine actual time constant values:
 *
 * 1) number of periods for a 0-to-1 transition to be >=0.5
 * 2) number of periods for a 0-to-1 transition to be >=0.999
 *
 * More periods provide stability. 
 * Fewer periods enhance sensitivity.
 */
#define expAvg(v,ea,eatc) ((v)*eatc+(1-eatc)*(ea))
#define EATC_0 0.50 /* 0/10 periods */
#define EATC_1 0.49 /* 1/11 periods */
#define EATC_2 0.29 /* 2/21 periods */
#define EATC_3 0.20 /* 3/31 periods */
#define EATC_4 0.15 /* 4/44 periods */
#define EATC_5 0.12 /* 5/55 periods */
#define EATC_6 0.10 /* 6/66 periods */

#if defined(MOCK_ARDUINO)
#define NO_MCU
#include "MockDuino.h"
#elif defined( ARDUINO_SAMD_ZERO )
#include "TinyZero.h"
#else
#define NO_MCU
namespace om { // abstract API implementable any way you like
	//////////////////// ARDUINO SPECIFIC ///////////////////
	Print& get_Print();
	int16_t serial_read();
	int16_t serial_available();
	void serial_begin(int32_t baud);
	void print(const char *value);
	void print(const char value);
    void print(uint8_t value, int16_t format = DEC);
    void print(int8_t value, int16_t format = DEC);
    void print(uint16_t value, int16_t format = DEC);
	void print(int16_t value, int16_t format = DEC);
    void print(uint32_t value, int16_t format = DEC);
	void print(int32_t value, int16_t format = DEC);
    void print(float value, int16_t places = 3);
    void print(double value, int16_t places = 3);
	void pinMode(int16_t pin, int16_t inout);
	int16_t digitalRead(int16_t pin);
	void digitalWrite(int16_t dirPin, int16_t value);
	void analogWrite(int16_t dirPin, int16_t value);
	int16_t analogRead(int16_t dirPin);
	uint32_t millis();
	void delay(int ms);
	void delayMicroseconds(uint16_t usDelay);

	////////////////// OTHER ///////////////////
    void setI2CPort(uint8_t port);

} // namespace om
#endif

////////////////////// API Extensions ////////////////////
namespace om {
    inline void println() {
        om::print('\n');
    }
    inline void println(int8_t value, int16_t format=DEC) {
				//om::print("println int8_t");
        om::print(value, format);
        om::print('\n');
    }
    inline void println(uint8_t value, int16_t format=DEC) {
				//om::print("println uint8_t");
        om::print(value, format);
        om::print('\n');
    }
    inline void println(int16_t value, int16_t format=DEC) {
				//om::print("println int16_t");
        om::print(value, format);
        om::print('\n');
    }
    inline void println(uint16_t value, int16_t format=DEC) {
				//om::print("println uint16_t");
        om::print(value, format);
        om::print('\n');
    }
    inline void println(int32_t value, int16_t format=DEC) {
				//om::print("println int32_t");
        om::print(value, format);
        om::print('\n');
    }
    inline void println(uint32_t value, int16_t format=DEC) {
				//om::print("println uint32_t");
        om::print(value, format);
        om::print('\n');
    }
    inline void println(float value, int16_t places=2) {
				//om::print("println float");
        om::print(value, places);
        om::print('\n');
    }
    inline void println(double value, int16_t places=3) {
				//om::print("println double");
        om::print(value, places);
        om::print('\n');
    }
    inline void println(const char value) {
				//om::print("println const char");
        om::print(value);
        om::print('\n');
    }
    inline void println(const char* value) {
				//om::print("println const char*");
        om::print(value);
        om::print('\n');
    }
} // namespace om


// Testing macros are useful for MOCK_ARDUINO 
#ifdef TEST
#define TESTCOUT1(k,v) cout << k << v << endl
#define TESTCOUT2(k1,v1,k2,v2) cout << k1<<v1 <<k2<<v2 << endl
#define TESTCOUT3(k1,v1,k2,v2,k3,v3) cout << k1<<v1 <<k2<<v2 <<k3<< v3 << endl
#define TESTCOUT4(k1,v1,k2,v2,k3,v3,k4,v4) cout << k1<<v1 <<k2<<v2 <<k3<<v3 <<k4<<v4 << endl
#define TESTDECL(t,v) t v
#define TESTEXP(e) e
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#define DIE() kill(getpid(), SIGABRT)
#else
#define TESTCOUT1(k,v)
#define TESTCOUT2(k1,v1,k2,v2)
#define TESTCOUT3(k1,v1,k2,v2,k3,v3)
#define TESTCOUT4(k1,v1,k2,v2,k3,v3,k4,v4)
#define TESTDECL(t,v)
#define TESTEXP(e)
#define DIE()
#endif

#define DEBUG_EOL() om::println("");
#define DEBUG_HEX(S,V) om::print(" " S ":");om::print(V,HEX);
#define DEBUG_DEC(S,V) om::print(" " S ":");om::print(V,DEC);

#define TICKS_PER_SECOND ((int32_t)1000)
#define MS_TICKS(ms) (1*ms) /* Convert milliseconds to Ticks */

////////////////////////////// OMILLI_H /////////////////////////
#endif
