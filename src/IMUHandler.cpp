#include "IMUHandler.h"
#include "config.h"
#include <Wire.h>
#include <math.h>

static constexpr float ACCEL_LSB_PER_G   = 16384.0f;
static constexpr float GYRO_LSB_PER_DPS  = 131.0f;
static constexpr float RAD_TO_DEGf       = 57.2957795f;

bool IMUHandler::init() {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x6B);
    Wire.write(0x00);
    if (Wire.endTransmission(true) != 0) {
        ready_ = false;
        return false;
    }

    // Gyro ±250°/s
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x1B);
    Wire.write(0x00);
    Wire.endTransmission(true);

    // Accel ±2g
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x1C);
    Wire.write(0x00);
    Wire.endTransmission(true);

    // DLPF 44 Hz accel / 42 Hz gyro for smoother readings
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x1A);
    Wire.write(0x03);
    Wire.endTransmission(true);

    delay(50);

    // Gyro bias calibration (assume glove stationary)
    const int samples = 200;
    long sgx = 0, sgy = 0, sgz = 0;
    int16_t ax, ay, az, gx, gy, gz;
    int taken = 0;
    for (int i = 0; i < samples; ++i) {
        if (readRaw(ax, ay, az, gx, gy, gz)) {
            sgx += gx; sgy += gy; sgz += gz;
            ++taken;
        }
        delay(3);
    }
    if (taken > 0) {
        gxBias_ = (float)sgx / taken;
        gyBias_ = (float)sgy / taken;
        gzBias_ = (float)sgz / taken;
    }

    // Seed Roll/Pitch from accelerometer
    if (readRaw(ax, ay, az, gx, gy, gz)) {
        float axg = ax / ACCEL_LSB_PER_G;
        float ayg = ay / ACCEL_LSB_PER_G;
        float azg = az / ACCEL_LSB_PER_G;
        roll_  = atan2f(ayg, azg) * RAD_TO_DEGf;
        pitch_ = atan2f(-axg, sqrtf(ayg * ayg + azg * azg)) * RAD_TO_DEGf;
    }

    ready_ = true;
    return true;
}

bool IMUHandler::readRaw(int16_t &ax, int16_t &ay, int16_t &az,
                         int16_t &gx, int16_t &gy, int16_t &gz) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B);
    if (Wire.endTransmission(false) != 0) return false;

    if (Wire.requestFrom((uint8_t)MPU_ADDR, (uint8_t)14, (uint8_t)true) != 14) {
        return false;
    }

    ax = (Wire.read() << 8) | Wire.read();
    ay = (Wire.read() << 8) | Wire.read();
    az = (Wire.read() << 8) | Wire.read();
    (void)((Wire.read() << 8) | Wire.read()); // temperature, ignore
    gx = (Wire.read() << 8) | Wire.read();
    gy = (Wire.read() << 8) | Wire.read();
    gz = (Wire.read() << 8) | Wire.read();
    return true;
}

void IMUHandler::update(float dt) {
    if (!ready_) return;

    int16_t ax, ay, az, gx, gy, gz;
    if (!readRaw(ax, ay, az, gx, gy, gz)) return;

    ax_g_ = ax / ACCEL_LSB_PER_G;
    ay_g_ = ay / ACCEL_LSB_PER_G;
    az_g_ = az / ACCEL_LSB_PER_G;

    gx_dps_ = (gx - gxBias_) / GYRO_LSB_PER_DPS;
    gy_dps_ = (gy - gyBias_) / GYRO_LSB_PER_DPS;
    gz_dps_ = (gz - gzBias_) / GYRO_LSB_PER_DPS;

    float accelRoll  = atan2f(ay_g_, az_g_) * RAD_TO_DEGf;
    float accelPitch = atan2f(-ax_g_, sqrtf(ay_g_ * ay_g_ + az_g_ * az_g_)) * RAD_TO_DEGf;

    const float a = COMP_FILTER_ALPHA;
    roll_  = a * (roll_  + gx_dps_ * dt) + (1.0f - a) * accelRoll;
    pitch_ = a * (pitch_ + gy_dps_ * dt) + (1.0f - a) * accelPitch;
}
