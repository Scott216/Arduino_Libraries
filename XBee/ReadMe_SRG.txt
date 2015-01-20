Last update from Repository 2/4/14
The 2/4/14 update requires xbee to be initialized as follows:
Reference: https://code.google.com/p/xbee-arduino/wiki/SoftwareSerialReleaseNotes
Serial.begin(9600);
xbee.setSerial(Serial);
xbee.begin(Serial);

Source: https://code.google.com/p/xbee-arduino/source/browse/

Following was fixed in repo, you can disregard
For library to work with Leonardo, you need to make a change in XBee.cpp
XBee::XBee(): _response(XBeeResponse()) 
Reference: http://forum.arduino.cc/index.php?topic=111354.msg839300#msg839300
