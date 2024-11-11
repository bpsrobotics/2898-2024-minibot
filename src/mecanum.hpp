#include "vec.hpp"

struct DriveValues {
    double frontLeft, frontRight, backLeft, backRight;
};

DriveValues operator + (DriveValues a, DriveValues b) {
    return {
        a.frontLeft + b.frontLeft,
        a.frontRight + b.frontRight,
        a.backLeft + b.backLeft,
        a.backRight + b.backRight
    };
}

DriveValues operator - (DriveValues a, DriveValues b) {
    return {
        a.frontLeft - b.frontLeft,
        a.frontRight - b.frontRight,
        a.backLeft - b.backLeft,
        a.backRight - b.backRight
    };
}

DriveValues operator * (DriveValues a, double b) {
    return {
        a.frontLeft * b,
        a.frontRight * b,
        a.backLeft * b,
        a.backRight * b
    };
}

DriveValues operator / (DriveValues a, double b) {
    return {
        a.frontLeft / b,
        a.frontRight / b,
        a.backLeft / b,
        a.backRight / b
    };
}

/* What happens when the requested movement requires motor thrust above maximum? */
enum OverflowBehavior {
    /* Scale down both movement types equally */
    OF_REDUCE_EQUALLY,
    /* Reduce lateral motion */
    OF_REDUCE_MOTION,
    /* Reduce rotation */
    OF_REDUCE_ROTATION,
    /* Don't reduce any movement (NOT RECOMMENDED) */
    OF_NO_REDUCE,
    /* Reduce movement by the same amount all the time (in half) */
    OF_REDUCE_ALWAYS
};

class MecanumDrive {
    public:
        MecanumDrive(double perpRatio, OverflowBehavior overflow) :
            perpRatio(perpRatio),
            invPerpRatio(1 / perpRatio),
            overflow(overflow) {}
        DriveValues calculate(vec2 motion, double rotation) {
            double _1 = 0.5 * motion.y, _2 = 0.5 * motion.x * invPerpRatio;
            double u = _1 + _2;
            double v = _1 - _2;
            DriveValues move = {
                0.5 * u,
                0.5 * v,
                0.5 * v,
                0.5 * u
            };
            DriveValues rotate = {
                rotation,
                -rotation,
                rotation,
                -rotation
            };
            DriveValues initial = move + rotate;
            if (overflow == OF_NO_REDUCE) return initial;
            if (overflow == OF_REDUCE_ALWAYS) return initial * 0.5;
            double high = max(max(abs(initial.frontLeft), abs(initial.frontRight)), max(abs(initial.backLeft), abs(initial.backRight)));
            if (high <= 1) return initial;
            if (overflow == OF_REDUCE_EQUALLY) {
                return initial / high;
            } else if (overflow == OF_REDUCE_MOTION) {
                double reduce = min(min((1 - abs(rotate.frontLeft)) / abs(move.frontLeft),
                                        (1 - abs(rotate.frontRight)) / abs(move.frontRight)),
                                    min((1 - abs(rotate.backLeft)) / abs(move.backLeft),
                                        (1 - abs(rotate.backRight)) / abs(move.backRight)));
                return rotate + move * reduce;
            } else if (overflow == OF_REDUCE_ROTATION) {
                double reduce = min(min((1 - abs(move.frontLeft)) / abs(rotate.frontLeft),
                                        (1 - abs(move.frontRight)) / abs(rotate.frontRight)),
                                    min((1 - abs(move.backLeft)) / abs(rotate.backLeft),
                                        (1 - abs(move.backRight)) / abs(rotate.backRight)));
                return move + rotate * reduce;
            }
            return initial; // impossible to get here, but just in case
        }
    private:
        OverflowBehavior overflow;
        double perpRatio, invPerpRatio;
};