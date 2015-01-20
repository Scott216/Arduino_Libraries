

#include <MemoryFree.h>  //http://arduino.cc/playground/Code/AvailableMemory

// On Arduino Duemilanove with ATmega328:
//
// Reported free memory with str commented out:
// 1824 bytes
//
// Reported free memory with str and Serial.println(str) uncommented:
// 1810
//
// Difference: 14 bytes (13 ascii chars + null terminator)

// 14-bytes string
//char str[] = "Hello, world!";


void setup() {
    Serial.begin(57600);
}


void loop() {
    //Serial.println(str);

    Serial.print("freeMemory()=");
    Serial.println(freeMemory());

    delay(2000);
}
