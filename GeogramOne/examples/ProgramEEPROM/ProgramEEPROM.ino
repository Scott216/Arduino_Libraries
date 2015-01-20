// Set EEEPROM for Geogram ONE
// Modified by SRG for the setting I prefer

#include <AltSoftSerial.h>  // http://www.pjrc.com/teensy/td_libs_AltSoftSerial.html
#include <EEPROM.h>
#include "eepromAnything.h" // http://github.com/DSSCircuits/GeogramONE

/*******EEPROM ADDRESSES**********/

#define PINCODE					0
#define SMSADDRESS				5
#define EMAILADDRESS			44
#define APN						83
#define RETURNADDCONFIG			87

#define TIMEZONE				88   //use -4 for EST
#define TIMEFORMAT				89   // bit 0: 0 - am/pm,  1 - 24 hour format  bit 1: 0 - mm/dd/yy , 1 - yy/mm/dd
#define ENGMETRIC				90   // 0 - English (mph, ft, etc...), 1 = Metric (kph, m, etc...)
#define GEODATAFORMAT1			91
#define GEODATAFORMAT2			93
#define BATTERYLOWLEVEL    		95
#define SENDINTERVAL			96
#define IOSTATE0				100
#define IOSTATE1				101
#define IOSTATE2				102
#define IOSTATE3				103
#define IOSTATE4				104
#define IOSTATE5				105
#define IOSTATE6				106
#define IOSTATE7				107
#define IOSTATE8				108
#define IOSTATE9				109
#define SLEEPTIMECONFIG			110
#define SLEEPTIMEON				111
#define SLEEPTIMEOFF			115
#define SPEEDLIMIT				119
#define SPEEDHYST				121
#define ACTIVE1					122
#define INOUT1					123
#define RADIUS1					124
#define LATITUDE1				128
#define LONGITUDE1				132
#define ACTIVE2					136
#define INOUT2					137
#define RADIUS2					138
#define LATITUDE2				142
#define LONGITUDE2				146
#define ACTIVE3					150
#define INOUT3					151
#define RADIUS3					152
#define LATITUDE3				156
#define LONGITUDE3				160
#define BREACHSPEED				164
#define BREACHREPS				165
#define BMA0X0F					166
#define BMA0X10					167
#define BMA0X11					168
#define BMA0X16					169
#define BMA0X17					170
#define BMA0X19					171
#define BMA0X1A					172
#define BMA0X1B					173
#define BMA0X20					174
#define BMA0X21					175
#define BMA0X25					176
#define BMA0X26					177
#define BMA0X27					178
#define BMA0X28					179
#define MOTIONMSG				200
#define BATTERYMSG				225
#define FENCE1MSG				250
#define FENCE2MSG				275
#define FENCE3MSG				300
#define SPEEDMSG				325
#define MAXSPEEDMSG				350
#define GEOGRAMONEID			375
#define D4MSG					400
#define D10MSG					425
#define HTTP1					450
#define HTTP2					500
#define HTTP3					550

char pincode[5] = "1234"; //pincode must be 4 digits
char smsaddress[39] = "19738193079"; //smsaddress must be 38 characters or less
char emailaddress[39] = "scott@goldthwaite.com"; //email address must be 38 characters or less
char batteryMsg[25] = "Low Battery Alert";
char motionMsg[25] = "Motion Detected";
char fence1Msg[25] = "Fence 1 Breach";
char fence2Msg[25] = "Fence 2 Breach";
char fence3Msg[25] = "Fence 3 Breach";
char speedMsg[25] = "Speed Limit Exceeded";
char geoIDMsg[25] = "GO1";
char maxSpeedMsg[25] = "Max Speed = ";
char http1[100] = "http://maps.google.com/?q=";
char http2[100] = "+(";
char http3[100] = ")&z=19";
char d4msg[25] = "D4 Switch Alert";
char d10msg[25] = "D10 Switch Alert";

