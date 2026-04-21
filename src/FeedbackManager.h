#pragma once

#include <Arduino.h>

class FeedbackManager {
public:
    void init();
    void update();

    void setColor(uint8_t r, uint8_t g, uint8_t b);
    void setGestureColor(int gestureId);
    void vibrateOnce();

    void onGestureChange(int gestureId);

private:
    uint8_t r_ = 0, g_ = 0, b_ = 0;

    bool vibroActive_ = false;
    unsigned long vibroEndMs_ = 0;

    int lastGesture_ = -1;
};
