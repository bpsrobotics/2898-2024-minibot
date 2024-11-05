#include <Arduino.h>
#include <Servo.h>

Servo talonSRX;

const int forwardChannelPin = 2; // Channel that recives signal.  Channel 1
int forwardSpeed = 0;

// function declarations here:
int getChannelValue(int);

void setup() {
  Serial.begin(9600);
  pinMode(forwardChannelPin, INPUT);
  talonSRX.attach(6);
}

void loop() {
  forwardSpeed = getChannelValue(forwardChannelPin);
  Serial.println(forwardSpeed);
  talonSRX.writeMicroseconds(forwardSpeed);
}

int getChannelValue(int channelPin) {
  float channelValue = pulseIn(channelPin, HIGH);
  if (channelValue < 100) { // not receiving any signal
    channelValue = 1500; // sets channel value to neutral position
  }
  channelValue = round(channelValue);
  channelValue = constrain(channelValue, 1000, 2000);

  return round(channelValue);
}