#include <Arduino.h>

// motors, outputs
const int wheelPWMPin = 6;
const int forwardChannelPin = 2; // Channel 1
int speed = 0;

// function declarations here:
int getChannelValue(int);

void setup() {
  Serial.begin(9600);
  pinMode(wheelPWMPin, OUTPUT);
  pinMode(forwardChannelPin, INPUT);
}

void loop() {
  speed = getChannelValue(forwardChannelPin);
  Serial.println(speed);
  analogWrite(wheelPWMPin, speed);
  delay(10);
}

int getChannelValue(int channelPin) {
  float channelValue = pulseIn(channelPin, HIGH);
  if (channelValue < 100) { // not receiving any signal
    channelValue = 1500; // sets channel value to neutral position
  }
  channelValue = round(channelValue);
  channelValue = constrain(channelValue, 1000, 2000);
  channelValue = map(channelValue, 1000, 2000, 0, 255); // constrants channel value to be between 0 and 255

  // if channel value was close to 0 or 255, set channel value to 0 or 255
  int roundAmount = 5;
  if (channelValue <= roundAmount) {
    channelValue = 0;
  }
  else if (channelValue >= 255 - roundAmount) {
    channelValue = 255;
  }
  else if (abs(127 - channelValue) <= roundAmount) {
    channelValue = 127;
  }

  return round(channelValue);
}