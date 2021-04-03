/*
 simple_menu.pde - 11/2011
 Basic example of ST7565_Menu Arduino library.
 
 Remember the ST7565 uses 3.3v, so a level shifter
 is needed if using a standard Arduino board.
 See Adafruit tutorial for more details:
  http://www.ladyada.net/learn/lcd/st7565.html
*/

#include <ST7565.h> // Adafruit LCD library

// Menu controls:
#define UP_PIN     3  
#define DOWN_PIN   4  
#define SELECT_PIN 2  

// LCD pins:
#define LCD_SID  9
#define LCD_SCLK 13
#define LCD_A0   7 
#define LCD_RST  6
#define LCD_CS   5
// Backlight:
#define GREEN 11
#define BLUE 10
#define RED 8
#define delayTime 20


// Must create an ST7565 instance before Menu:
ST7565 glcd(LCD_SID, LCD_SCLK, LCD_A0, LCD_RST, LCD_CS);

int backlight_state = 0;

int redVal;
int blueVal;
int greenVal;

//Backlight PWM values:
uint8_t brightness_levels[7] = { 0, 20, 50, 90, 130, 190, 250 };
uint8_t brightness_index; // Index of brightness_levels

void setup() {
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(RED, OUTPUT);
  digitalWrite(GREEN, HIGH);
  digitalWrite(BLUE, HIGH);
  digitalWrite(RED, HIGH);
  Serial.begin(9600);
  
  glcd.begin(0x18); // Initialise LCD 
  delay(10); // Just to be safe
  glcd.clear();
  glcd.drawstring(0,1,"Hello world");
  glcd.display();
  toggle_backlight();
}

void toggle_backlight() {
  backlight_state = backlight_state^1;
  //digitalWrite(BACKLIGHT_LED, backlight_state);
}

void loop() {
  int redVal = 255;
  int blueVal = 0;
  int greenVal = 0;
  for( int i = 0 ; i < 255 ; i += 1 ){
    greenVal += 1;
    redVal -= 1;
    analogWrite( GREEN, greenVal );
    analogWrite( RED, redVal );
    delay( delayTime );
  }
  redVal = 0;
  blueVal = 0;
  greenVal = 255;
  for( int i = 0 ; i < 255 ; i += 1 ){
    blueVal += 1;
    greenVal -= 1;
    analogWrite( BLUE, blueVal );
    analogWrite( GREEN, greenVal );

    delay( delayTime );
  }
  redVal = 0;
  blueVal = 255;
  greenVal = 0;
  for( int i = 0 ; i < 255 ; i += 1 ){
    redVal += 1;
    blueVal -= 1;
    analogWrite( RED, redVal );
    analogWrite( BLUE, blueVal );

    delay( delayTime );
  }
}
