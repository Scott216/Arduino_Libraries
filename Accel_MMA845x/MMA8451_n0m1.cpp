/************************************************************************************
 * 	
 * 	Name    : MMA8453_n0m1 Library mod by Bilbolodz                         
 * 	Author  : Noah Shibley, Michael Grant, NoMi Design Ltd. http://n0m1.com                       
 * 	Date    : Jun 07th 2012                                    
 * 	Version : 0.11                                             
 * 	Notes   : Arduino Library for use with the Freescale MMA8451Q via i2c. 
              Some of the lib source from Kerry D. Wong
			  http://www.kerrywong.com/2012/01/09/interfacing-mma8451q-with-arduino/
 * 
 * 
 * 	This file is part of MMA8451_n0m1.
 * 
 * 		    MMA8451_n0m1 is free software: you can redistribute it and/or modify
 * 		    it under the terms of the GNU General Public License as published by
 * 		    the Free Software Foundation, either version 3 of the License, or
 * 		    (at your option) any later version.
 * 
 * 		    AtTouch is distributed in the hope that it will be useful,
 * 		    but WITHOUT ANY WARRANTY; without even the implied warranty of
 * 		    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * 		    GNU General Public License for more details.
 * 
 * 		    You should have received a copy of the GNU General Public License
 * 		    along with MMA8451_n0m1.  If not, see <http://www.gnu.org/licenses/>.
 * 
 ***********************************************************************************/

#include "MMA8451_n0m1.h"

MMA8451_n0m1* MMA8451_n0m1::pMMA8451_n0m1 = 0; 

MMA8451_n0m1::MMA8451_n0m1()
{
	pMMA8451_n0m1 = this;
	dataMode_ = false;
	shakeMode_ = false; 
	ISRFlag = false;
	shake_ = false;
	shakeAxisX_ = false;
	shakeAxisY_ = false;
	shakeAxisZ_ = false;
	I2CAddr = 0x1c; //The i2C address of the MMA8451 chip. 0x1D is another common value.
	gScaleRange_ = 2;  //default 2g
	debug = false;
	off_x=0;
	off_y=0;
	off_z=0;
}

/***********************************************************
 * 
 * setI2CAddr
 *
 * 
 *   
 ***********************************************************/
void MMA8451_n0m1::setI2CAddr(int address)
{
	I2CAddr = address; I2CAddr; 
}

/***********************************************************
 * 
 * update
 *
 * 
 *   
 ***********************************************************/
void MMA8451_n0m1::update()
{
if (debug) Serial.print("update() ");
	if(dataMode_)
	{
		if (debug) Serial.println("in dataMode_");
		xyz(x_,y_,z_);
		measure_time_=millis();
	}
	
	if(shakeMode_ == true || motionMode_ == true || dataModeInt_ == true)
	{
		if (debug) Serial.println("interrupt driven");
		clearInterrupt();
	}
	
}

/***********************************************************
 * 
 * clearInterrupt
 *
 * 
 *   
 ***********************************************************/
