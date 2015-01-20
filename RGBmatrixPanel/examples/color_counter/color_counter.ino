#include <Adafruit_GFX.h>   // Core graphics library
#include <RGBmatrixPanel.h> // Hardware-specific library

#define CLK 12
#define OE  38
#define A   30 
#define B   32 
#define C   34 
#define LAT 36 
RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false, 2);

void setup() 
{
  matrix.begin();
  matrix.setTextSize(1);    // size 1 == 8 pixels high
  matrix.fillScreen(0); // clear display 
}

void loop() 
{

  uint8_t r=0, g=0, b=0;
  char dispBuf[20]; 
  
  for (r=0; r < 8; r++)
  {
    for (g=0; g < 8; g++)
    {
      for (b=0; b < 8; b++)
      {
        matrix.fillScreen(0); // clear display 
        matrix.setTextColor(matrix.Color333(r,g,b));
        matrix.setCursor(6, 4); 
        sprintf(dispBuf, "%d %d %d", r,g,b);  
        matrix.print(dispBuf);
        delay(700);
      } // blue
    } // green 
  } // red
 
}

