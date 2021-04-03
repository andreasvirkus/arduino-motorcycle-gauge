#define ignitionSensor 1 
#define displayButton 4  // Press button pin
#define debounceDelay 30 // Debounce period to prevent flickering when pressing or releasing the button
// LCD pins:
#define LCD_SID  9
#define LCD_SCLK 12
#define LCD_A0   7 
#define LCD_RST  6
#define LCD_CS   5
// Backlight:
#define GREEN 11
#define BLUE 10
#define RED 8

#include <ST7565.h> // Adafruit LCD library
ST7565 glcd(LCD_SID, LCD_SCLK, LCD_A0, LCD_RST, LCD_CS);

volatile byte rpmRotations=0;
boolean allow = false;
boolean up = true;
boolean ignoreDn = false; 
long lastDebounceTime = 0;
long timer = 0;
long btnDnTime = 0;                  
long btnUpTime = 0;                  
int screenReading = LOW;             
int lastScreenButtonState = LOW;
//Tach:
const unsigned int pulsesPerRev = 1;
unsigned long lastPulseTime = 0;
unsigned long rpm = 0;
int add = 0;

void setup(){
  pinMode(displayButton, INPUT);
  // Backlight
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(RED, OUTPUT);
  analogWrite(GREEN, 100);
  analogWrite(BLUE, 0);
  analogWrite(RED, 200);

  Serial.begin(9600);
  glcd.begin(0x18);
  delay(50);
  glcd.clear();
  
  glcd.drawrect(20, 30, 90, 25, WHITE);
  
  //attachInterrupt(ignitionSensor, countIgnition, FALLING);
}

void checkButtonPress(){
  screenReading = digitalRead(displayButton);
  if(screenReading == HIGH && lastScreenButtonState == LOW && millis()-btnDnTime > debounceDelay){
    btnDnTime = millis();
  }
  else if(screenReading == LOW && lastScreenButtonState == HIGH && !ignoreDn){
    allow = !allow;
    btnDnTime=millis();
  }
  if(screenReading==LOW){
    ignoreDn = false;
  }
  lastScreenButtonState = screenReading;
} 

void makeBlocks(int width, int height){
  int lastHeight = 0;
  int i;
  for(i=0;i<height;i++){
    glcd.fillrect(20, 39-lastHeight, width, 50, WHITE);
    lastHeight += 9;
    glcd.fillrect(20, 39-lastHeight, width, 50, BLACK);
    lastHeight++;
  }
  glcd.fillrect(117,0,8,1,WHITE);
}

void displayRPM(){
//  noInterrupts();
  int mapValue = map(rpm,0,13000,0,90);
  char buf[8];
  itoa(rpm, buf, 7); 
  glcd.drawstring(50, 2, buf);
  
  //glcd.drawrect(20, 30, 90, 25, WHITE);
  glcd.fillrect(20, 30, mapValue, 25, WHITE); 
//  glcd.drawstring(0,2,buf);
//  char buf[12];
//  itoa(mapValue, buf, 10);
//  glcd.drawstring(118, 7, "F"); 
//  glcd.drawrect(117, 0, 8, 50, WHITE);
//  glcd.fillrect(119, 0, 4, 49, BLACK);
//  makeBlocks(4, mapValue);
//  interrupts();
}

void loop(){
  checkButtonPress();
  if(allow){
    for(i=0;i<  
    glcd.fillrect
   
   
    
  }

  doLoop();
  displayRPM();


  delay(3000);
}
