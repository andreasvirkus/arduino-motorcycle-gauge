// Motorcycle LCD Dash.
// http://hackaday.com/2011/08/26/a-beginners-guide-to-burning-arduino-bootloaders/
// http://kokoraskostas.blogspot.com/2013/12/arduino-inductive-spark-plug-sensor.html

//Bike measurments:
//tyre: 90/90 - 18
//tyre radius = 12.185" = 309.499 millimeters
//tyre diameter = 24.378" = 619.2012 millimeters
//wheel circumference = pi*2*r =~76.586 inches = 1.9452844 meters
//v = 2πr × RPM × (60/1000) km/hr
//v = r × RPM × 0.10472
//distance = motorcycle moves 1 circumference per revolution

//60 revolutions per second = 1 Hz

// WHEELCIRC = 2 * PI * radius (in meters)
// speed = rpmilli * WHEELCIRC * "milliseconds per hour" / "meters per kilometer"
// simplify the equation to reduce the number of floating point operations
// speed = rpmilli * WHEELCIRC * 3600000 / 1000
// speed = rpmilli * WHEELCIRC * 3600

// NIGHT:  Brightness(15) | Contrast(0x18)
// DAY:    Brightness()  |  Contrast()
#define hallSensor 0     // Hall sensor pin
#define ignitionSensor 1 
#define photoCellPin 0   // Photo cell pin (Equivalent to a0)
#define fuel A3          // Fuel sensor pin
#define displayButton 4  // Press button pin
#define screens 2        // Number of dash screens
#define holdTime 2000    // Time requirement to reset trip
#define debounceDelay 30 // Debounce period to prevent flickering when pressing or releasing the button
#define EEPROMtrip       // The trip address on EEPROM
#define EEPROModo        // The odo address on EEPROM
#define fuelConsumption 6.6  // L / 100km
#define fuelGaugePosX 117
#define fuelGaugePosY 0
#define lightLevel 400
#define BAUDRATE 9600

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
//#include <graphics/graphics.h>  // draw_text(char *string, unsigned char x, unsigned char y, unsigned char *font, unsigned char spacing)
#include <EEPROM.h>      
#include <Wire.h>
#include <RTClib.h>
#include <allFonts.h>


RTC_DS1307 rtc;

ST7565 glcd(LCD_SID, LCD_SCLK, LCD_A0, LCD_RST, LCD_CS);

volatile byte wheelRotations=0;
volatile byte rpmRotations=0;
boolean reedSwitchState; 
boolean lastReedSwitchState;
boolean ignoreDn = false;            // Whether to ignore the button release because the click+hold was triggered
float rotationTime;  
float wheelCircumference = 1.9452844;   // Circumference of the tyre (inches) = 1.9452844 meters
float RPM;
float kph = 0.00;
float distance = 0;
float trip = 0;
unsigned long startRotation = 0;  
unsigned long endRotation = 0;
unsigned long startPress = 0;
unsigned long rotations = 0;
unsigned long lastmillis = 0;
long lastDebounceTime = 0;
long timer = 0;
long btnDnTime = 0;                  // Time the button was pressed down
long btnUpTime = 0;                  // Time the button was released
int screenReading = LOW;             // Value read from the button
int lastScreenButtonState = LOW;     // Buffered value of the button's previous state
int screenButtonState;
int reedButtonState;
int val;
int val2;
int screen=1;
int nullCounter = 0;
int count;
int doWrite;
int backlightState = 0;
int estDistLeft = 0;
//Tach:
const unsigned int pulsesPerRev = 1;
unsigned long lastPulseTime = 0;
unsigned long rpm = 0;
uint8_t brightness_levels[7] = { 0, 20, 50, 90, 130, 190, 250 };
uint8_t brightness_index; // Index of brightness_levels