void MMA8451_n0m1::clearInterrupt()
{
	
	
if (debug) Serial.print("(clearInterrupt) ");
  if(ISRFlag)
  {
	if (debug) Serial.println(" ISRFlag ON");
	 byte sourceSystem;
	 byte error_code=I2c.read(I2CAddr,REG_INT_SOURCE,byte(1),&sourceSystem); //check which system fired the interrupt
	 if (debug) {
	    Serial.print("I2c.read return code: ");
	    Serial.print(error_code,HEX);
	    Serial.print(" Source of interrupt");
	    Serial.println(sourceSystem,HEX);
	    }
	
     	if((sourceSystem&0x80) == 0x80) {} //Auto-sleep/Wake
     	if((sourceSystem&0x40) == 0x40) {} //FIFO
	if((sourceSystem&0x20) == 0x20) //Transient
	{	
		  //Perform an Action since Transient Flag has been set
	      //Read the Transient to clear system interrupt and Transient
	      byte srcTrans;
		  shake_ = true;
	      I2c.read(I2CAddr,REG_TRANSIENT_SRC ,byte(1),&srcTrans);
		
		  if((srcTrans&0x02) == 0x02)
		  {
			shakeAxisX_ = true;
		  }
		  if((srcTrans&0x08) == 0x08)
		  {
			shakeAxisY_ = true;
		  }
		  if((srcTrans&0x20) == 0x20)
		  {
			shakeAxisZ_ = true;
		  }
//		xyz(x_,y_,z_);
	}
	
	if((sourceSystem&0x10) == 0x10) {} //Landscape/Portrait
	if((sourceSystem&0x08) == 0x08) {} //Pulse
	
	if((sourceSystem&0x04) == 0x04) //FreeFall Motion
	{
	      byte srcFF;
	      I2c.read(I2CAddr,REG_FF_MT_SRC ,byte(1),&srcFF);
		motion_ = true;
//		xyz(x_,y_,z_);
	
	}
	
	if((sourceSystem&0x01) == 0x01) //DateReady
	{
	if (debug) Serial.println("Processing Dataready interrupt");
	detachISRProc();
	xyz(x_,y_,z_);
	attachISRProc();
	dataready_ = true;
	}
	ISRFlag = false; 
  }
    else {
if (debug) {
Serial.println(" ISRFlag OFF dumping accelrator registers ");

  byte buf[0x32];
    byte error_code = I2c.read(I2CAddr, REG_OUT_X_MSB, 0x32, buf);


	    Serial.print("I2c.read return code: ");
	    Serial.println(error_code,HEX);

for (byte reg_add =0;reg_add <=0x31;reg_add++) {
	Serial.print(reg_add,HEX);
	Serial.print(" - ");
	Serial.println(buf[reg_add],HEX);
}



}
}

}

/***********************************************************
 * 
 * attachISRProc
 *
 * 
 *   
 ***********************************************************/
void MMA8451_n0m1::attachISRProc()
{
	if (debug) Serial.println("attachISRProc()");
  
  switch (arduinoINTPin_) {
    case 2:
      attachInterrupt(0,accelISR,FALLING); 
      break;
    case 3:
      attachInterrupt(1,accelISR,FALLING); 
      break;
    case 7:  // for Leonardo only 
      attachInterrupt(4,accelISR,FALLING); 
      break;
    default:
     #ifdef PINCHANGE_INT
		   	pinMode(arduinoINTPin, INPUT); digitalWrite(arduinoINTPin, HIGH);
		   	PCintPort::attachInterrupt(arduinoINTPin,accelISR,FALLING);  
      #else
		 	  Serial.println("no INT on pin, define PINCHANGE_INT");	
      #endif
      break;
  }
}



/***********************************************************
 * 
 * detachISRProc
 *
 * 
 *   
 ***********************************************************/
// SRG Modified: Changed to switch() and added support for Leonardo pin D7
void MMA8451_n0m1::detachISRProc()
{
if (debug) Serial.println("detachISRProc()");
  switch (arduinoINTPin_) {
    case 2:
      detachInterrupt(0); 
      break;
    case 3:
      detachInterrupt(1); 
      break;
    case 7:
      detachInterrupt(4);  // for Leonardo only 
      break;
    default:
      #ifdef PINCHANGE_INT
		 	  pinMode(arduinoINTPin, INPUT); digitalWrite(arduinoINTPin, HIGH);
		 	  PCintPort::attachInterrupt(arduinoINTPin,accelISR,FALLING);  
      #else
		 	  Serial.println("no INT on pin, define PINCHANGE_INT");	
     #endif
      break;
  }
}



