#pragma once

#include <ESP32Servo.h>

class ServoController {
public:
    void init();
    void setPan(float roll);
    void setTilt(float pitch);
    void update();

    int getPanAngle() const { return (int)panCurrent_; }
    int getTiltAngle() const { return (int)tiltCurrent_; }

private:
    float mapAngle(float value, float inMin, float inMax, float outMin, float outMax) const;

    Servo panServo_;
    Servo tiltServo_;

    float panTarget_ = 90, panCurrent_ = 90;
    float tiltTarget_ = 90, tiltCurrent_ = 90;

    static constexpr float SMOOTH = 0.15f;
};