void setup(){
  pinMode(displayButton, INPUT);
  pinMode(fuel, INPUT);
  // Backlight
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(RED, OUTPUT);
  digitalWrite(GREEN, LOW);
  digitalWrite(BLUE, LOW);
  digitalWrite(RED, LOW);
  Serial.begin(BAUDRATE);
  glcd.begin(0x18);
  delay(50);
  glcd.clear();
  
  attachInterrupt(ignitionSensor, ignitionIsr, FALLING);
  //attachInterrupt(hallSensor, countWheelRotations, FALLING);
  
  Wire.begin();
  rtc.begin();

  if (!rtc.isrunning()){
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    //rtc.adjust(DateTime(__DATE__, __TIME__));
    }
  rtc.adjust(DateTime(__DATE__, __TIME__));
  displayEst();
}

void countWheelRotations(){
  wheelRotations++;
 } 

void DoTimeAndDistanceCalcs(){
  rotationTime = millis() - lastmillis;
  if (rotationTime >= 250){ //Uptade every one second, this will be equal to reading frecuency (Hz).
     detachInterrupt(0); //Disable interrupt when calculating
     /*
     Serial.print(wheelRotations); //print the word "Hz".
     Serial.print("\t");
     Serial.print(rotationTime);
     Serial.print("\t");
     */
     distance = wheelRotations * wheelCircumference * (rotationTime/1000);     // Distance = revolutions * circumference and divided by 1000 for m -> km conversion
     trip += distance / 1000;
     kph = distance * 3.6;
     /*
     Serial.print(distance);
     Serial.print("\tK/ph=\t");
     Serial.println(kph);
     */
     wheelRotations = 0; // Restart the RPM counter
     lastmillis = millis(); // Uptade lasmillis
     attachInterrupt(0, countWheelRotations, FALLING); //enable interrupt
  }  
}

void ignitionIsr(){
  unsigned long now = micros();
  unsigned long interval = now - lastPulseTime;
  if (interval > 5000){
     rpm = 60000000UL/(interval * pulsesPerRev);
     lastPulseTime = now;
}  
    rpmRotations++;
}

void selectScreen(){
  screenReading = digitalRead(displayButton);
  
  // Test for button press and store the press down time
  if(screenReading == HIGH && lastScreenButtonState == LOW && millis()-btnDnTime > debounceDelay){
    btnDnTime = millis();
  }
  
  // Test for button hold
  if(screenReading == HIGH && lastScreenButtonState == HIGH && screen == 2 && millis()-btnDnTime > holdTime){
    trip = 0;
    displayRed();
    ignoreDn = true;
  }
  // Test for button release and increment screen
  else if(screenReading == LOW && lastScreenButtonState == HIGH && !ignoreDn){
    screen = (screen% screens)+1; // Cycles through 1 -> screens
    btnDnTime=millis();
  }
  if(screenReading==LOW){
    ignoreDn = false;
  }
  lastScreenButtonState = screenReading;
} 

void checkBrightness(){
  //photoCellReading = 1023 - analogRead(photoCellPin);
  if(analogRead(photoCellPin) > lightLevel)
    toggle_backlight(0);
  else
    toggle_backlight(1);  
}

void toggle_backlight(boolean mode) {
  backlightState = backlightState^1;
  if(mode){
    //glcd.st7565_set_brightness(15);
    analogWrite(GREEN, 100);
    analogWrite(BLUE, 100);
    analogWrite(RED, 100);
  }
  else{
    //glcd.st7565_set_brightness(24);
    digitalWrite(GREEN, LOW);
    digitalWrite(BLUE, LOW);
    digitalWrite(RED, LOW);
  }
}

void fillFuelGauge(int width, int height){
  int lastHeight = 0;
  int i;
  for(i=0;i<height;i++){
    glcd.fillrect(119, 39-lastHeight, width, 9, WHITE);
    lastHeight += 9;
    glcd.fillrect(119, 39-lastHeight, width, 1, BLACK);
    lastHeight++;
  }
  glcd.fillrect(117,0,8,1,WHITE);
}

