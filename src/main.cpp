#include <Arduino.h>
#include <Servo.h>
#include <Wire.h>
#include <MPU6050.h>
#include "mecanum.hpp"

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

// MPU6050 mpu;
// MecanumDrive drive(0.8, OF_REDUCE_EQUALLY);

int forwardSpeed = 0;

struct ChInfo {
    /* The physical pin the channel is connected to. */
    uint8_t pin;
    /* Was the pin `HIGH` last time? */
    bool wasOn;
    /* Did `value` change this time? */
    bool valueChanged;
    /* The last time the pin was detected to be `HIGH`, in microseconds since startup. */
    unsigned long onUS;
    /* The value of the channel, from `-1.0` to `1.0`. */
    float value;
    /* The minimum and maximum values of the channel, because not all of them are exactly 1000-2000us. */
    long lowest, highest;
};

struct {
    // volatile because they're used in interrupts
    volatile ChInfo rightStickHorizontal = {rightStickHorizontalPin, false, false, 0, 0.0f, 1000, 2000};
    volatile ChInfo rightStickVertical = {rightStickVerticalPin, false, false, 0, 0.0f, 1000, 2000};
    volatile ChInfo leftStickVertical = {leftStickVerticalPin, false, false, 0, 0.0f, 1000, 2000};
    volatile ChInfo leftStickHorizontal = {leftStickHorizontalPin, false, false, 0, 0.0f, 1000, 2000};
    // ChInfo channel = {/* pin */2, false, false, 0, 0.0f};
} channels;

void updateChannel(volatile ChInfo& channel) {
    const bool newState = digitalRead(channel.pin);
    const bool oldState = channel.wasOn;
    channel.valueChanged = false;
    if (newState != oldState) {
        const unsigned long now = micros();
        if (newState) {
            // store rising time
            channel.onUS = now;
            channel.wasOn = true;
        } else {
            channel.wasOn = false;
            // how long was the pulse?
            long delta = now - channel.onUS;
            if (delta < NOISE_THRESHOLD) return;
            channel.value = (float)(map(constrain(delta, channel.lowest, channel.highest), channel.lowest, channel.highest, 1000, 2000) - 1500) / 500.0f;
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
    attachInterrupt(digitalPinToInterrupt(rightStickHorizontalPin), []{ updateChannel(channels.rightStickHorizontal); }, CHANGE);
    attachInterrupt(digitalPinToInterrupt(rightStickVerticalPin), []{ updateChannel(channels.rightStickVertical); }, CHANGE);
    attachInterrupt(digitalPinToInterrupt(leftStickVerticalPin), []{ updateChannel(channels.leftStickVertical); }, CHANGE);
    attachInterrupt(digitalPinToInterrupt(leftStickHorizontalPin), []{ updateChannel(channels.leftStickHorizontal); }, CHANGE);
    // talonSRX.attach(6);
    // mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_16G);
    // mpu.setAccelPowerOnDelay(MPU6050_DELAY_3MS);
    // 
    // mpu.setIntFreeFallEnabled(false);
    // mpu.setIntZeroMotionEnabled(false);
    // mpu.setIntMotionEnabled(false);
    // 
    // mpu.setDHPFMode(MPU6050_DHPF_5HZ);
}

void loop() {
    // should be interpretable by the Serial Plotter in theory
    Serial.println(
        String("RSTICKH:") + channels.rightStickHorizontal.value
        + ",RSTICKV:" + channels.rightStickVertical.value
        + ",LSTICKH:" + channels.leftStickHorizontal.value
        + ",LSTICKV:" + channels.leftStickVertical.value
    );
}