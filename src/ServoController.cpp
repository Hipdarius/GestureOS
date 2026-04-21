#include "ServoController.h"
#include "config.h"

void ServoController::init() {
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    panServo_.setPeriodHertz(50);
    tiltServo_.setPeriodHertz(50);
    panServo_.attach(SERVO_PAN_PIN, 500, 2400);
    tiltServo_.attach(SERVO_TILT_PIN, 500, 2400);
    panServo_.write(90);
    tiltServo_.write(90);
}

float ServoController::mapAngle(float v, float inMin, float inMax, float outMin, float outMax) const {
    if (v < inMin) v = inMin;
    if (v > inMax) v = inMax;
    return outMin + (v - inMin) * (outMax - outMin) / (inMax - inMin);
}

void ServoController::setPan(float roll) {
    panTarget_ = mapAngle(roll, -90.0f, 90.0f, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
}

void ServoController::setTilt(float pitch) {
    tiltTarget_ = mapAngle(pitch, -90.0f, 90.0f, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
}

void ServoController::update() {
    panCurrent_  += (panTarget_  - panCurrent_)  * SMOOTH;
    tiltCurrent_ += (tiltTarget_ - tiltCurrent_) * SMOOTH;

    if (panCurrent_ < SERVO_MIN_ANGLE) panCurrent_ = SERVO_MIN_ANGLE;
    if (panCurrent_ > SERVO_MAX_ANGLE) panCurrent_ = SERVO_MAX_ANGLE;
    if (tiltCurrent_ < SERVO_MIN_ANGLE) tiltCurrent_ = SERVO_MIN_ANGLE;
    if (tiltCurrent_ > SERVO_MAX_ANGLE) tiltCurrent_ = SERVO_MAX_ANGLE;

    panServo_.write((int)panCurrent_);
    tiltServo_.write((int)tiltCurrent_);
}
