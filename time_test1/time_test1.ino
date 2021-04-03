#include <Wire.h>
#include <avr/wdt.h>
#include "RTClib.h"

RTC_DS1307 rtc;
byte ch;

void setup () {
  wdt_disable();
  pinMode(13, OUTPUT);
  Serial.begin(9600);
  delay(250);
  Serial.println(F("Main Program Started"));  
  Wire.begin();
  rtc.begin();
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
  }
  rtc.adjust(DateTime(__DATE__, __TIME__));
}

void loop () {
    DateTime now = rtc.now();

    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    delay(1000);
    
    if (Serial.available()){
      ch=Serial.read();
      if(ch=='2'){
        WriteRam(10, 999);
      }//else if(ch=='3'){
        //ReadRam(10, 3);
      //}
      else if(ch=='9'){
        EnterFlashMode();
      }
      else if(ch=='1'){
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
  wdt_enable(WDTO_2S);
  wdt_reset();
  //while(1) {};
}

void WriteRam(int address,byte data){
  Wire.beginTransmission(0x68);     // Select DS1307
  Wire.write(address+8);             // address location starts at 8, 0-6 are date, 7 is control
  Wire.write(data);                  // send data
  Wire.endTransmission();
}
