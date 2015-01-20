/* *********************************************************************
   Simple class that provides the current time & date. At intervals,
   the network time protocol (NTP) is used to obtain the time from
   a time server. Between synchronization, the arduino's millisecond
   counter is used to extrapolate the current time. 
   
   NTP query based on code from Arduino Ethernet library.
   Conversion between Unix time & natural time represenation based on 
   code by Michael Margolis. 
   
   This library is free software; you can redistribute it and/or 
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   Visit http://www.MegunoLink.com/Libraries for the latest version.
   ********************************************************************* */

#include <SPI.h>  
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <avr/pgmspace.h>

#define DEBUG

class DateTime
{
  public:
    byte 
      Second, 
      Minute,
      Hour,
      Day,
      Month; // 1 => Jan, 2 => Feb, etc.
    word Year;
};

class Clock
{
  // Port used for UDP packets.
  const int mc_nLocalUDPPort;
  
  // Time between refreshing time reference [milliseconds] &
  // between retry attempts if we don't get the NTP time [milliseconds]
  const unsigned long mc_ulRefreshInterval;
  const unsigned long mc_ulRetryInterval;
  
  // Arduino time when NTP time was last acquired.
  unsigned long m_ulLastAcquired;

  // Time last acquired from NTP server. 0 if none.
  unsigned long m_ulLastNTPTime; 
  
  // Time when we last tried to get NTP time. 0 if we were successful.
  unsigned long m_ulLastAttempt;
  
  // IP Address for time server
  const IPAddress mc_TimeServer;
  
  // Time is stored in UTC format internal. This offset is
  // applied in GetTimestamp();
  int m_nTimezoneOffset; // [min]. 
  
  public: 
    Clock();
    
    void Setup();
    void Maintain();
    
    unsigned long GetTimestamp();
    void WriteTime(HardwareSerial *pPort);
    void WriteDateTime(HardwareSerial *pPort);
    
    void DecodeTo(DateTime &dt);
    
    void SetTimezoneOffset(int nHours, int nMinutes);
  
  private:
    void RefreshNTPTime();
    void SendNTPPacket(EthernetUDP &rUDP, const IPAddress &rAddress);
    inline bool IsLeapYear(unsigned int uYear)
    {
      return !(uYear%4) && ( (uYear%100) || !(uYear%400) );
    }
};