/*************************************************************
* 
* xyz
* 
* Get accelerometer readings (x, y, z)
* by default, standard 14 bits mode is used.
* 
* This function also convers 2's complement number to
* signed integer result.
* 
* If accelerometer is initialized to use low res mode,
* isHighRes must be passed in as false.
*
*************************************************************/
void MMA8451_n0m1::xyz(int& x, int& y, int& z)
{

if (debug) Serial.println("xyz()");

  byte buf[6];

  if (highRes_) 
  {
    byte error_code = I2c.read(I2CAddr, REG_OUT_X_MSB, 6, buf);
if (debug) {
	    Serial.print("highRes_ I2c.read return code: ");
	    Serial.println(error_code,HEX);
}

    x = buf[0] << 6 | buf[1] >> 2 & 0x3F;
    y = buf[2] << 6 | buf[3] >> 2 & 0x3F;
    z = buf[4] << 6 | buf[5] >> 2 & 0x3F;
  }
  else 
  {
    byte error_code = I2c.read(I2CAddr, REG_OUT_X_MSB, 3, buf);
if (debug) {
	    Serial.print("lowhRes_ I2c.read return code: ");
	    Serial.println(error_code,HEX);
}
    x = buf[0] << 6;
    y = buf[1] << 6;
    z = buf[2] << 6;
  }


  if (x > 8191) x = x - 16384;
  if (y > 8191) y = y - 16384;
  if (z > 8191) z = z - 16384;

}

/***********************************************************
 * 
 * dataMode
 *
 * 
 *   
 ***********************************************************/
void MMA8451_n0m1::dataMode(boolean highRes, int gScaleRange)
{
	highRes_ = highRes;
	gScaleRange_ = gScaleRange;
	dataMode_ = true;
	byte statusCheck;
	
	//setup i2c
	I2c.begin();
	
	//register settings must be made in standby mode
	I2c.read(I2CAddr,REG_CTRL_REG1,byte(1),&statusCheck);
    I2c.write(I2CAddr, REG_CTRL_REG1, byte(statusCheck & ~activeMask));
	
	if( gScaleRange_ <= 3){ gScaleRange_ = FULL_SCALE_RANGE_2g; } //0-3 = 2g
	else if( gScaleRange_ <= 5){ gScaleRange_ = FULL_SCALE_RANGE_4g; } //4-5 = 4g
	else if( gScaleRange_ <= 8){ gScaleRange_ = FULL_SCALE_RANGE_8g; }// 6-8 = 8g
	else if( gScaleRange_ > 8) { gScaleRange_ = FULL_SCALE_RANGE_8g; } //boundary
	I2c.write(I2CAddr,REG_XYZ_DATA_CFG, byte(gScaleRange_));
    
    //set highres 14bit or lowres 8bit
    I2c.read(I2CAddr,REG_CTRL_REG1,byte(1),&statusCheck);	
	if(highRes){
	    I2c.write(I2CAddr, REG_CTRL_REG1, byte(statusCheck & ~resModeMask));
	}
    else { 
  		I2c.write(I2CAddr, REG_CTRL_REG1, byte(statusCheck | resModeMask));	    
	}
    //active Mode
 	I2c.read(I2CAddr,REG_CTRL_REG1,byte(1),&statusCheck);
    I2c.write(I2CAddr, REG_CTRL_REG1, byte(statusCheck | activeMask));
}

/***********************************************************
 * 
 * dataModeInt
 *
 * 
 *   
 ***********************************************************/