//=========== GUI ================

void displayKPH(){
  char buf[12];
  itoa(kph, buf, 10);
  //glcd.setFont(font_courB18);
  glcd.drawstring(55, 4, buf);
  glcd.drawstring(72, 4, " km/h");
  //draw_text(buf, 55, 4, Verdana24, 1);
  //draw_text(" km/h", 72, 4, Tahoma10, 1);
}

void displayFuel(){
  int readValue = analogRead(fuel);
  int mapValue = map(readValue,500,1023,0,5);
  char buf[12];
  itoa(mapValue, buf, 10);
  glcd.drawstring(118, 7, "F"); 
  glcd.drawrect(117, fuelGaugePosY, 8, 50, WHITE);
  glcd.fillrect(119, fuelGaugePosY, 4, 49, BLACK);
  fillFuelGauge(4, mapValue);
  //glcd.fillrect(fuelGaugePosX, fuelGaugePosY, 8, 50 - 10*mapValue, BLACK);
}

void displayRPM(){
  int i;
  noInterrupts();
  Serial.println(rpm);
  interrupts();
  delay(150);
  int mapValue = map(rpm,0,13000,0,90);
  char buf[12];
  itoa(rpm, buf, 10); 
  glcd.drawstring(0, 0, "RPM");
  glcd.drawrect(20, 0, 90, 10, WHITE);
  for(i=0;i<27;i++){
    if(i%2==0)
      glcd.drawline(20+i*3, 8, 20+i*3, 3, WHITE);
    else
      glcd.drawline(20+i*3, 8, 20+i*3, 6, WHITE);
  }
  for(i=0;i<5;i++){
    glcd.drawline(101 + i*2, 8, 101 + i*2, 0, WHITE);
  }
  glcd.fillrect(20, 0, mapValue, 10, WHITE); 
  glcd.drawstring(0,2,buf);
}

void displayTrip(){
  char buf[12];
  itoa(trip, buf, 10); 
  glcd.drawstring(0, 6, "9 999");
  glcd.drawstring(32, 6, "km");
}

void displayOdo(){
  glcd.drawstring(55, 6, "999 999");
  glcd.drawstring(98, 6, "km");
}

void displayEst(){ 
  int readValue = analogRead(fuel);
  int mapValue = map(readValue,500,1023,0,180); // How much is left of the max 18.0 L
  estDistLeft = mapValue/fuelConsumption*10;
  char buf[12];
  itoa(estDistLeft, buf, 10);
  glcd.drawstring(45, 7, "Est.");
  glcd.drawstring(74, 7, buf);
  glcd.drawstring(95, 7, "km");
}

void displayGear(){ 
  char gear[5];
  //itoa(whichGear(), gear, 4);
  glcd.drawstring(15, 4, "N"); 
}

void displayTime(){
    DateTime now = rtc.now();
    char thisHour[12];
    itoa(now.hour(), thisHour, 10);
    char thisMinute[12];
    itoa(now.minute(), thisMinute, 10);
    glcd.drawstring(0, 7, thisHour);
    glcd.drawstring(12, 7, ":");
    glcd.drawstring(18, 7, thisMinute);
    /*
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    */
}

void switchScreen(){
    switch(screen){
    case 1:
      screen1();
      break;
    case 2:
      screen2();
      break;
    }
}

void screen1(){
  glcd.clear();
  displayKPH();
  displayRPM();
  displayTrip();
  displayOdo();
  displayFuel();
  displayGear(); 
  displayTime();
}

void screen2(){
  glcd.clear();
  displayKPH();
  displayRPM();
  displayTrip();
  displayFuel();
  displayTime();
  displayOdo();
}

void loop(){
  //DoTimeAndDistanceCalcs();
  checkBrightness();
  selectScreen();
  switchScreen();
  glcd.display();
  Serial.println(rpm);
//  Serial.println(rpmRotations);
  rpmRotations = 0;
  delay(100);
}
