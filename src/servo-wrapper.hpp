#include <Servo.h>

class ServoWrapper {
    public:
        ServoWrapper(int pin) : pin(pin) {
            servo = new Servo();
        }
        void begin() {
            servo->attach(pin);
        }
        void drive(float value) {
            if (value > 1.0) value = 1.0;
            if (value < -1.0) value = -1.0;
            // if (value != lastValue) {
                // if (lastValue == 0) servo.attach(pin);
                servo->writeMicroseconds(1500 + 500 * value);
                // if (value == 0) servo.detach();
                lastValue = value;
            // }
        }
    private:
        Servo* servo;
        float lastValue;
        int pin;
};