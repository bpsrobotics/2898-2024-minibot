#include <Arduino.h>
#include <Servo.h>

Servo talonSRX;

// Motor pins
#define frontLeftMotorPin 6
#define frontRightMotorPin 9
#define backLeftMotorPin 10
#define backRightMotorPin 11

#define rightStickHorizontalPin 2
#define rightStickVerticalPin 3
#define leftStickVerticalPin 4
#define leftStickHorizontalPin 5

#define NOISE_THRESHOLD 100

int forwardSpeed = 0;

struct ChInfo {
    /** The physical pin the channel is connected to. */
    uint8_t pin;
    /** Was the pin `HIGH` last tick? */
    bool wasOn;
    /** Did `value` change this tick? */
    bool valueChanged;
    /** The last time the pin was detected to be `HIGH`, in microseconds since startup. */
    unsigned long onUS;
    /** The value of the channel, from `-1.0` to `1.0`. */
    float value;
};

struct {
    ChInfo rightStickHorizontal = {rightStickHorizontalPin, false, false, 0, 0.0f};
    ChInfo rightStickVertical = {rightStickVerticalPin, false, false, 0, 0.0f};
    ChInfo leftStickVertical = {leftStickVerticalPin, false, false, 0, 0.0f};
    ChInfo leftStickHorizontal = {leftStickHorizontalPin, false, false, 0, 0.0f};
    // ChInfo channel = {/* pin */2, false, 0};
} channels;

void updateChannel(ChInfo& channel) {
    const bool newState = digitalRead(channel.pin);
    const bool oldState = channel.wasOn;
    channel.valueChanged = false;
    if (newState != oldState) {
        if (newState) {
            // store rising time
            channel.onUS = micros();
            channel.wasOn = true;
        } else {
            channel.wasOn = false;
            // how long was the pulse?
            unsigned long delta = micros() - channel.onUS;
            if (delta < NOISE_THRESHOLD) return;
            channel.value = (float)((long)constrain(delta, 1000, 2000) - 1500) / 500.0f;
            channel.valueChanged = true;
        }
    }
}

void setup() {
    Serial.begin(9600);
    pinMode(rightStickHorizontalPin, INPUT);
    pinMode(rightStickVerticalPin, INPUT);
    pinMode(leftStickVerticalPin, INPUT);
    pinMode(leftStickHorizontalPin, INPUT);
    talonSRX.attach(6);
}

void loop() {
    updateChannel(channels.rightStickHorizontal);
    updateChannel(channels.rightStickVertical);
    updateChannel(channels.leftStickHorizontal);
    updateChannel(channels.leftStickVertical);
    // should be interpretable by the Serial Plotter in theory
    Serial.println(
        String("RSTICKH:") + channels.rightStickHorizontal.value
        + ",RSTICKV:" + channels.rightStickVertical.value
        + ",LSTICKH:" + channels.leftStickHorizontal.value
        + ",LSTICKV:" + channels.leftStickVertical.value
    );
}