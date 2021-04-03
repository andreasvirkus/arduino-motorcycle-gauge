/* Serial Loop */

#include <SoftwareSerial.h>

#define rxPin 8
#define txPin 9

SoftwareSerial mySerial(rxPin, txPin); // RX, TX
char myChar ;

void setup() {
  Serial.begin(9600);   
  Serial.println("AT");

  mySerial.begin(38400);
  mySerial.println("AT");
}

void loop() {
  while (mySerial.available()) {
    myChar = mySerial.read();
    Serial.print(myChar);
  }

  while (Serial.available()) {
    myChar = Serial.read();
    Serial.print(myChar); //echo
    mySerial.print(myChar);
  }
}
