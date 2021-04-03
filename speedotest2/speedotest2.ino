//Motorcycle LCD Dash.
// http://hackaday.com/2011/08/26/a-beginners-guide-to-burning-arduino-bootloaders/

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


#define reed A0          // Reed switch pin 
#define fuel A3          // Fuel sensor pin
#define displayButton 9  // Press button pin
#define screens 2        // Number of menu screens
#define holdTime 2000    // Time requirment to reset trip
#define debounceDelay 40 // Debounce period to prevent flickering when pressing or releasing the button
#define EEPROMtrip       // The trip address on EEPROM
#define EEPROModo        // The odo address on EEPROM

#include <LiquidCrystal.h>
#include <EEPROM.h>      

LiquidCrystal lcd(12, 11, 5, 4, 3, 6);

boolean reedSwitchState; 
boolean lastReedSwitchState;
boolean ignoreDn = false;            // Whether to ignore the button release because the click+hold was triggered
float rotationTime;  
float distanceTravelledPerSecond;
float wheelCircumference = 1.9452844;   // Circumference of the tyre (inches) = 1.9452844 meters
float RPM;
float kph = 0.00;
float distance = 0;
unsigned long startRotation = 0;  
unsigned long endRotation = 0;
unsigned long startPress = 0;
unsigned long rotations = 0;
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
  
  
  // Arduino Timer1 setup
  //=============
  // Stop interrupts
  cli();
  // Set Timer1 interrupt at 1kHz
  TCCR1A = 0;// Set entire TCCR1A register to 0
  TCCR1B = 0;// Same for TCCR1B
  TCNT1  = 0;
  // Set timer count for 1khz increments
  OCR1A = 1999;// = (1/1000) / ((1/(16*10^6))*8) - 1
  // Turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS11 bit for 8 prescaler
  TCCR1B |= (1 << CS11);   
  // Enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  // Allow interrupts
  sei();
  //=============
  // END TIMER SETUP
  
  Serial.begin(9600);
}

ISR(TIMER1_COMPA_vect) { //Interrupt at freq of 1kHz to measure reed switch
  if(timer==0){
    val = digitalRead(reed);
    timer++;
  }
  else if(timer==10){
    val2 = digitalRead(reed);
    if (val == val2) {
      if (val != reedButtonState) {
        lastReedSwitchState = reedSwitchState;  
        reedSwitchState = val;   
        if((lastReedSwitchState == HIGH) && (reedSwitchState == LOW)){  
          //startRotation = millis();
          DoTimeAndDistanceCalcs();
          nullCounter = 0;
        }
      }
      reedButtonState = val;
    } 
    timer = 0;
  }
  else{
    timer ++; // Increment both timers
    nullCounter++;
  }
  if(nullCounter > 2000){
    kph = 0;
    nullCounter = 0;
    doWrite = 1;
  }
}

void DoTimeAndDistanceCalcs()
{
    rotations++;
    endRotation = millis();
    rotationTime = endRotation - startRotation;               // Period for one rotation (in ms!)
    startRotation = endRotation;                              // Hertz = rotations per second
    RPM = 1/(rotationTime*1000)*60;                                // RPM = 1/Frequency  and times 3600 for ms -> minutes conversion
    distance = rotations * wheelCircumference / 1000;         // Distance = revolutions * circumference and divided by 1000 for m -> km conversion
    distanceTravelledPerSecond = RPM * wheelCircumference;
    kph = distanceTravelledPerSecond *3600;
}

void displayKPH(){
  lcd.setCursor(0,0);
  lcd.print("Speed ");
  lcd.setCursor(6, 0);
  lcd.print(kph);
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

void displayTrip(){
  lcd.setCursor(0,0);
  lcd.print("Trip");
  lcd.setCursor(6,0);
  lcd.print(distance, 1);
  lcd.print(" km");
}

/*
void displayTrip(){
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
    distance = 0;
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
  displayTrip();
//  displayOdo();
}

void loop(){
  selectScreen();
  switchScreen();
  delay(150);  
}