// SRG Modified: Added support for Leonardo interrrupt on Pin D7
void MMA8451_n0m1::dataModeInt(boolean highRes, int gScaleRange, byte odr, boolean enableINT2,int arduinoINTPin) 
{
	enableINT2_ = enableINT2;
	arduinoINTPin_= arduinoINTPin;
	highRes_ = highRes;
	gScaleRange_ = gScaleRange;
	dataModeInt_ = true;
	byte statusCheck;
	boolean error = false;

  //DataSheet pg40, When IPOL is ‘0’ (default value) any interrupt event will signaled with a logical 0
  //Serial.println("Ustawiona oblsuga przerwania");

  if (debug) Serial.println("detachISRProc()");
  
  switch (arduinoINTPin_) {
    case 2:
      attachInterrupt(0,accelISR,FALLING); 
      break;
    case 3:
      attachInterrupt(1,accelISR,FALLING); 
      break;
    case 7:  // for Leonardo only 
      attachInterrupt(4,accelISR,FALLING); 
      break;
    default:
      #ifdef PINCHANGE_INT
		 	  pinMode(arduinoINTPin, INPUT); digitalWrite(arduinoINTPin, HIGH);
		 	  PCintPort::attachInterrupt(arduinoINTPin,accelISR,FALLING);  
      #else
		 	  Serial.println("no INT on pin, define PINCHANGE_INT");	
      #endif
      break;
  }
  
	 
	 //setup i2c
	I2c.begin();
	
	//register settings must be made in standby mode
	setStandby();
	
	setODR(odr); //Set device in ODR rate

	if( gScaleRange_ <= 3){ gScaleRange_ = FULL_SCALE_RANGE_2g; } //0-3 = 2g
	else if( gScaleRange_ <= 5){ gScaleRange_ = FULL_SCALE_RANGE_4g; } //4-5 = 4g
	else if( gScaleRange_ <= 8){ gScaleRange_ = FULL_SCALE_RANGE_8g; }// 6-8 = 8g
	else if( gScaleRange_ > 8) { gScaleRange_ = FULL_SCALE_RANGE_8g; } //boundary
	I2c.write(I2CAddr,REG_XYZ_DATA_CFG, byte(gScaleRange_));

    
    //set highres 14bit or lowres 8bit
    I2c.read(I2CAddr,REG_CTRL_REG1,byte(1),&statusCheck);	
    
	if(highRes){
	    I2c.write(I2CAddr, REG_CTRL_REG1, byte(statusCheck & ~resModeMask));
	}
    else { 
  		I2c.write(I2CAddr, REG_CTRL_REG1, byte(statusCheck | resModeMask));	    
	}
	 I2c.read(I2CAddr, REG_CTRL_REG4, byte(1), &statusCheck);
	 statusCheck |= 0x01;
	 I2c.write(I2CAddr, REG_CTRL_REG4, statusCheck);  //Enable Data Ready Interrupt in the System
	 byte intSelect = 0x01;
	 if(enableINT2) intSelect = 0x00;
	 I2c.read(I2CAddr, REG_CTRL_REG5, byte(1), &statusCheck);
	 
	 statusCheck |= intSelect;
	 I2c.write(I2CAddr, REG_CTRL_REG5, statusCheck); //INT2 0x0, INT1 0x01 
    //active Mode
    	setActive();
}



/***********************************************************
 * 
 * shakeMode
 *
 * 
 *   
 ***********************************************************/
void MMA8451_n0m1::shakeMode(int threshold, boolean enableX, boolean enableY, boolean enableZ, boolean enableINT2,int arduinoINTPin)
{

  //DataSheet pg40, When IPOL is ‘0’ (default value) any interrupt event will signaled with a logical 0
  
  switch (arduinoINTPin) {
    case 2:
      attachInterrupt(0,accelISR,FALLING); 
      break;
    case 3:
      attachInterrupt(1,accelISR,FALLING); 
      break;
    case 7:  // for Leonardo only 
      attachInterrupt(4,accelISR,FALLING); 
      break;
    default:
      #ifdef PINCHANGE_INT
		 	  pinMode(arduinoINTPin, INPUT); digitalWrite(arduinoINTPin, HIGH);
		 	  PCintPort::attachInterrupt(arduinoINTPin,accelISR,FALLING);  
      #else
		 	  Serial.println("no INT on pin, define PINCHANGE_INT");	
      #endif
      break;
  }
  
	 boolean error = false;
	 byte statusCheck;
	
	 //setup i2c
	 I2c.begin();
	 
	 I2c.write(I2CAddr, REG_CTRL_REG1, byte(0x18)); //Set device in 100 Hz ODR, Standby
	
	 byte xyzCfg = 0x10; //latch always enabled
	 if(enableX) xyzCfg |= 0x02;
	 if(enableY) xyzCfg |= 0x04;
	 if(enableZ) xyzCfg |= 0x08;
	
	 I2c.write(I2CAddr, REG_TRANSIENT_CFG, xyzCfg);  //XYZ + latch 0x1E
	 I2c.read(I2CAddr, REG_TRANSIENT_CFG, byte(1), &statusCheck);
	 if(statusCheck != xyzCfg) error = true;

	
	 if(threshold > 127) threshold = 127; //8g is the max.
	 I2c.write(I2CAddr, REG_TRANSIENT_THS, byte(threshold));  //threshold 
	 I2c.read(I2CAddr, REG_TRANSIENT_THS, byte(1), &statusCheck);
	 if(statusCheck != byte(threshold)) error = true;

	 
	 I2c.write(I2CAddr, REG_TRANSIENT_COUNT, byte(0x05)); //Set the Debounce Counter for 50 ms
	 I2c.read(I2CAddr,REG_TRANSIENT_COUNT, byte(1), &statusCheck);
	 if(statusCheck != 0x05) error = true;

	 I2c.read(I2CAddr, REG_CTRL_REG4, byte(1), &statusCheck);
	 statusCheck |= 0x20;
	 I2c.write(I2CAddr, REG_CTRL_REG4, statusCheck);  //Enable Transient Detection Interrupt in the System
	  	
	 byte intSelect = 0x20;
	 if(enableINT2) intSelect = 0x00;
	 I2c.read(I2CAddr, REG_CTRL_REG5, byte(1), &statusCheck);
	 statusCheck |= intSelect;
	 I2c.write(I2CAddr, REG_CTRL_REG5, statusCheck); //INT2 0x0, INT1 0x20 

	 I2c.read(I2CAddr, REG_CTRL_REG1, byte(1), &statusCheck); //Read out the contents of the register
	 statusCheck |= 0x01; //Change the value in the register to Active Mode.
	 I2c.write(I2CAddr, REG_CTRL_REG1, statusCheck); //Write in the updated value to put the device in Active Mode
	
	if(error)
	{
		Serial.println("Shake mode setup error");
		Serial.println("retrying...");
		delay(100);
		shakeMode(threshold,enableX,enableY,enableZ,enableINT2,arduinoINTPin);
	}
	
	shakeMode_ = true;
	
}
/***********************************************************
 * 
 * motionMode
 *
 * 
 *   
 ***********************************************************/
