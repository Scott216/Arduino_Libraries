/* *********************************************************
   Clock Test.
   Prints the time every second to the serial port. Time is
   maintained by the clock class. 
   
   Monitor time using MegunoLink, a free tool for communicating
   with the Arduino. MegunoLink can graph data, log serial data
   upload HEX files and simulate serial devices. Download
   MegunoLink from http://www.MegunoLink.com/
*/

#include <SPI.h>  
#include <Ethernet.h>
#include <Clock.h>

Clock g_Clock;
unsigned long g_ulLastPrint;

void setup()
{
  byte ArduinoMAC[] = {0xAE, 0xAD, 0xBF, 0xEA, 0xDE, 0xED}; 
  IPAddress ArduinoIP(192, 168, 15, 55);

  Serial.begin(9600);
  
  // Initialize Ethernet.
  Ethernet.begin(ArduinoMAC, ArduinoIP);

  // Initialize time for last print. 
  g_ulLastPrint = millis();
  
  // Setup the clock. In New Zealand we are 12 hours ahead of GMT 
  g_Clock.Setup();
  g_Clock.SetTimezoneOffset(12,0); 
}

void loop()
{
  g_Clock.Maintain();
  
  if (millis() - g_ulLastPrint > 1000)
  {
    g_ulLastPrint = millis();
    g_Clock.WriteDateTime(&Serial);
    Serial.write('\n');
  }
}
