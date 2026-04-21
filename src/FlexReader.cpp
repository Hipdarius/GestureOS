#include "FlexReader.h"
#include "config.h"

void FlexReader::init() {
    analogReadResolution(12);
    analogSetPinAttenuation(FLEX_PIN_1, ADC_11db);
    analogSetPinAttenuation(FLEX_PIN_2, ADC_11db);
    min1_ = FLEX_RAW_MIN;
    max1_ = FLEX_RAW_MAX;
    min2_ = FLEX_RAW_MIN;
    max2_ = FLEX_RAW_MAX;
}

void FlexReader::setCalibration(int rawMin1, int rawMax1, int rawMin2, int rawMax2) {
    min1_ = rawMin1; max1_ = rawMax1;
    min2_ = rawMin2; max2_ = rawMax2;
}

float FlexReader::normalize(int raw, int rawMin, int rawMax) const {
    if (rawMax == rawMin) return 0.0f;
    float pct = 100.0f * (float)(raw - rawMin) / (float)(rawMax - rawMin);
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    return pct;
}

void FlexReader::update() {
    raw1_ = analogRead(FLEX_PIN_1);
    raw2_ = analogRead(FLEX_PIN_2);

    float f1 = normalize(raw1_, min1_, max1_);
    float f2 = normalize(raw2_, min2_, max2_);

    flex1_ = ALPHA * f1 + (1.0f - ALPHA) * flex1_;
    flex2_ = ALPHA * f2 + (1.0f - ALPHA) * flex2_;
}
