#pragma once

#include <Arduino.h>

class FlexReader {
public:
    void init();
    void update();

    float getFlex1() const { return flex1_; }
    float getFlex2() const { return flex2_; }

    int getRaw1() const { return raw1_; }
    int getRaw2() const { return raw2_; }

    void setCalibration(int rawMin1, int rawMax1, int rawMin2, int rawMax2);

private:
    float normalize(int raw, int rawMin, int rawMax) const;

    int raw1_ = 0, raw2_ = 0;
    float flex1_ = 0, flex2_ = 0;

    int min1_, max1_, min2_, max2_;

    // Exponential smoothing
    static constexpr float ALPHA = 0.3f;
};
