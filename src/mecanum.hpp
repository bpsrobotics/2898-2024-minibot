#include "vec.hpp"

struct DriveValues {
    float frontLeft, frontRight, backLeft, backRight;
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

DriveValues operator * (DriveValues a, float b) {
    return {
        a.frontLeft * b,
        a.frontRight * b,
        a.backLeft * b,
        a.backRight * b
    };
}

DriveValues operator / (DriveValues a, float b) {
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

enum AsymmetricFixBehavior {
    AF_MIN_CIRCLE,
    AF_MIN_SQUARE,
    AF_FIT
};

const uint8_t TF_ROTATE_NONE = 0b00;
const uint8_t TF_ROTATE_90DEG = 0b01;
const uint8_t TF_ROTATE_180DEG = 0b10;
const uint8_t TF_ROTATE_270DEG = 0b11;

const uint8_t TF_FLIP_X = 0b100;
const uint8_t TF_FLIP_Y = 0b1000;

const uint8_t TF_INVERT_ROTATION = 0b10000;

float min(float a, float b) {
    if (a > b) return b;
    else return a;
}

float max(float a, float b) {
    if (a < b) return b;
    else return a;
}

class MecanumDrive {
    public:
        MecanumDrive(
            float perpRatio,
            OverflowBehavior overflow = OF_REDUCE_EQUALLY,
            AsymmetricFixBehavior asymm = AF_FIT,
            uint8_t transform = 0
        ) :
            perpRatio(perpRatio),
            invPerpRatio(1 / perpRatio),
            overflow(overflow),
            asymm(asymm),
            transform(transform)
        {
            if (asymm == AF_MIN_SQUARE) {
                multiplier = 4 * perpRatio / sqrtf(2 * perpRatio * perpRatio + 2);
            } else if (asymm == AF_MIN_CIRCLE) {
                multiplier = 4 * perpRatio / sqrtf(perpRatio * perpRatio + 1);
            }
        }
        DriveValues calculate(vec2 motion, float rotation) {
            if (transform & TF_FLIP_X) motion.x = -motion.x;
            if (transform & TF_FLIP_Y) motion.y = -motion.y;
            switch (transform & 0b11) {
                case TF_ROTATE_90DEG:
                    motion = vec2(-motion.y, motion.x);
                    break;
                case TF_ROTATE_180DEG:
                    motion = vec2(-motion.x, -motion.y);
                    break;
                case TF_ROTATE_270DEG:
                    motion = vec2(motion.y, -motion.x);
                    break;
            }
            if (transform & TF_INVERT_ROTATION) rotation = -rotation;
            if (asymm == AF_MIN_SQUARE) {
                motion *= multiplier;
            } else if (asymm == AF_MIN_CIRCLE) {
                motion *= multiplier;
                float mm = motion.magnitude();
                if (mm > multiplier) {
                    motion *= multiplier / mm;
                }
            } else {
                motion.y *= 4;
                motion.x *= 4 * perpRatio;
            }
            float _1 = 0.5f * motion.y, _2 = 0.5f * motion.x * invPerpRatio;
            float u = _1 + _2;
            float v = _1 - _2;
            DriveValues move = {
                0.5f * u,
                0.5f * v,
                0.5f * v,
                0.5f * u
            };
            DriveValues rotate =
                (transform & TF_ROTATE_90DEG) ? DriveValues{
                    rotation,
                    rotation,
                    -rotation,
                    -rotation
                } : (transform & TF_ROTATE_180DEG) ? DriveValues{
                    rotation,
                    -rotation,
                    rotation,
                    -rotation
                } : (transform & TF_ROTATE_270DEG) ? DriveValues{
                    -rotation,
                    -rotation,
                    rotation,
                    rotation
                } : DriveValues{
                    -rotation,
                    rotation,
                    -rotation,
                    rotation
                };
            DriveValues initial = move + rotate;
            if (overflow == OF_NO_REDUCE) return initial;
            if (overflow == OF_REDUCE_ALWAYS) return initial * 0.5;
            float high = max(max(fabs(initial.frontLeft), fabs(initial.frontRight)), max(fabs(initial.backLeft), fabs(initial.backRight)));
            if (high <= 1) return initial;
            if (overflow == OF_REDUCE_EQUALLY) {
                return initial / high;
            } else if (overflow == OF_REDUCE_MOTION) {
                float reduce = min(min((1 - fabs(rotate.frontLeft)) / fabs(move.frontLeft),
                                        (1 - fabs(rotate.frontRight)) / fabs(move.frontRight)),
                                    min((1 - fabs(rotate.backLeft)) / fabs(move.backLeft),
                                        (1 - fabs(rotate.backRight)) / fabs(move.backRight)));
                return rotate + move * reduce;
            } else if (overflow == OF_REDUCE_ROTATION) {
                float reduce = min(min((1 - fabs(move.frontLeft)) / fabs(rotate.frontLeft),
                                        (1 - fabs(move.frontRight)) / fabs(rotate.frontRight)),
                                    min((1 - fabs(move.backLeft)) / fabs(rotate.backLeft),
                                        (1 - fabs(move.backRight)) / fabs(rotate.backRight)));
                return move + rotate * reduce;
            }
            return initial; // impossible to get here, but just in case
        }
        OverflowBehavior overflow;
        AsymmetricFixBehavior asymm;
        float perpRatio, invPerpRatio, multiplier;
        uint8_t transform;
    private:
};