// http://kokoraskostas.blogspot.com/2013/12/arduino-inductive-spark-plug-sensor.html
#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 7, 6);

const int ignitionPin = 3;
const int ignitionInterrupt = 1;
const unsigned int pulsesPerRev = 1;
unsigned long lastPulseTime = 0;
unsigned long rpm = 0;

void setup() { 
  lcd.begin(16,2); 
  Serial.begin(9600);  
  pinMode(ignitionPin, INPUT);
  attachInterrupt(ignitionInterrupt, &ignitionIsr, RISING);
}

void ignitionIsr(){
  unsigned long now = micros();
  unsigned long interval = now - lastPulseTime;
  if (interval > 5000){
     rpm = 60000000UL/(interval * pulsesPerRev);
     lastPulseTime = now;
  }  
}

void loop() {
  noInterrupts();
  Serial.println(rpm);
  lcd.clear();
  lcd.print(rpm);
  interrupts();
  delay(150);
}
