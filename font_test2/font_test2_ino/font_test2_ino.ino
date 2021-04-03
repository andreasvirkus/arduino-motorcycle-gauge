#include <Dogm.h>
#define GREEN 11
#define BLUE 10
#define RED 8

int a0Pin = PIN_A0_DEFAULT;      // address line a0 for the dogm module
int sensorPin = 0;  // analog input

Dogm dogm(a0Pin);

void setup() {
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(RED, OUTPUT);
  digitalWrite(GREEN, LOW);
  digitalWrite(BLUE, LOW);
  digitalWrite(RED, LOW);
  Serial.begin(9600);
  dogm.start();
  do{
  dogm.setFont(font_7x14);
  dogm.setXY(10, 10);
  dogm.print("Hello world");
  }while(dogm.next());
}

void loop() {
  char buf[12];
  itoa(kph, buf, 10);
  dogm.start();
  do{
  dogm.setFont(font_7x14);
  dogm.setXY(20, 4);
  dogm.print(buf);
  }while(dogm.next());

  int readValue = analogRead(fuel);
  int mapValue = map(readValue,500,1023,0,5);
  char buf[12];
  itoa(mapValue, buf, 10);
  do{
  dogm.
  glcd.drawstring(118, 7, "F"); 
  glcd.drawrect(117, 0, 8, 50, BLACK);
  glcd.fillrect(117, 0, 8, 10*mapValue, BLACK);
}
