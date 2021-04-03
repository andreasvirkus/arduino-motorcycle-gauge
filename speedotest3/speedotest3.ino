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

#define fuel A3          // Fuel sensor pin
#define displayButton 4  // Press button pin
#define screens 2        // Number of menu screens
#define holdTime 2000    // Time requirement to reset trip
#define debounceDelay 40 // Debounce period to prevent flickering when pressing or releasing the button
#define EEPROMtrip       // The trip address on EEPROM
#define EEPROModo        // The odo address on EEPROM

#include <LiquidCrystal.h>
#include <EEPROM.h>      

LiquidCrystal lcd(12, 11, 5, 4, 7, 6);

volatile byte wheelRotations=0;
boolean reedSwitchState; 
boolean lastReedSwitchState;
boolean ignoreDn = false;            // Whether to ignore the button release because the click+hold was triggered
float rotationTime;  
float distanceTravelledPerSecond;
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
int nullCounter=0;
int count;
int doWrite;
//Tach:
const int ignitionPin = 2;
const int ignitionInterrupt = 0;
const unsigned int pulsesPerRev = 1;
unsigned long lastPulseTime = 0;
unsigned long rpm = 0;

// Creating a bitmap block to be used in the fuel gauge.
byte block[8] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
};

void setup(){
  lcd.createChar(0, block);
  lcd.begin(16, 2);
  lcd.setCursor(0,0);
  lcd.print("===== Init =====");
  lcd.setCursor(0,1);
  delay(200);
  for(int i=0;i<16;i++){
    lcd.print(".");
    delay(50);
  }
  pinMode(1,OUTPUT);//tx
  pinMode(reed, INPUT);
  pinMode(displayButton, INPUT);
  pinMode(fuel, INPUT);
  pinMode(ignitionPin, INPUT);
  attachInterrupt(ignitionInterrupt, &ignitionIsr, RISING);
  attachInterrupt(0, countWheelRotations, FALLING);
  Serial.begin(9600);
}

void countWheelRotations(){
  wheelRotations++;
 } 

void DoTimeAndDistanceCalcs()
{
  rotationTime = millis() - lastmillis;
  if (rotationTime >= 250){ //Update every one second, this will be equal to reading frecuency (Hz).
     detachInterrupt(0);//Disable interrupt when calculating
     Serial.print(wheelRotations); //print the word "Hz".
     Serial.print("\t");
     Serial.print(rotationTime);
     Serial.print("\t");
     //print revolutions per second or Hz. And print new line or enter.
     //rotationTime = millis() - lastmillis;               // Period for one rotation (in ms!)
     //RPM = wheelRotations/60;                                // RPM = 1/Frequency  and times 3600 for ms -> minutes conversion
     distance = wheelRotations * wheelCircumference * (rotationTime/1000);     // Distance = revolutions * circumference and divided by 1000 for m -> km conversion
     trip += distance / 1000;
     Serial.print(distance);
     //distanceTravelledPerSecond = RPM * wheelCircumference;
     kph = distance * 3.6;
     Serial.print("\tK/ph=\t");
     Serial.println(kph);
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

//=========== GUI ================

void displayKPH(){
  lcd.setCursor(0,0);
  lcd.print("Speed ");
  lcd.setCursor(6, 0);
  lcd.print(kph, 0);
  lcd.print(" km/h");
}

void displayFuel(){
  lcd.setCursor(0,1);
  lcd.print("Fuel");
  lcd.setCursor(6, 1);
  int readValue = analogRead(fuel);
  int mapValue = map(readValue,500,1023,0,8);
  for(int j=0;j<mapValue;j++){
    lcd.write(byte(0));
  }
}

void displayRPM(){
  noInterrupts();
  Serial.println(rpm);
  interrupts();
  delay(150);
}

void displayTrip(){
  lcd.setCursor(0,0);
  lcd.print("Trip");
  lcd.setCursor(6,0);
  lcd.print(trip, 1);
  lcd.print(" km");
}

/*
void displayOdo(){
  lcd.setCursor(0,1);
  lcd.print("Odo");
  lcd.setCursor(6,0);
  lcd.print(totalDistance, 1);
  lcd.print(" km");
}
*/

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

void selectScreen(){
  screenReading = digitalRead(displayButton);
  
  // Test for button press and store the press down time
  if(screenReading == HIGH && lastScreenButtonState == LOW && millis()-btnDnTime > debounceDelay){
    btnDnTime = millis();
  }
  
  // Test for button hold
  if(screenReading == HIGH && lastScreenButtonState == HIGH && screen == 2 && millis()-btnDnTime > holdTime){
    trip = 0;
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

void screen1(){
  lcd.clear();
  displayKPH();
  displayFuel(); 
}

void screen2(){
  lcd.clear();
  //displayTime();
  //displayOdo();
  displayTrip();
  displayRPM();
}

void loop(){
  DoTimeAndDistanceCalcs();
  selectScreen();
  switchScreen();
  delay(250);
}
