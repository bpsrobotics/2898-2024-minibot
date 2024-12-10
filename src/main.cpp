#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>
#include "mecanum.hpp"
#include "servo-wrapper.hpp"

// Motor pins
#define frontLeftMotorPin 5
#define backLeftMotorPin 6
#define backRightMotorPin 9
#define frontRightMotorPin 10

#define rightStickHorizontalPin 2
#define rightStickVerticalPin 3
#define leftStickVerticalPin 4
#define leftStickHorizontalPin 7

#define NOISE_THRESHOLD 100
#define CHANNEL_DEADZONE 50
#define CHANNEL_DEADZONE_CENTER 45

// MPU6050 mpu;
MecanumDrive drive(1.0 /* todo */, OF_REDUCE_EQUALLY, AF_FIT);

ServoWrapper frontLeft(frontLeftMotorPin);
ServoWrapper frontRight(frontRightMotorPin);
ServoWrapper backLeft(backLeftMotorPin);
ServoWrapper backRight(backRightMotorPin);

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
    /* for debugging */
    long delta, deltaNormalized;
    /* The value of the channel, from `-1.0` to `1.0`. */
    float value;
};

struct {
    // volatile because they're used in interrupts
    volatile ChInfo rightStickHorizontal = {rightStickHorizontalPin, false, false, 0, 0, 0, 0.0f};
    volatile ChInfo rightStickVertical = {rightStickVerticalPin, false, false, 0, 0, 0, 0.0f};
    volatile ChInfo leftStickVertical = {leftStickVerticalPin, false, false, 0, 0, 0, 0.0f};
    volatile ChInfo leftStickHorizontal = {leftStickHorizontalPin, false, false, 0, 0, 0, 0.0f};
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
            channel.delta = delta;
            if (delta >= 2000 - CHANNEL_DEADZONE) delta = 2000;
            if (delta <= 1000 + CHANNEL_DEADZONE) delta = 1000;
            if (abs(delta - 1500) <= CHANNEL_DEADZONE_CENTER) delta = 1500;
            channel.deltaNormalized = delta;
            channel.value = (float)(delta - 1500) / 500.0f;
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
    frontLeft.begin();
    frontRight.begin();
    backLeft.begin();
    backRight.begin();
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
    DriveValues values = drive.calculate(
        vec2(
            channels.rightStickHorizontal.value,
            channels.rightStickVertical.value
        ),
        0.0
        // channels.leftStickHorizontal.value
    );
    frontLeft.drive(values.frontLeft);
    frontRight.drive(values.frontRight);
    backLeft.drive(values.backLeft);
    backRight.drive(values.backRight);
    // float v = channels.rightStickVertical.value;
    // frontLeft.drive(v);
    // frontRight.drive(v);
    // backLeft.drive(v);
    // backRight.drive(v);
    Serial.println(
        String("RH:") + channels.rightStickHorizontal.value
            + ", RV:" + channels.rightStickVertical.value
            + ", LH:" + channels.leftStickHorizontal.value
            + ", LV:" + channels.leftStickVertical.value
    );
}