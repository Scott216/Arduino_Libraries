If using move mode or shake mode with Leonardo, don't use interrupt pins D2 or D3 because they are used by I2C.

On a Leonardo the dataMode example worked great

Library source: http://github.com/n0m1/MMA8453_n0m1

Blog Posts about using accelerometer:
http://n0m1.com/2012/02/12/shake-rattle-roll-the-mma8453q-arduino/
http://n0m1.com/2012/04/08/locomotion-mma8453-with-interrupts/


Library was updated on Github to use wire.h library
The DSS I2C version is on http://github.com/bilbolodz/MMA8451
 