#include <math.h>

class vec2 {
    public:
        vec2(double x, double y) : x(x), y(y) {};
        vec2(const vec2& v) : x(v.x), y(v.y) {}
        const double x, y;
        vec2 operator + (const vec2& other) {
            return vec2(x + other.x, y + other.y);
        }
        vec2 operator - (const vec2& other) {
            return vec2(x - other.x, y - other.y);
        }
        vec2 operator * (const double& other) {
            return vec2(x * other, y * other);
        }
        vec2 operator / (const double& other) {
            return vec2(x / other, y / other);
        }
        double magnitude() {
            return sqrt(this->magnitudeSquared());
        }
        double magnitudeSquared() {
            return x * x + y * y;
        }
        vec2 normalize() {
            return this->operator/(magnitude());
        }
};