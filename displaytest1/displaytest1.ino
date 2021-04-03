#include <ST7565.h>

// LCD pins:
// the LCD backlight is connected up to a pin so you can turn it on & off
#define RED_LED
#define GREEN_LED
#define BLUE_LED
#define BACKLIGHT_LED 10
#define LCD_SID  9
#define LCD_SCLK 8
#define LCD_A0   7 
#define LCD_RST  6
#define LCD_CS   5

// pin 9 - Serial data out (SID)
// pin 8 - Serial clock out (SCLK)
// pin 7 - Data/Command select (RS or A0)
// pin 6 - LCD reset (RST)
// pin 5 - LCD chiAp select (CS)

#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 

ST7565 glcd(LCD_SID, LCD_SCLK, LCD_A0, LCD_RST, LCD_CS);

// The setup() method runs once, when the sketch starts
void setup()   {                
  Serial.begin(9600);
  
  // turn on backlight
  pinMode(BACKLIGHT_LED, OUTPUT);
  digitalWrite(BACKLIGHT_LED, HIGH);

  // initialize and set the contrast to 0x18
  glcd.begin(0x18);

  glcd.display(); // show splashscreen
  delay(2000);
  glcd.clear();
}


void loop()                     
{}