void setup()
{
	EEPROM_writeAnything(PINCODE,pincode);
	EEPROM_writeAnything(SMSADDRESS,smsaddress);
	EEPROM_writeAnything(EMAILADDRESS,emailaddress);
	EEPROM_writeAnything(APN,(unsigned long)500);
	EEPROM_writeAnything(RETURNADDCONFIG,(uint8_t)0x01);
	EEPROM_writeAnything(BATTERYLOWLEVEL,(uint8_t)32);
	EEPROM_writeAnything(IOSTATE0,(uint8_t)0);
	EEPROM_writeAnything(IOSTATE1,(uint8_t)0x06); //int falling
	EEPROM_writeAnything(IOSTATE2,(uint8_t)4);
	EEPROM_writeAnything(IOSTATE3,(uint8_t)4);
	EEPROM_writeAnything(IOSTATE4,(uint8_t)4);
	EEPROM_writeAnything(IOSTATE5,(uint8_t)4);
	EEPROM_writeAnything(IOSTATE6,(uint8_t)0);
	EEPROM_writeAnything(IOSTATE7,(uint8_t)0);
	EEPROM_writeAnything(IOSTATE8,(uint8_t)0);
	EEPROM_writeAnything(IOSTATE9,(uint8_t)0);
	EEPROM_writeAnything(ACTIVE1,(uint8_t)0);
	EEPROM_writeAnything(ACTIVE2,(uint8_t)0);
	EEPROM_writeAnything(ACTIVE3,(uint8_t)0);
	EEPROM_writeAnything(INOUT1,(uint8_t)0);
	EEPROM_writeAnything(INOUT2,(uint8_t)0);
	EEPROM_writeAnything(INOUT3,(uint8_t)0);
	EEPROM_writeAnything(RADIUS1,(unsigned long)0);
	EEPROM_writeAnything(RADIUS2,(unsigned long)0);
	EEPROM_writeAnything(RADIUS3,(unsigned long)0);
	EEPROM_writeAnything(LATITUDE1,(long)0);
	EEPROM_writeAnything(LATITUDE2,(long)0);
	EEPROM_writeAnything(LATITUDE3,(long)0);
	EEPROM_writeAnything(LONGITUDE1,(long)0);
	EEPROM_writeAnything(LONGITUDE2,(long)0);
	EEPROM_writeAnything(LONGITUDE3,(long)0);
	EEPROM_writeAnything(SPEEDLIMIT,(uint16_t)0);
	EEPROM_writeAnything(SPEEDHYST,(uint8_t)3);
	EEPROM_writeAnything(TIMEZONE,(int8_t)-4);   //use -4 for EST
	EEPROM_writeAnything(TIMEFORMAT,(uint8_t)0);   // bit 0: 0 - am/pm,  1 - 24 hour format bit 1: 0 - mm/dd/yy, 1 - yy/mm/dd
	EEPROM_writeAnything(ENGMETRIC,(uint8_t)0);  // 0 - English (mph, ft, etc...), 1 = Metric (kph, m, etc...)
	EEPROM_writeAnything(BMA0X0F,(uint8_t)0x05); //was 3
	EEPROM_writeAnything(BMA0X10,(uint8_t)0x08);
	EEPROM_writeAnything(BMA0X11,(uint8_t)0x00); //default 0x00 per datasheet
	EEPROM_writeAnything(BMA0X16,(uint8_t)0x07);
	EEPROM_writeAnything(BMA0X17,(uint8_t)0x00);
	EEPROM_writeAnything(BMA0X19,(uint8_t)0x04);
	EEPROM_writeAnything(BMA0X1A,(uint8_t)0x00);
	EEPROM_writeAnything(BMA0X1B,(uint8_t)0x00);
	EEPROM_writeAnything(BMA0X20,(uint8_t)0x06);
	EEPROM_writeAnything(BMA0X21,(uint8_t)0x8E);
	EEPROM_writeAnything(BMA0X25,(uint8_t)0x0F); //default 0x0F per datasheet
	EEPROM_writeAnything(BMA0X26,(uint8_t)0xC0); //default 0xC0 per datasheet
	EEPROM_writeAnything(BMA0X27,(uint8_t)0x00);
	EEPROM_writeAnything(BMA0X28,(uint8_t)0x05);
	EEPROM_writeAnything(SENDINTERVAL,(unsigned long)0x00);
	EEPROM_writeAnything(SLEEPTIMEON,(unsigned long)0x00);
	EEPROM_writeAnything(SLEEPTIMEOFF,(unsigned long)0x00);
	EEPROM_writeAnything(SLEEPTIMECONFIG,(uint8_t)0x03);
	EEPROM_writeAnything(BREACHSPEED,(uint8_t)0x02);
	EEPROM_writeAnything(BREACHREPS,(uint8_t)0x0A);
	EEPROM_writeAnything(GEODATAFORMAT1,(uint16_t)6147); //date, time, battery%, battery volts
	EEPROM_writeAnything(GEODATAFORMAT2,(uint16_t)512); //# satellites
	EEPROM_writeAnything(MOTIONMSG,motionMsg);
	EEPROM_writeAnything(BATTERYMSG,batteryMsg);
	EEPROM_writeAnything(FENCE1MSG,fence1Msg);
	EEPROM_writeAnything(FENCE2MSG,fence2Msg);
	EEPROM_writeAnything(FENCE3MSG,fence3Msg);
	EEPROM_writeAnything(SPEEDMSG,speedMsg);
	EEPROM_writeAnything(MAXSPEEDMSG,maxSpeedMsg);
	EEPROM_writeAnything(GEOGRAMONEID,geoIDMsg);
	EEPROM_writeAnything(HTTP1,http1);
	EEPROM_writeAnything(HTTP2,http2);
	EEPROM_writeAnything(HTTP3,http3);
	EEPROM_writeAnything(D4MSG,d4msg);
	EEPROM_writeAnything(D10MSG,d10msg);
}

void loop()
{
}
