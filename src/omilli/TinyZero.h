#ifndef OM_TINYZERO_H
#define OM_TINYZERO_H

#include <Arduino.h>

namespace om {
	inline Print& get_Print() {
		return SerialUSB;
	}
	inline int16_t serial_read() {
		return SerialUSB.read();
	}
	inline int16_t serial_available() {
		return SerialUSB.available();
	}
	inline void serial_begin(int32_t baud) {
		SerialUSB.begin(baud);
	}
	inline void print(const char *value) {
		SerialUSB.print(value);
	}
	inline void print(const char value) {
		SerialUSB.print(value);
	}
  inline void print(int8_t value, int16_t format = DEC) {
    SerialUSB.print(value, format);
  }
  inline void print(uint8_t value, int16_t format = DEC) {
    SerialUSB.print(value, format);
  }
  inline void print(int16_t value, int16_t format = DEC) {
    SerialUSB.print((float) value, 0);
  }
  inline void print(uint16_t value, int16_t format = DEC) {
    SerialUSB.print((float) value, 0);
  }
  inline void print(int32_t value, int16_t format = DEC) {
    SerialUSB.print((double) value, 0);
  }
  inline void print(uint32_t value, int16_t format = DEC) {
    SerialUSB.print((double) value, 0);
  }
  inline void print(float value, int16_t format = 2) {
    SerialUSB.print(value, format);
  }
  inline void print(double value, int16_t format = 3) {
    SerialUSB.print(value, format);
  }
  inline void pinMode(int16_t pin, int16_t inout) {
		::pinMode(pin, inout);
	}
	inline int16_t digitalRead(int16_t pin) {
		return ::digitalRead(pin);
	}
	inline void digitalWrite(int16_t dirPin, int16_t value) {
		::digitalWrite(dirPin, value);
	}
	inline void analogWrite(int16_t dirPin, int16_t value) {
		::analogWrite(dirPin, value);
	}
	inline int16_t analogRead(int16_t dirPin) {
		return ::analogRead(dirPin);
	}
	inline void delay(int ms) {
		::delay(ms);
	}
	inline void delayMicroseconds(uint16_t usDelay) {
		::delayMicroseconds(usDelay);
	}
	inline uint32_t millis() {
		return ::millis();
	}

    void setI2CPort(uint8_t port);
} // namespace om


#endif
