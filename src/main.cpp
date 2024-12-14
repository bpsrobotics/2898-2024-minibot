#include <Arduino.h>
#include <Wire.h>
#include <Arduino_LSM6DS3.h>
#include "mecanum.hpp"
#include "servo-wrapper.hpp"

// Motor pins
#define frontLeftMotorPin 5
#define backLeftMotorPin 6
#define backRightMotorPin 9
#define frontRightMotorPin 10

// Controller pins
#define rightStickHorizontalPin 2
#define rightStickVerticalPin 3
// #define leftStickVerticalPin 4
#define leftStickHorizontalPin 4
#define switchAPin 7
#define switchDPin 8
#define switchBPin 12

#define fanPin 11

#define NOISE_THRESHOLD 100
#define CHANNEL_DEADZONE 50
#define CHANNEL_DEADZONE_CENTER 45
// #define DO_FIELD_ORIENTED

MecanumDrive drive(1.0f /* todo */, OF_REDUCE_EQUALLY, AF_FIT, TF_FLIP_X | TF_ROTATE_270DEG);

ServoWrapper frontLeft(frontLeftMotorPin);
ServoWrapper frontRight(frontRightMotorPin);
ServoWrapper backLeft(backLeftMotorPin);
ServoWrapper backRight(backRightMotorPin);

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
    // long delta, deltaNormalized;
    /* The value of the channel, from `-1.0` to `1.0`. */
    float value;
};

struct {
    // volatile because they're used in interrupts
    volatile ChInfo rightStickHorizontal = {rightStickHorizontalPin, false, false, 0, 0.0f};
    volatile ChInfo rightStickVertical = {rightStickVerticalPin, false, false, 0, 0.0f};
    // volatile ChInfo leftStickVertical = {leftStickVerticalPin, false, false, 0, 0.0f};
    volatile ChInfo leftStickHorizontal = {leftStickHorizontalPin, false, false, 0, 0.0f};
    volatile ChInfo switchA = {switchAPin, false, false, 0, 0.0f};
    volatile ChInfo switchD = {switchDPin, false, false, 0, 0.0f};
#ifdef DO_FIELD_ORIENTED
    volatile ChInfo switchB = {switchBPin, false, false, 0, 0.0f};
#endif
    // ChInfo channel = {/* pin */2, false, false, 0, 0.0f};
} channels;

#ifdef DO_FIELD_ORIENTED
bool wasFieldOriented = false;
float angle;
unsigned long lastGyroTime = 0;
#endif

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
            // channel.delta = delta;
            // apply deadzone
            if (delta >= 2000 - CHANNEL_DEADZONE) delta = 2000;
            if (delta <= 1000 + CHANNEL_DEADZONE) delta = 1000;
            if (abs(delta - 1500) <= CHANNEL_DEADZONE_CENTER) delta = 1500;
            // channel.deltaNormalized = delta;
            // convert 1000 <= delta <= 2000 to -1.0 <= value <= 1.0
            channel.value = (float)(delta - 1500) / 500.0f;
            channel.valueChanged = true;
        }
    }
}

void setup() {
    Serial.begin(9600);
    
    // Controller channel inputs
    pinMode(rightStickHorizontalPin, INPUT);
    pinMode(rightStickVerticalPin, INPUT);
    // pinMode(leftStickVerticalPin, INPUT);
    pinMode(leftStickHorizontalPin, INPUT);
    pinMode(switchAPin, INPUT);
    pinMode(switchDPin, INPUT);
#ifdef DO_FIELD_ORIENTED
    pinMode(switchBPin, INPUT);
#endif
    attachInterrupt(digitalPinToInterrupt(rightStickHorizontalPin), []{ updateChannel(channels.rightStickHorizontal); }, CHANGE);
    attachInterrupt(digitalPinToInterrupt(rightStickVerticalPin), []{ updateChannel(channels.rightStickVertical); }, CHANGE);
    // attachInterrupt(digitalPinToInterrupt(leftStickVerticalPin), []{ updateChannel(channels.leftStickVertical); }, CHANGE);
    attachInterrupt(digitalPinToInterrupt(leftStickHorizontalPin), []{ updateChannel(channels.leftStickHorizontal); }, CHANGE);
    attachInterrupt(digitalPinToInterrupt(switchAPin), []{ updateChannel(channels.switchA); }, CHANGE);
    attachInterrupt(digitalPinToInterrupt(switchDPin), []{ updateChannel(channels.switchD); }, CHANGE);
#ifdef DO_FIELD_ORIENTED
    attachInterrupt(digitalPinToInterrupt(switchBPin), []{ updateChannel(channels.switchB); }, CHANGE);
#endif
    
    pinMode(fanPin, OUTPUT);
    
#ifdef DO_FIELD_ORIENTED
    IMU.begin();
    lastGyroTime = millis();
#endif
    
    // Motors
    frontLeft.begin();
    frontRight.begin();
    backLeft.begin();
    backRight.begin();
}

void loop() {
    if (channels.switchA.value > 0.5f) {
        frontLeft.drive(0.0f);
        frontRight.drive(0.0f);
        backLeft.drive(0.0f);
        backRight.drive(0.0f);
        digitalWrite(fanPin, LOW);
        delay(2500);
        return;
    }
    
#ifdef DO_FIELD_ORIENTED
    if (IMU.gyroscopeAvailable()) {
        float x, y, z;
        IMU.readGyroscope(x, y, z);
        unsigned long now = millis();
        float delta = (now - lastGyroTime) / 1000.0f;
        angle += z * delta;
        lastGyroTime = now;
    }

    bool isFieldOriented = channels.switchB.value > 0.5f;
    
    if (isFieldOriented != wasFieldOriented) {
        if (isFieldOriented) {
            angle = 0.0f; // reset
        }
        wasFieldOriented = isFieldOriented;
    }
#endif
    
    vec2 stick = vec2(
        channels.rightStickHorizontal.value,
        channels.rightStickVertical.value
    );
    
#ifdef DO_FIELD_ORIENTED
    if (isFieldOriented) stick.rotate(angle);
#endif
    
    // Do some complicated math
    DriveValues values = drive.calculate(
        stick,
        // 0.0f
        channels.leftStickHorizontal.value * 0.25f
    );
    // Move motors
    frontLeft.drive(values.frontLeft);
    frontRight.drive(values.frontRight);
    backLeft.drive(values.backLeft);
    backRight.drive(values.backRight);
    
    // Fans
    digitalWrite(fanPin, channels.switchD.value > 0.5f ? HIGH : LOW);
    
    // Debugging assistance
    Serial.println(
        String("RH:") + channels.rightStickHorizontal.value
            + ", RV:" + channels.rightStickVertical.value
            + ", LH:" + channels.leftStickHorizontal.value
            + ", SA:" + channels.switchA.value
            + ", SD:" + channels.switchD.value
    );
}