void MMA8451_n0m1::motionMode(int threshold, boolean enableX, boolean enableY, boolean enableZ, boolean enableINT2,int arduinoINTPin)
{

  //DataSheet pg40, When IPOL is ‘0’ (default value) any interrupt event will signaled with a logical 0
  switch (arduinoINTPin) {
    case 2:
      attachInterrupt(0,accelISR,FALLING); 
      break;
    case 3:
      attachInterrupt(1,accelISR,FALLING); 
      break;
    case 7:  // for Leonardo only 
      attachInterrupt(4,accelISR,FALLING); 
      break;
    default:
      #ifdef PINCHANGE_INT
		 	  pinMode(arduinoINTPin, INPUT); digitalWrite(arduinoINTPin, HIGH);
		 	  PCintPort::attachInterrupt(arduinoINTPin,accelISR,FALLING);  
      #else
		 	  Serial.println("no INT on pin, define PINCHANGE_INT");	
      #endif
      break;
  }
  
	 boolean error = false;
	 byte statusCheck;
	
	 //setup i2c
	 I2c.begin();
	 
	 I2c.write(I2CAddr, REG_CTRL_REG1, byte(0x18)); //Set device in 100 Hz ODR, Standby
	
	 byte xyzCfg = 0x80; //latch always enabled
	 xyzCfg |= 0x40; //Motion not free fall
	 if(enableX) xyzCfg |= 0x08;
	 if(enableY) xyzCfg |= 0x10;
	 if(enableZ) xyzCfg |= 0x20;
	
	 I2c.write(I2CAddr, REG_FF_MT_CFG, xyzCfg);  //XYZ + latch + motion
	 I2c.read(I2CAddr, REG_FF_MT_CFG, byte(1), &statusCheck);
	 if(statusCheck != xyzCfg) error = true;

	 if(threshold > 127) threshold = 127; //a range of 0-127.
	 I2c.write(I2CAddr, REG_FF_MT_THS, byte(threshold));  //threshold 
	 I2c.read(I2CAddr, REG_FF_MT_THS, byte(1), &statusCheck);
	 if(statusCheck != byte(threshold)) error = true;

	 I2c.write(I2CAddr, REG_FF_MT_COUNT, byte(0x0A)); //Set the Debounce Counter for 100 ms
	 I2c.read(I2CAddr,REG_FF_MT_COUNT, byte(1), &statusCheck);
	 if(statusCheck != 0x0A) error = true;

	 I2c.read(I2CAddr, REG_CTRL_REG4, byte(1), &statusCheck);
	 statusCheck |= 0x04;
	 I2c.write(I2CAddr, REG_CTRL_REG4, statusCheck); //Enable Motion Interrupt in the System

	 byte intSelect = 0x04;
	 if(enableINT2) intSelect = 0x00;
	 I2c.read(I2CAddr, REG_CTRL_REG5, byte(1), &statusCheck);
	 statusCheck |= intSelect;
	 I2c.write(I2CAddr, REG_CTRL_REG5, statusCheck); //INT2 0x0, INT1 0x04
	 
	 I2c.read(I2CAddr, REG_CTRL_REG1, byte(1), &statusCheck); //Read out the contents of the register
	 statusCheck |= 0x01; //Change the value in the register to Active Mode.
	 I2c.write(I2CAddr, REG_CTRL_REG1, statusCheck); //Write in the updated value to put the device in Active Mode
	
	if(error)
	{
		Serial.println("Motion mode setup error");
		Serial.println("retrying...");
		delay(100);
		motionMode(threshold,enableX,enableY,enableZ,enableINT2,arduinoINTPin);
	}
	
	motionMode_ = true;
}

