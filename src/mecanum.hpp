#include "vec.hpp"

struct DriveValues {
    double frontLeft, frontRight, backLeft, backRight;
};

class MecanumDrive {
    public:
        MecanumDrive(double perpRatio) :
            perpRatio(perpRatio),
            invPerpRatio(1 / perpRatio) {}
        DriveValues calculate(vec2 motion, double rotation) {
            double _1 = 0.5 * motion.y, _2 = 0.5 * motion.x * invPerpRatio;
            double u = _1 + _2;
            double v = _1 - _2;
            rotation *= 0.5;
            return {
                (0.5 + rotation) * u,
                (0.5 - rotation) * v,
                (0.5 + rotation) * v,
                (0.5 - rotation) * u
            };
        }
    private:
        double perpRatio, invPerpRatio;
};