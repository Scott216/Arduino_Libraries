/*
  Twitter.cpp - Arduino library to Post messages to Twitter using OAuth.
  Copyright (c) NeoCat 2010-2011. All right reserved.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 */

// ver1.2 - Use <string.h>
// ver1.3 - Support IDE 1.0

// SRG 7/3/12 - changed client. to tweetclient.

#include <string.h>
#include "Twitter.h"

#define LIB_DOMAIN "arduino-tweet.appspot.com"

#if defined(ARDUINO) && ARDUINO < 100
static uint8_t server[] = {0,0,0,0}; // IP address of LIB_DOMAIN
Twitter::Twitter(const char *token) : client(server, 80), token(token)
{
}
#else
Twitter::Twitter(const char *token) : token(token)
{
}
#endif

bool Twitter::post(const char *msg)
{
#if defined(ARDUINO) && ARDUINO < 100
	DNSError err = EthernetDNS.resolveHostName(LIB_DOMAIN, server);
	if (err != DNSSuccess) {
		return false;
	}
#endif
	parseStatus = 0;
	statusCode = 0;
#if defined(ARDUINO) && ARDUINO < 100
	if (tweetclient.connect()) {
#else
	if (tweetclient.connect(LIB_DOMAIN, 80)) {
#endif
		tweetclient.println("POST http://" LIB_DOMAIN "/update HTTP/1.0");
		tweetclient.print("Content-Length: ");
		tweetclient.println(strlen(msg)+strlen(token)+14);
		tweetclient.println();
		tweetclient.print("token=");
		tweetclient.print(token);
		tweetclient.print("&status=");
		tweetclient.println(msg);
	} else {
		return false;
	}
	return true;
}

bool Twitter::checkStatus(Print *debug)
{
	if (!tweetclient.connected()) {
		if (debug)
			while(tweetclient.available())
				debug->print((char)tweetclient.read());
		tweetclient.flush();
		tweetclient.stop();
		return false;
	}
	if (!tweetclient.available())
		return true;
	char c = tweetclient.read();
	if (debug)
		debug->print(c);
	switch(parseStatus) {
	case 0:
		if (c == ' ') parseStatus++; break;  // skip "HTTP/1.1 "
	case 1:
		if (c >= '0' && c <= '9') {
			statusCode *= 10;
			statusCode += c - '0';
		} else {
			parseStatus++;
		}
	}
	return true;
}

int Twitter::wait(Print *debug)
{
	while (checkStatus(debug));
	return statusCode;
}
