#include <Dogm.h>

int a0Pin = PIN_A0_DEFAULT;      // address line a0 for the dogm module
int sensorPin = 0;  // analog input

Dogm dogm(a0Pin);

void setup() {
  dogm.clear();
  dogm.start();
  dog_DrawStr(0,2,font_7x14, "Hello");
  dog_DrawStr(0,5,font_6x9, "world");
}

void loop(){}
