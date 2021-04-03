#define hallSensor 0     // Hall sensor pin
#define ignitionSensor 0 
#define fuel A3          // Fuel sensor pin
#define fuelConsumption 6.6  // L / 100km

#define LCD_SID  9
#define LCD_SCLK 12
#define LCD_A0   7 
#define LCD_RST  6
#define LCD_CS   5

#define GreenPin 11
#define BluePin 10
#define RedPin 8

#include <U8glib.h>

U8GLIB_LM6059 u8g(LCD_SCLK, LCD_SID, LCD_CS, LCD_A0, LCD_RST);   // Pin assignments for the ST7565: u8g(SCK, MOSI, CS, A0, RST)

volatile byte wheelRotations=0;
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

void setup(void) {
  Serial.begin(9600); // Setup serial library at 9600 bps
  // Set RGB PWM pins to outputs
  pinMode(BluePin, OUTPUT);
  pinMode(GreenPin, OUTPUT);
  pinMode(RedPin, OUTPUT);
  pinMode(fuel, INPUT);
  analogWrite(BluePin, HIGH);
  analogWrite(GreenPin, HIGH);
  analogWrite(RedPin, HIGH);
  
  //attachInterrupt(ignitionSensor, ignitionIsr, RISING);
  attachInterrupt(hallSensor, countWheelRotations, FALLING);
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
  if (rotationTime >= 250){
     detachInterrupt(0);
     distance = wheelRotations * wheelCircumference * (rotationTime/1000);    
     trip += distance / 1000;
     kph = distance * 3.6;

     wheelRotations = 0; 
     lastmillis = millis();
     attachInterrupt(0, countWheelRotations, FALLING);
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

void displayKPH(){
  char buf[12];
  itoa(kph, buf, 10);
  u8g.setFont(u8g_font_gdr25);  //font_osr26n
  u8g.drawStr(4, 55, buf);
  u8g.drawStr(4, 72, " km/h");
}

void displayFuel(){
  int readValue = analogRead(fuel);
  int mapValue = map(readValue,500,1023,0,5);
  char buf[12];
  itoa(mapValue, buf, 10);
  u8g.drawStr(118, 7, "F"); 
  // x, y - top left corner ||  w, h - width, height
  //u8g.drawFrame(x, y, w, h);
  //u8g.drawBox(x, y, w, h);
  //glcd.drawrect(117, 0, 8, 50, BLACK);
  //glcd.fillrect(117, 0, 8, 10*mapValue, BLACK);
}

void displayRPM(){
  noInterrupts();
  Serial.println(rpm);
  interrupts();
  delay(150);
  int mapValue = map(rpm,0,13000,0,90);
  char buf[12];
  itoa(rpm, buf, 10); 
  u8g.setFont(u8g_font_5x7);
  u8g.drawStr(0, 1, "RPM");
  u8g.drawFrame(20, 0, 90, 10);
  u8g.drawBox(20, 0, mapValue, 10); 
  u8g.drawStr(2,2,buf);
}

void displayTrip(){
  char buf[12];
  itoa(trip, buf, 10); 
  u8g.setFont(u8g_font_5x7);
  u8g.drawStr(0, 6, buf);
  //u8g.setPrintPos(x, y);
  //u8g.print(buf, 1);  - prints with the precision of a 100m
  u8g.drawStr(32, 6, "km");
}

void displayOdo(){
  u8g.drawStr(55, 6, "999 999");
  u8g.drawStr(98, 6, "km");
}

void displayEst(){ 
  int readValue = analogRead(fuel);
  int mapValue = map(readValue,500,1023,0,180); // How much is left of the max 18.0 L
  estDistLeft = mapValue/fuelConsumption*10;
  char buf[12];
  itoa(estDistLeft, buf, 10);
  //u8g.setFont(u8g_font_5x7);
  //u8g.drawStr(45, 7, "Est.");
  //u8g.drawStr(74, 7, buf);
  //u8g.drawStr(95, 7, "km");
}

void displayGear(){ 
  //u8g.setFont(u8g_font_courR24);
  //u8g.drawStr(15, 4, "N"); 
}

void displayTime(){
    //u8g.setFont(u8g_font_5x7);
    //u8g.drawStr(0,7,"HH:MM");
}

void screen1(){
  clear_screen();
  displayKPH();
  displayRPM();
  displayTrip();
  displayOdo();
  displayFuel();
  displayGear(); 
  displayTime();
  displayEst();
}
void loop(void){
  DoTimeAndDistanceCalcs();
  screen1();
  delay(250);  
}
