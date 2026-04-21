#pragma once

#include <Arduino.h>

class IMUHandler {
public:
    bool init();
    void update(float dt);

    float getRoll() const { return roll_; }
    float getPitch() const { return pitch_; }

    float getAccelX() const { return ax_g_; }
    float getAccelY() const { return ay_g_; }
    float getAccelZ() const { return az_g_; }

    float getGyroX() const { return gx_dps_; }
    float getGyroY() const { return gy_dps_; }
    float getGyroZ() const { return gz_dps_; }

    bool isReady() const { return ready_; }

private:
    bool readRaw(int16_t &ax, int16_t &ay, int16_t &az,
                 int16_t &gx, int16_t &gy, int16_t &gz);

    bool ready_ = false;
    float roll_ = 0.0f;
    float pitch_ = 0.0f;

    float ax_g_ = 0, ay_g_ = 0, az_g_ = 0;
    float gx_dps_ = 0, gy_dps_ = 0, gz_dps_ = 0;

    float gxBias_ = 0, gyBias_ = 0, gzBias_ = 0;
};
