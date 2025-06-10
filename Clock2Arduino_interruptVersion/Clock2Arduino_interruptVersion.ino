// Clock2Arduino
// version 0.0.1, 2025-06-10
// by l8-e https://linktr.ee/l8.minus.e
// Repository: https://github.com/l8-e/modules-Clock2Arduino/
// Code is licences with MIT license

// This code expects an Arduino Nano being wired as shown on https://github.com/l8-e/modules-Clock2Arduino/
// This allows the Arduino to read on D03 an external clock signal on Eurorack level.
// In addition, this code manages an internal clock signal, which is send to D02.
// If no plug is present in the jack, the Arduino will read its internal signal via D03.

// This version of the code use Arduino's hardware interrupts to react to the clock signal instead of pulling 
// the status of the clock pin in the loop. This was suggested by asciisynth (https://blinkl.info) via instagram. 

#define CLOCK_IN 3  // internal clock input is read via D03
#define CLOCK_OUT 2 // internal clock output is sent via D02
#define LED_OUT 13  // led is connected to pin D13


#define USE_INTERNAL_PULLUP true // if true, the internall pullup on d03 is used instead of an external resistor, connected to power. 
// this simplifies the schematics and was suggested by asciisynth (https://blinkl.info) via instagram. 

#define SYNC_LED_TO_INTERNAL false // if true, the led is flashes in sync with the internal clock. if false, the LED is synced to the external clock signal

const unsigned long cycleLen=250; // this defines the cycle length of the internal clock. One complete tick lasts cycleLen milli seconds
const unsigned long upTime=50; // this defines how many milli seconds of each cycle the clock signal is set to HIGH



bool externalClockWasHigh;
unsigned long lastHigh;
unsigned long ticks[10];
byte tickPos;
unsigned long averageTick;


bool internalClockIsHigh;
unsigned long internalClockIsHighSince;
unsigned long internalClockLastCycleStart;

void setup() {
  externalClockWasHigh = false;
  internalClockIsHigh=false;
  internalClockIsHighSince=0;
  internalClockLastCycleStart=0;

  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  
  pinMode(CLOCK_OUT, OUTPUT);
  pinMode(LED_OUT,   OUTPUT);

  if(USE_INTERNAL_PULLUP){
    pinMode(CLOCK_IN,  INPUT_PULLUP); 
  }else{
    pinMode(CLOCK_IN,  INPUT); 
  }

  attachInterrupt(digitalPinToInterrupt(CLOCK_IN), clockSignalInterrupt, CHANGE);

  Serial.println("setup done.");
} // end of Setup

void handleInternalClock(unsigned long t){
  if(internalClockIsHigh){
    if (t-internalClockIsHighSince >= upTime){
      digitalWrite(CLOCK_OUT,LOW);
      internalClockIsHigh = false;
      if(SYNC_LED_TO_INTERNAL){
        digitalWrite(LED_OUT,LOW);
      }
    }
  }
  if(t- internalClockLastCycleStart >= cycleLen){
    digitalWrite(CLOCK_OUT,HIGH);
    if(SYNC_LED_TO_INTERNAL){
      digitalWrite(LED_OUT,HIGH);
    }
    internalClockIsHigh = true;
    internalClockIsHighSince=t;
    internalClockLastCycleStart = t;
  }
}

void clockSignalInterrupt(){
  externalClockWasHigh= !externalClockWasHigh;
  if(externalClockWasHigh){
    if(!SYNC_LED_TO_INTERNAL){
      digitalWrite(LED_OUT,HIGH);
    }
    externalClockWasHigh = true;
  }else{
    if(!SYNC_LED_TO_INTERNAL){
      digitalWrite(LED_OUT,LOW);
    }
    externalClockWasHigh = false;
  }
}

void loop() {
  unsigned long t = millis();
  handleInternalClock(t);
}