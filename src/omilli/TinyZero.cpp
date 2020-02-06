#ifndef OMILLI_TINYZERO_H
#define OMILLI_TINYZERO_H

#include "Arduino.h"
#include <Wire.h>
#include "TinyZero.h"



namespace om {

// **Wireling boards attached through an Adapter board**
// Selects the correct address of the port being used in the Adapter board
void setI2CPort(uint8_t port) {
  Wire.beginTransmission(0x70);
  Wire.write(0x04 + port);
  Wire.endTransmission();
}

} // om


#endif
