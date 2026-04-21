#include "GestureClassifier.h"
#include "config.h"
#include <math.h>

bool GestureClassifier::isFingersRelaxed(float flex1, float flex2) const {
    return flex1 < GESTURE_GRAB_FLEX_THRESH && flex2 < GESTURE_POINT_FLEX2_MIN;
}

int GestureClassifier::rawClassify(float roll, float pitch, float flex1, float flex2) const {
    // Grab (both fingers bent) takes priority — unambiguous fist
    if (flex1 > GESTURE_GRAB_FLEX_THRESH && flex2 > GESTURE_GRAB_FLEX_THRESH) {
        return GESTURE_GRAB;
    }

    // Point (index extended, other bent)
    if (flex1 < GESTURE_POINT_FLEX1_MAX && flex2 > GESTURE_POINT_FLEX2_MIN) {
        return GESTURE_POINT;
    }

    // Tilts require fingers relaxed
    if (isFingersRelaxed(flex1, flex2)) {
        // Pick the dominant axis
        if (fabsf(roll) > fabsf(pitch)) {
            if (roll < GESTURE_TILT_LEFT_ROLL)  return GESTURE_TILT_LEFT;
            if (roll > GESTURE_TILT_RIGHT_ROLL) return GESTURE_TILT_RIGHT;
        } else {
            if (pitch < GESTURE_TILT_UP_PITCH)   return GESTURE_TILT_UP;
            if (pitch > GESTURE_TILT_DOWN_PITCH) return GESTURE_TILT_DOWN;
        }
    }

    // Neutral zone → NONE
    if (fabsf(roll)  < GESTURE_NEUTRAL_ROLL_MAX  &&
        fabsf(pitch) < GESTURE_NEUTRAL_PITCH_MAX &&
        isFingersRelaxed(flex1, flex2)) {
        return GESTURE_NONE;
    }

    return GESTURE_NONE;
}

int GestureClassifier::classify(float roll, float pitch, float flex1, float flex2) {
    // Hysteresis: if a tilt is active, relax the exit threshold.
    float rEff = roll, pEff = pitch;
    switch (current_) {
        case GESTURE_TILT_LEFT:
            if (roll < GESTURE_TILT_LEFT_ROLL + GESTURE_HYSTERESIS) rEff = GESTURE_TILT_LEFT_ROLL - 1;
            break;
        case GESTURE_TILT_RIGHT:
            if (roll > GESTURE_TILT_RIGHT_ROLL - GESTURE_HYSTERESIS) rEff = GESTURE_TILT_RIGHT_ROLL + 1;
            break;
        case GESTURE_TILT_UP:
            if (pitch < GESTURE_TILT_UP_PITCH + GESTURE_HYSTERESIS) pEff = GESTURE_TILT_UP_PITCH - 1;
            break;
        case GESTURE_TILT_DOWN:
            if (pitch > GESTURE_TILT_DOWN_PITCH - GESTURE_HYSTERESIS) pEff = GESTURE_TILT_DOWN_PITCH + 1;
            break;
        default: break;
    }

    int detected = rawClassify(rEff, pEff, flex1, flex2);

    if (detected == candidate_) {
        if (stableCount_ < 100) stableCount_++;
    } else {
        candidate_ = detected;
        stableCount_ = 1;
    }

    if (stableCount_ >= GESTURE_DEBOUNCE_COUNT) {
        current_ = candidate_;
    }

    return current_;
}

const char* GestureClassifier::name(int gestureId) {
    switch (gestureId) {
        case GESTURE_NONE:       return "NONE";
        case GESTURE_TILT_LEFT:  return "TILT_LEFT";
        case GESTURE_TILT_RIGHT: return "TILT_RIGHT";
        case GESTURE_TILT_UP:    return "TILT_UP";
        case GESTURE_TILT_DOWN:  return "TILT_DOWN";
        case GESTURE_GRAB:       return "GRAB";
        case GESTURE_POINT:      return "POINT";
        default:                 return "UNKNOWN";
    }
}
