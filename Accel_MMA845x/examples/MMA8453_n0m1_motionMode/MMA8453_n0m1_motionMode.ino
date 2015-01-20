/************************************************************************************
 * 	
 * 	Name    : MMA8453_n0m1 Library Example: MotionMode                       
 * 	Author  : Noah Shibley, Michael Grant, NoMi Design Ltd. http://n0m1.com                       
 * 	Date    : Feb 10th 2012                                    
 * 	Version : 0.1                                              
 * 	Notes   : Arduino Library for use with the Freescale MMA8453Q via i2c. 
 *
 ***********************************************************************************/
// Blog post with more info
// http://n0m1.com/2012/02/12/shake-rattle-roll-the-mma8453q-arduino/
// http://n0m1.com/2012/04/08/locomotion-mma8453-with-interrupts/

// Source
// Original Author (lib now uses wire) http://github.com/n0m1/MMA8453_n0m1
// Forked https://github.com/bilbolodz/MMA8451?source=c


#include <I2C.h>  // http://dsscircuits.com/articles/arduino-i2c-master-library.html
#include <MMA8451_n0m1.h>  // http://github.com/n0m1/MMA8453_n0m1
                           // http://github.com/bilbolodz/MMA8451

#define MMA8453Q_INT1 false  // Use interrupt pin 1 on accelerometer
#define MMA8453Q_INT2 true   // Use interrupt pin 2 on accelerometer
#define ARDUINO_INTPIN2  2   // Use pin D2 on Arduino for Interrupt - Don't use on Leonardo
#define ARDUINO_INTPIN3  3   // Use pin D3 on Arduino for Interrupt - Don't use on Leonardo
#define ARDUINO_INTPIN3  7   // Use pin D7 on Arduino for Interrupt - LEONARDO Only



MMA8451_n0m1 accel;

void setup()
{
  Serial.begin(9600);
  accel.setI2CAddr(0x1D); //change your device address if necessary, default is 0x1C
  /*
  threshold [0-127] formula: 0.5g/ 0.063g = 7.9 counts, round to 8 counts.  4 seems to work well
  enable X, 
  enable Y,
  enable Z, 
  enable MMA8453Q INT pin 2 (true=pin2, false= pin 1), 
  arduino INT pin number
  */
  accel.motionMode(4, true, true, true, MMA8453Q_INT1, ARDUINO_INTPIN2 );
  
  Serial.println("MMA8453_n0m1 library");
  Serial.println("Motion Example");
  Serial.println("n0m1.com");
}

void loop()
{
  accel.update();
  
  if(accel.motion())
  {
   Serial.println("motion!");
    
  }
  
  
  
}


