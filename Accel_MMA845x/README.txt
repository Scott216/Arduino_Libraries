Name    : MMA8451_n0m1 Library                         
Author  : Noah Shibley, Michael Grant, NoMi Design Ltd. http://n0m1.com, modified for MMA8451 by bilbolodz bilbo@lodz.pl
Date    : Feb 10th 2012                                    
Version : 0.1                                              
Notes   : Arduino Library for use with the Freescale MMA8453Q via i2c. 
          Some of the lib source from Kerry D. Wong
	  http://www.kerrywong.com/2012/01/09/interfacing-mma8453q-with-arduino/
	  This library has been tested with the Freescale MMA8453Q Accelerometer
	  chip, however the Freescale MMA8451, and MMA8452 probably also work.
	  Refer to the examples for how to use this library.  
			
Dependencies:	
	 I2C library:
	 http://dsscircuits.com/articles/arduino-i2c-master-library.html
	
	
			

Bilbolodz:
Examples NOT modified!

List of Functions:

Function: setI2CAddr
Description: set the i2c device address to a new value, default is 0x1C
Parameters: (int) device address: a common value is 0x1D
------------------------------------------------------	
Function: dataMode
Description: set the device to return raw data values
Parameters: (boolean) enable highRes 10bit,
	    (int) set g force range: [2g,4g,8g]
------------------------------------------------------
Function: x
Return: (int) returns the x axis value
------------------------------------------------------
Function: y
Return: (int) returns the y axis value
------------------------------------------------------ 
Function: z
Return: (int) returns the z axis value
------------------------------------------------------
Function: shakeMode
Description: set to transient detection mode
Parameters: (int) threshold [0-127] formula: 6g/ 0.063g = 95.2 counts, round to 96,
   	    (boolean) enable X, 
   	    (boolean) enable Y,
   	    (boolean) enable Z, 
   	    (boolean) enable MMA8453Q INT pin 2 (false= pin 1), 
   	    (int) arduino INT pin number
------------------------------------------------------
Function: shake
Return: (boolean) returns true if there is shaking (high pass filtered motion)
------------------------------------------------------
Function: shakeAxisX
Return: (boolean) returns true if there is shake on the x axis
------------------------------------------------------
Function: shakeAxisY
Return: (boolean) returns true if there is shake on the y axis
------------------------------------------------------
Function: shakeAxisZ
Return: (boolean) returns true if there is shake on the z axis
------------------------------------------------------ 
Function: motionMode
Description: set to motion detection mode
Parameters: (int) threshold [0-127] formula: 6g/ 0.063g = 95.2 counts, round to 96,
   	    (boolean) enable X, 
   	    (boolean) enable Y,
   	    (boolean) enable Z, 
   	    (boolean) enable MMA8453Q INT pin 2 (false= pin 1), 
   	    (int) arduino INT pin number
------------------------------------------------------  
Function: motion
Return: (boolean) returns true if there is motion
------------------------------------------------------ 
Function: update
Description: update data values, or clear interrupts. Use at start of loop()
------------------------------------------------------ 
Function: regRead
Description: read one of the accelerometer registers
------------------------------------------------------ 
Function: regWrite
Description: set one of the accelerometer registers
------------------------------------------------------
Function: setRangeSetting
Parameters: (int) gScaleRange set g force range: [2g,4g,8g]
Description: Compatibilty Function
------------------------------------------------------
Function: setFullResBit
Parameters: (boolean) highRes enable highRes 10bit
Description: Compatibilty Function
------------------------------------------------------
Function: readAccel
Parameters: (int) *x pass x pointer
	    (int) *y pass y pointer
	    (int) *z pass z pointer
Description: Compatibility Function