/***********************************************************
 * 
 * regRead
 *
 * 
 *   
 ***********************************************************/
void MMA8451_n0m1::regRead(byte reg, byte *buf, byte count)
{
   I2c.read(I2CAddr, reg, count, buf);
}
 
/***********************************************************
 * 
 * regWrite
 *
 * 
 *   
 ***********************************************************/
void MMA8451_n0m1::regWrite(byte reg, byte val)
{
  I2c.write(I2CAddr, reg, val);
}

/***********************************************************
 * 
 * accelISR
 *
 * 
 *   
 ***********************************************************/
void accelISR(void){
	MMA8451_n0m1::pMMA8451_n0m1->ISRFlag = true;
	MMA8451_n0m1::pMMA8451_n0m1->measure_time_= millis();

}

/***********************************************************
 * 
 * setODR
 *
 * 
 *   
 ***********************************************************/
void MMA8451_n0m1::setODR(byte odr) {
	byte statusCheck;

	odr <<=3;

	//register settings must be made in standby mode
	boolean was_active = setStandby();


    I2c.read(I2CAddr,REG_CTRL_REG1,byte(1),&statusCheck);	
    statusCheck = statusCheck & ~odrMask;
    I2c.write(I2CAddr, REG_CTRL_REG1, byte(statusCheck | odr));

    //active Mode if was active
    if (was_active) setActive();
}

/***********************************************************
 * 
 * reset
 *
 * 
 *   
 ***********************************************************/
void MMA8451_n0m1::reset() {
    //Reset device
    I2c.write(I2CAddr, REG_CTRL_REG2, byte(0x40));
    delay(100);

}


/***********************************************************
 * 
 * setOversampling
 *
 * 
 *   
 ***********************************************************/
void MMA8451_n0m1::setOversampling(byte mods) {
	byte statusCheck;

	//register settings must be made in standby mode
	boolean was_active = setStandby();


    I2c.read(I2CAddr,REG_CTRL_REG2,byte(1),&statusCheck);
    statusCheck = statusCheck & ~modsMask;
    I2c.write(I2CAddr, REG_CTRL_REG2, byte(statusCheck | mods));

    //active Mode if was active
    if (was_active) setActive();
}

/***********************************************************
 * 
 * setHPF_Cutoff_freq
 *
 * 
 *   
 ***********************************************************/
void MMA8451_n0m1::setHPF_Cutoff_freq(byte freq) {
	byte statusCheck;
	
	freq=freq & HPFCutOffFreqMask;

	//register settings must be made in standby mode
	boolean was_active = setStandby();

    I2c.read(I2CAddr,REG_HP_FILTER_CUTOFF,byte(1),&statusCheck);
    statusCheck = statusCheck & ~HPFCutOffFreqMask;
    I2c.write(I2CAddr, REG_HP_FILTER_CUTOFF, byte(statusCheck | freq));

    //active Mode if was active
    if (was_active) setActive();
}

