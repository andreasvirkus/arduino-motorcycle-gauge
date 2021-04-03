// Motorcycle LCD Dash.
// http://hackaday.com/2011/08/26/a-beginners-guide-to-burning-arduino-bootloaders/

// Bike measurments:
// tyre: 90/90 - 18
// tyre radius = 12.185" = 309.499 millimeters
// tyre diameter = 24.378" = 619.2012 millimeters
// wheel circumference = pi*2*r =~76.586 inches = 1.9452844 meters
// v = 2 × π × r × RPM × (60/1000) km/h
// v = r × RPM × 0.10472
// distance = motorcycle moves 1 circumference per revolution

// 60 revolutions per second = 1 Hz

// WHEELCIRC = 2 * PI * radius (in meters)
// speed = rpmilli * WHEELCIRC * "milliseconds per hour" / "meters per kilometer"
// simplify the equation to reduce the number of floating point operations
// speed = rpmilli * WHEELCIRC * 3600000 / 1000
// speed = rpmilli * WHEELCIRC * 3600

#define hallSensor 0     // Hall sensor pin
#define ignitionSensor 0 
#define fuel A3          // Fuel sensor pin
#define displayButton 4  // Press button pin
#define screens 2        // Number of dash screens
#define holdTime 2000    // Time requirement to reset trip
#define debounceDelay 30 // Debounce period to prevent flickering when pressing or releasing the button
#define EEPROMtrip       // The trip address on EEPROM
#define EEPROModo        // The odo address on EEPROM
#define fuelConsumption 6.6  // L / 100km

// LCD pins:
#define LCD_SID  9
#define LCD_SCLK 12
#define LCD_A0   7 
#define LCD_RST  6
#define LCD_CS   5
// LCD backlight:
#define GreenPin 11
#define BluePin 10
#define RedPin 8

/*
#include <EEPROM.h>      
#include <Wire.h>
#include <RTClib.h>
#include <string.h>
*/
#include <U8glib.h>

U8GLIB_LM6059 u8g(LCD_SCLK, LCD_SID, LCD_CS, LCD_A0, LCD_RST);   // Pin assignments for the ST7565: u8g(SCK, MOSI, CS, A0, RST)
//U8GLIB_LM6059 u8g(LCD_SCLK, LCD_SID, LCD_CS, LCD_A0);
//RTC_DS1307 rtc;

volatile byte wheelRotations=0;
boolean reedSwitchState; 
boolean lastReedSwitchState;
boolean ignoreDn = false;            // Whether to ignore the button release because the click+hold was triggered
char kphbuf[12];
char fuelbuf[12];
char tripbuf[12];
char estbuf[12];
float rotationTime;  
float wheelCircumference = 1.9452844;   // Circumference of the tyre (inches) = 1.9452844 meters
unsigned long startRotation = 0;  
unsigned long endRotation = 0;
unsigned long startPress = 0;
unsigned long rotations = 0;
unsigned long lastmillis = 0;
long RPM;
long kph = 0.00;
long distance = 0;
long trip = 0;
long lastDebounceTime = 0;
long timer = 0;
long btnDnTime = 0;                  // Time the button was pressed down
long btnUpTime = 0;                  // Time the button was released
long estDistLeft;
int screenReading = LOW;             // Value read from the button
int lastScreenButtonState = LOW;     // Buffered value of the button's previous state
int screenButtonState;
int reedButtonState;
int fuelMapValue;
int rpmMapValue;
int estMapValue;
int val;
int val2;
int nullCounter = 0;
int count;
int doWrite;
int backlightState = 0;
//Tach:
const unsigned int pulsesPerRev = 1;
unsigned long lastPulseTime = 0;
unsigned long rpm = 0;
uint8_t brightness_levels[7] = { 0, 20, 50, 90, 130, 190, 250 };
uint8_t brightness_index; // Index of brightness_levels

