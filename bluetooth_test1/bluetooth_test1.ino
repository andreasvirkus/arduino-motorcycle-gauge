#include <avr/wdt.h>

byte ch;
void setup(){
  pinMode(13, OUTPUT);
  Serial.begin(9600);
  Serial.println(F("Main Program Started"));
}
void loop(){
    if (Serial.available()){
      ch=Serial.read();
      if(ch=='9'){
        EnterFlashMode();
      }else if(ch=='1'){
        digitalWrite(13, HIGH);
      }else if(ch=='0'){
        digitalWrite(13, LOW);
      }else{
        Serial.println(ch);
      }
    }
}
void EnterFlashMode(){
  Serial.println(F("Entering Bootloader"));
  wdt_enable(WDTO_30MS);
  Serial.println(F("Something's not right.")); // "Something     " gets typed out, then crashes.
}