/***********************************************************
 * 
 * setHPF
 *
 * 
 *   
 ***********************************************************/
void MMA8451_n0m1::setHPF(boolean filter) {
	byte statusCheck;

	//register settings must be made in standby mode
	boolean was_active = setStandby();

    I2c.read(I2CAddr,REG_XYZ_DATA_CFG,byte(1),&statusCheck);
    
    if (filter) {
    I2c.write(I2CAddr, REG_HP_FILTER_CUTOFF, byte(statusCheck | HPFMask));
    }
    else {
    I2c.write(I2CAddr, REG_HP_FILTER_CUTOFF, byte(statusCheck & ~HPFMask));
    }
    
    //active Mode if was active
    if (was_active) setActive();
}


/***********************************************************
 * 
 * setActive
 *
 * 
 *   
 ***********************************************************/
void MMA8451_n0m1::setActive() {
	byte statusCheck;

	I2c.read(I2CAddr,REG_CTRL_REG1,byte(1),&statusCheck);
	I2c.write(I2CAddr, REG_CTRL_REG1, byte(statusCheck | activeMask));

}

/***********************************************************
 * 
 * setStandby
 *
 * 
 *   
 ***********************************************************/
boolean MMA8451_n0m1::setStandby() {
	byte statusCheck;
	
	I2c.read(I2CAddr,REG_CTRL_REG1,byte(1),&statusCheck);

if (statusCheck & activeMask) {

	I2c.write(I2CAddr, REG_CTRL_REG1, byte(statusCheck & ~activeMask));
return true;
} else return false;

}

/***********************************************************
 * 
 * setLNoise
 *
 * 
 *   
 ***********************************************************/
void MMA8451_n0m1::setLNoise() {
	byte statusCheck;

	I2c.read(I2CAddr,REG_CTRL_REG1,byte(1),&statusCheck);
	I2c.write(I2CAddr, REG_CTRL_REG1, byte(statusCheck | lowNoiseMask));
}

void MMA8451_n0m1::clearLNoise() {
	byte statusCheck;

	I2c.read(I2CAddr,REG_CTRL_REG1,byte(1),&statusCheck);
	I2c.write(I2CAddr, REG_CTRL_REG1, byte(statusCheck & ~lowNoiseMask));
}

void MMA8451_n0m1::setOFFX(byte off_x) {
	//Serial.print("setOFFX: ");
	//Serial.println(off_x,HEX);

	boolean was_active = setStandby();
	byte error_code=I2c.write(I2CAddr, REG_OFF_X, off_x);
	//Serial.print("I2c.read return code: ");
	//Serial.println(error_code,HEX);
	//active Mode if was active
	if (was_active) setActive();
}

void MMA8451_n0m1::setOFFY(byte off_y) {
	boolean was_active = setStandby();
	I2c.write(I2CAddr, REG_OFF_Y, off_y);
	if (was_active) setActive();
}

void MMA8451_n0m1::setOFFZ(byte off_z) {
	boolean was_active = setStandby();
	I2c.write(I2CAddr, REG_OFF_Z, off_z);
	if (was_active) setActive();
}

char MMA8451_n0m1::getOFFX() {
	byte statusCheck;
	//Serial.println("getOFFX");
	byte error_code=I2c.read(I2CAddr,REG_OFF_X,byte(1),&statusCheck);
	//Serial.print("I2c.read return code: ");
	//Serial.print(error_code,HEX);
	//Serial.println(statusCheck,HEX);
	return statusCheck;
}
char MMA8451_n0m1::getOFFY() {
	byte statusCheck;
	I2c.read(I2CAddr,REG_OFF_Y,byte(1),&statusCheck);
	return statusCheck;
}
char MMA8451_n0m1::getOFFZ() {
	byte statusCheck;
	I2c.read(I2CAddr,REG_OFF_Z,byte(1),&statusCheck);
	return statusCheck;
}
