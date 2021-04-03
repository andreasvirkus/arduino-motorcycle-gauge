//Motorcycle LCD speedometer
//RPM->Km/ph formula  [ (2 * (pi) * r * rpm * 60) / 1000  ]
//Wheel circumference = diameter * pi

#include <LiquidCrystal.h>

#define reedSwitchPin A0
#define fuel A3
  
  float rotationTime;  
  float distanceTravelledPerSecond;
  float wheelCircumference = 74.581;  // Circumference of the tyre (inches)
  float RPM;
  //float speeds;
  float maximumSpeed;
  float mph=0.00;
  unsigned int distanceTravelled;
  unsigned long distance = 0; 
  unsigned long revoloutions = 0;
  unsigned long startRotation = 0;  
  unsigned long endRotation = 0;  
  //unsigned long timeInSeconds;
  //unsigned long timeInMilliseconds;  
  //unsigned long lastMillis;
  boolean reedSwitchState; 
  boolean lastReedSwitchState;
  int buttonState;
  int val;
  int val2;
 
LiquidCrystal lcd(12,11,5,4,3,2); // lcd(12,11,5,4,3,2); <- minu setup  || lcd(7,8,9,10,11,12); <- originaal

byte bar[8] = {
  B1111111,
  B1111111,
  B1111111,
  B1111111,
  B1111111,
  B1111111,
  B1111111,
};
 
void DoTimeAndDistanceCalcs()
{
    revoloutions ++;  
    endRotation = millis();  
    rotationTime = endRotation - startRotation;  
    startRotation = endRotation;  
    RPM = (60000 / rotationTime);  
    distance = RPM * wheelCircumference * 60 / 1000;
    distanceTravelled = revoloutions * wheelCircumference / 100;  
    distanceTravelledPerSecond = RPM * wheelCircumference; 
    mph = distanceTravelledPerSecond / 100000 * 60;
    //distanceTravelledPerSecond = distanceTravelledPerSecond / 100000 * 60;
    //speeds = distanceTravelledPerSecond;  
    //timeInSeconds = millis();  
    //timeInSeconds = timeInMilliseconds / 1000;  
}
 
void setup(){
  pinMode(fuel, INPUT);
  pinMode(reedSwitchPin, INPUT);
  lcd.print("Init..");
  delay(50);
  Serial.begin(9600);
  buttonState = digitalRead(reedSwitchPin);
  lcd.begin(16,2);
  lcd.setCursor(0,0);
  lcd.print("Speed");
  lcd.setCursor(0,1);
  lcd.print("Dist.");
}
void loop(){
  displayMPH();  
}

void displayMPH(){
  val = digitalRead(reedSwitchPin);
  delay(10);
  val2 = digitalRead(reedSwitchPin);
  if (val == val2) {
    if (val != buttonState) {
      lastReedSwitchState = reedSwitchState;  
      reedSwitchState = val;   
      if((lastReedSwitchState == LOW) && (reedSwitchState == HIGH)){  
        DoTimeAndDistanceCalcs();
 
        lcd.setCursor(7, 0);
        lcd.print(mph);
        lcd.setCursor(7, 1);
        lcd.print(distance);
        lcd.print(" m");
        delay(50); 
       }
    }
  buttonState = val ;
  }
}
