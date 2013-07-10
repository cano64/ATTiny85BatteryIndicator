/*
  attiny85 battery voltage level meter for 4.5V battery pack
  reads VCC and compares to internal 1V1 voltage
  light up leds based on voltage remaining
  
  3 LEDS: Green, Yellow, Red (blinking Red, rapid blinking Red)
    G       .. 70%
    GY  70% .. 60%
    Y   60% .. 40%
    RY  40% .. 30%
    R   30% .. 10%
    rR  10% .. 0%
    rr  0%  .. 
                100% = 4500mV
                  0% = 2600mV


// Atmel ATTiny85 pinout
//
//                          +---\/---+
//              RESET  PB5  | 1    8 |  VCC
//           (A3) (D3) PB3  |*2~   7*|  PB2 (D2) (A1)
//           (A2) (D4) PB4  |*3~  ~6 |  PB1 (D1)
//                     GND  | 4   ~5*|  PB0 (D0) (A0)
//                          +--------+
//       *analog, ~PWM, Dn Arduino pin, An Analog channel

*/

#include "sdelay.h"

#define BATT100 4500
#define BATT0 2600
#define BATT_STEP ((BATT100 - BATT0) / 100)
#define BATT10 (BATT0 + 10*BATT_STEP)
#define BATT30 (BATT0 + 30*BATT_STEP)
#define BATT40 (BATT0 + 40*BATT_STEP)
#define BATT60 (BATT0 + 60*BATT_STEP)
#define BATT70 (BATT0 + 70*BATT_STEP)

#define BLINK_TIME_ON_SLOW 500
#define BLINK_TIME_OFF_SLOW 500
#define BLINK_TIME_ON_FAST 100
#define BLINK_TIME_OFF_FAST 900

#define LED_RED 0
#define LED_YELLOW 1
#define LED_GREEN 2


byte redStatus = 0; //last red status (for blinking)
int sleepTime = 500; //initial time between readings, will decrease for blinking

void setup() {
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  setLEDs(1, 1, 1);
  delay(1000);  
  setLEDs(0, 0, 0);
}


void  loop() {
  int vcc = readVCC();
  if (vcc > BATT70) {
    setLEDs(1, 0, 0);
  } else if (vcc >  BATT60) {
    setLEDs(1, 1, 0);
  } else if (vcc >  BATT40) {
    setLEDs(0, 1, 0);
  } else if (vcc >  BATT30) {
    setLEDs(0, 1, 1);
  } else if (vcc >  BATT10) {
    setLEDs(0, 0, 1);
  } else if (vcc >  BATT0) {
//    setLEDs(0, 0, redStatus && 0b0100);
    setLEDs(0, 0, redStatus && 0b0001);
  } else {
    sleepTime = 100;
    setLEDs(0, 0, redStatus && 0b0001);
  }
  redStatus++;
//  delay(125);
  sdelay(sleepTime);
}

void setLEDs(byte G, byte Y, byte R) {
  digitalWrite(LED_GREEN, G);
  digitalWrite(LED_YELLOW, Y);
  digitalWrite(LED_RED, R);
}

//reads internal 1V1 reference against VCC
//return number 0 .. 1023
int analogReadInternal() {
  #if defined(__AVR_ATtiny84__) 
    ADMUX = _BV(MUX5) | _BV(MUX0); // For ATtiny84
  #elif defined(__AVR_ATtiny85__) 
    ADMUX = _BV(MUX3) | _BV(MUX2); // For ATtiny85
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);  // For ATmega328
  #endif 
  delay(1); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  uint8_t low = ADCL;
  return (ADCH << 8) | low; 
}

//calculate VCC based on internal referrence
//return voltage in mV
int readVCC() {
  return ((uint32_t)1024 * (uint32_t)1100) / analogReadInternal();
}




//
