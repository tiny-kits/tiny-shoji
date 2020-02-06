 #include <Arduino.h>
#include <Wire.h>         // For I2C communication with sensor
#include <Wireling.h>               // For interfacing with Wirelings
#include "tiny-shoji.h"
#include "Accel3Thread.h"
#include "RangeThread.h"
#include "LEDThread.h"

/////////// APPLICATION ///////////

// The "om" namespace gives us access to the OMilli operating system,
// which lets us handle different Arduino boards in exactly the same way.
// Since this application uses TinyCircuit components, OMilli will 
// detect the proper board configuration for the src/omilli/TinyZero 
// implementation of OMilli.
using namespace om;

// The production version of the application uses an RGB LED.
// For debugging, om::print() or om::println() are normally sufficient,
// but more may be required. It is also possible to hook up a
// TinyCircuit display such as the AST1042 0.42" OLED. To use
// the OLED display, simply plug it into I2C port 0 and change
// the following define to "true".
#define DISPLAY_OLED false

// Main application setup
void setup() { 
    // Serial I/O has lowest priority, so you may need to
    // decrease baud rate to fix Serial I/O problems.
    //om::serial_begin(19200);  // very long USB cables
    om::serial_begin(38400);    // 3' USB cables
    
    // Standard I2C is 100kbit/second, which can support transmission
    // lengths of a meter or longer. TinyCircuit cables are much shorter,
    // so we can set up I2C for the higher 400kbit/second bandwidth.
    // Using higher I2C speeds gives us rapid and crisp interactions 
    // with peripheral devices.
    Wire.begin();
    Wire.setClock(400000); 
    om::pinMode(LED_BUILTIN, OUTPUT);

    // Initialize threads
    accelThread.setup();
    rangeThread.setup(I2CPORT_TOF, VL53L0X_PERIOD);
    ledThread.setup(I2CPORT_DISPLAY, LED_FRAMERATE_PLA);
    threadRunner.setup(LED_BUILTIN);
}

// Main application loop
void loop() { 
    threadRunner.run(); // run all initialized threads
}
