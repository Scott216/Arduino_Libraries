#include <SPI.h> // needed in Arduino 0019 or later
#include <Ethernet.h>
#include <Twitter.h>


// Ethernet Shield Settings
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// If you don't specify the IP address, DHCP is used(only in Arduino 1.0 or later).
byte ip[] = { 192, 168, 216, 91 };

// Your Token to Tweet (get it from http://arduino-tweet.appspot.com/)
Twitter twitter("466990809-9Tnfv81A1HOTPeIE62q1YqOwGZ0gXlZOcESslAfN");

// Message to post
char msg[] = "Hello, World! I'm Arduino!";

void setup()
{
  delay(2000);
  Ethernet.begin(mac, ip);
  // or you can use DHCP for autoomatic IP address configuration.
  // Ethernet.begin(mac);
  Serial.begin(57600);
  
  Serial.println("connecting to The Twitter");
  if (twitter.post(msg)) 
  {
    // Specify &Serial to output received response to Serial.
    // If no output is required, you can just omit the argument, e.g.
    // int status = twitter.wait();
    int status = twitter.wait(&Serial);
    if (status == 200) 
	{
      Serial.println("OK.");
    } 
	else 
	{
      Serial.print("failed : code ");
      Serial.println(status);
    }
  } else 
  {
    Serial.println("connection failed.");
  }
}

void loop()
{
}
