//bike speedometer
//by Amanda Ghassaei 2012
//http://www.instructables.com/id/Arduino-Bike-Speedometer/
//outputs speed of bicycle to LCD

//calculations
//tire radius ~ 13.5 inches
//circumference = pi*2*r =~85 inches
//max speed of 35mph =~ 616inches/second
//max rps =~7.25

#define reed A0
#define fuel A3

#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

float radius = 13.5; // tire radius (in inches)
int reedVal;
long timer = 0; // time between one full rotation (in ms)
float mph = 0.00;
float circumference;

int maxReedCounter = 150; //min time (in ms) of one rotation (for debouncing)
int reedCounter;

byte bar[8] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
};

void setup(){
  lcd.createChar(0, bar);
  lcd.begin(16, 2);
  reedCounter = maxReedCounter;
  circumference = 2*3.14*radius;
  pinMode(1,OUTPUT);//tx
  pinMode(reed, INPUT);
  pinMode(fuel, INPUT);
  lcd.print("init..");
  
  // TIMER SETUP- the timer interrupt allows preceise timed measurements of the reed switch
  //for mor info about configuration of arduino timers see http://arduino.cc/playground/Code/Timer1
  cli();//stop interrupts
  //Set timer1 interrupt at 1kHz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;
  // set timer count for 1khz increments
  OCR1A = 1999;// = (1/1000) / ((1/(16*10^6))*8) - 1
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS11 bit for 8 prescaler
  TCCR1B |= (1 << CS11);   
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei();//allow interrupts
  //END TIMER SETUP
  
  Serial.begin(9600);
}

ISR(TIMER1_COMPA_vect) {//Interrupt at freq of 1kHz to measure reed switch
  reedVal = digitalRead(reed); //get val of A0
  if (reedVal){ //if reed switch is closed
    if (reedCounter == 0){ //min time between pulses has passed
    mph = (56.8*float(circumference))/float(timer); //calculate miles per hour
      timer = 0; //reset timer
      reedCounter = maxReedCounter; //reset reedCounter
    }
    else{
      if (reedCounter > 0){ //don't let reedCounter go negative
        reedCounter -= 1; //decrement reedCounter
      }
    }
  }
  else{ //if reed switch is open
    if (reedCounter > 0){ //don't let reedCounter go negative
      reedCounter -= 1; //decrement reedCounter
    }
  }
  if (timer > 2000){
    mph = 0;
  }
  else{
    timer += 1; //increment timer
  } 
}

void displayMPH(){
  lcd.clear();
  lcd.noDisplay();
  lcd.setCursor(0,0);
  lcd.print("S  ");
  lcd.print(mph);
  lcd.setCursor(0, 1);
  int readValue = analogRead(fuel);
  int mapValue = map(readValue, 500, 1023, 0, 15);
  lcd.print("F");
  for(int i=0;i<mapValue;i++){
    lcd.write(byte(0));
  }
  lcd.display();
  delay(50);
}

void loop(){
  displayMPH();
}
