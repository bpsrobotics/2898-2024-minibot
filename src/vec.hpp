#include <math.h>

class vec2 {
    public:
        vec2(float x, float y) : x(x), y(y) {};
        vec2(const vec2& v) : x(v.x), y(v.y) {}
        float x, y;
        vec2 clone() {
            return vec2(x, y);
        }
        vec2 operator + (const vec2& other) {
            return vec2(x + other.x, y + other.y);
        }
        vec2 operator - (const vec2& other) {
            return vec2(x - other.x, y - other.y);
        }
        vec2 operator * (const float& other) {
            return vec2(x * other, y * other);
        }
        vec2 operator / (const float& other) {
            return vec2(x / other, y / other);
        }
        /* Componentwise multiplication, aka Hadamard product. */
        vec2 operator * (const vec2& other) {
            return vec2(x * other.x, y * other.y);
        }
        /* Componentwise division. */
        vec2 operator / (const vec2& other) {
            return vec2(x / other.x, y / other.y);
        }
        vec2 operator += (const vec2& other) {
            return assign(x + other.x, y + other.y);
        }
        vec2 operator -= (const vec2& other) {
            return assign(x - other.x, y - other.y);
        }
        vec2 operator *= (const float& other) {
            return assign(x * other, y * other);
        }
        vec2 operator /= (const float& other) {
            return assign(x / other, y / other);
        }
        /* Componentwise multiplication, aka Hadamard product. */
        vec2 operator *= (const vec2& other) {
            return assign(x * other.x, y * other.y);
        }
        /* Componentwise division. */
        vec2 operator /= (const vec2& other) {
            return assign(x / other.x, y / other.y);
        }
        vec2 operator = (const vec2& other) {
            return assign(other.x, other.y);
        }
        float magnitude() {
            return sqrtf(this->magnitudeSquared());
        }
        float magnitudeSquared() {
            return x * x + y * y;
        }
        vec2 normalize() {
            return this->operator/(magnitude());
        }
        vec2 rotate(float angle) {
            float sina = sinf(angle);
            float cosa = cosf(angle);
            float x0 = x, y0 = y;
            x = x0 * cosa - y0 * sina;
            x = x0 * sina + y0 * cosa;
            return *this;
        }
        vec2 rotated(float angle) {
            return clone().rotate(angle);
        }
    private:
        vec2 assign(float x, float y) {
            this->x = x;
            this->y = y;
            return *this;
        }
};