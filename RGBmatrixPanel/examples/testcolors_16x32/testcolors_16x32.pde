// testcolors demo for RGBmatrixPanel library.
// Renders 512 colors on a 16x32 RGB LED matrix.
// Library supports 4096 colors, but there aren't that many pixels!

#include <Adafruit_GFX.h>   // Core graphics library
#include <RGBmatrixPanel.h> // Hardware-specific library

#define CLK 8  // MUST be on PORTB!
#define LAT A3
#define OE  9
#define A   A0
#define B   A1
#define C   A2
RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false);

void setup() {
  matrix.begin();
  uint8_t r=0, g=0, b=0;

  // Draw top half
  for (uint8_t x=0; x < 32; x++) {      
    for (uint8_t y=0; y < 8; y++) {  
      matrix.drawPixel(x, y, matrix.Color333(r, g, b));
      r++;
      if (r == 8) {
        r = 0;
        g++;
        if (g == 8) {
          g = 0;
          b++;
        }
      }
    }
  }

  // Draw bottom half
  for (uint8_t x=0; x < 32; x++) {      
    for (uint8_t y=8; y < 16; y++) {  
      matrix.drawPixel(x, y, matrix.Color333(r, g, b));
      r++;
      if (r == 8) {
        r = 0;
        g++;
        if (g == 8) {
          g = 0;
          b++;
        }
      }
    }
  }
}

void loop() {
  // do nothing
}