void setup(void) {
  Serial.begin(9600); // Setup serial library at 9600 bps
  // Set RGB PWM pins to outputs
  pinMode(BluePin, OUTPUT);
  pinMode(GreenPin, OUTPUT);
  pinMode(RedPin, OUTPUT);

  pinMode(displayButton, INPUT);
  pinMode(fuel, INPUT);
  //Serialbegin(57600);  // Bluetooth requires a faster serial write
   
  setBacklight(0, 0, 0);
  u8g.setContrast(100);
  
  //attachInterrupt(ignitionSensor, ignitionIsr, RISING);
  attachInterrupt(hallSensor, countWheelRotations, FALLING);
  
  /*
  #ifdef AVR
    Wire.begin();
  #else
    Wire1.begin(); // Shield I2C pins connect to alt I2C bus on Arduino Due
  #endif
  rtc.begin();

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(__DATE__, __TIME__));
  } */ 
}

void setBacklight(int red, int green, int blue){
// Backlight PWM values.  255 = dimmest, 0 = brightest.
  analogWrite(RedPin, red);
  analogWrite(GreenPin, green);
  analogWrite(BluePin, blue);
}

void clear_screen(void) {
  u8g.setColorIndex(0);
  u8g.drawBox(0, 0, u8g.getWidth(), u8g.getHeight());
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
}

void toggle_backlight() {
  backlightState = backlightState^1;
  if(backlightState == LOW) u8g.setContrast(255);
  else if(backlightState == HIGH) u8g.setContrast(100);
}


//=========== GUI ================

void displayKPH(){
  itoa(kph, kphbuf, 10);
}

void displayFuel(){
  int readValue = analogRead(fuel);
  fuelMapValue = map(readValue,500,1023,0,5);
  itoa(fuelMapValue, fuelbuf, 10);
}

void displayRPM(){
  noInterrupts();
  Serial.println(rpm);
  interrupts();
  delay(150);
  rpmMapValue = map(rpm,0,13000,0,90);
}

void displayTrip(){
  itoa(trip, tripbuf, 10); 
}

void displayOdo(){
}

void displayEst(){ 
  int readValue = analogRead(fuel);
  estMapValue = map(readValue,500,1023,0,180); // How much is left of the max 18.0 L
  estDistLeft = estMapValue/fuelConsumption*10;
  itoa(estDistLeft, estbuf, 10);
}

void displayGear(){ 
}

void displayTime(){
}

void draw(){
  //Gear
  u8g.setFont(u8g_font_courR24); //font_osr26n
  u8g.setPrintPos(5, 40);
  u8g.print("N");
  //Speedometer
  u8g.setPrintPos(4, 80);
  u8g.print(kphbuf);
  u8g.setFont(u8g_font_5x7);
  u8g.setPrintPos(6, 100);
  u8g.print(" km/h");
  //Fuel
  u8g.setFont(u8g_font_5x7);
  u8g.setPrintPos(118,7);
  u8g.print("F"); 
  u8g.drawFrame(115, 0, 8, 50);
  u8g.drawBox(115, 0, 8, 10*fuelMapValue);
  //Tachometer
  u8g.setPrintPos(0, 1);
  u8g.print("RPM");
  u8g.drawFrame(20, 0, 90, 10);
  u8g.drawBox(20, 0, rpmMapValue, 10); 
  //Trip
  u8g.setPrintPos(0, 6);
  u8g.print(tripbuf);
  u8g.print(" km");
  //Odometer
  u8g.setPrintPos(55, 6);
  u8g.print("999 999");
  u8g.print(" km"); 
  //Clock
  u8g.setPrintPos(0,7);
  u8g.print("HH:MM");
  //Est. left
  u8g.setPrintPos(45, 7);
  u8g.print("Est.");
  u8g.print(estbuf);
  u8g.print(" km");
}

void screen(){
  u8g.firstPage();
  displayKPH();
  displayFuel();
  displayRPM();
  displayTrip();
  displayOdo();
  displayGear(); 
  displayTime();
  displayEst();
  do{
  draw();
  }while(u8g.nextPage() );
}

void loop(void){
  DoTimeAndDistanceCalcs();
  screen(); 
}
