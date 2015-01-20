/************************************************************************************
 * 	
 * 	Name    : MMA8453_n0m1 Library Example: DataMode                       
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
#include <MMA8451_n0m1.h>  // http://github.com/bilbolodz/MMA8451

MMA8451_n0m1 accel;

void setup()
{
  Serial.begin(9600);
  accel.setI2CAddr(0x1D); //change your device address if necessary, default is 0x1C
  accel.dataMode(true, 2); //enable highRes 10bit, 2g range [2g,4g,8g]
  Serial.println("MMA8453_n0m1 library");
  Serial.println("XYZ Data Example");
  Serial.println("n0m1.com");
}

void loop()
{
  accel.update();
  
  Serial.print("x: ");
  Serial.print(accel.x());
  Serial.print(" y: ");
  Serial.print(accel.y());
  Serial.print(" z: ");
  Serial.println(accel.z());
  delay(200);

}



