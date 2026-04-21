#include "FeedbackManager.h"
#include "config.h"

void FeedbackManager::init() {
    ledcSetup(LED_CH_R, LED_PWM_FREQ, LED_PWM_RES);
    ledcSetup(LED_CH_G, LED_PWM_FREQ, LED_PWM_RES);
    ledcSetup(LED_CH_B, LED_PWM_FREQ, LED_PWM_RES);

    ledcAttachPin(LED_RED_PIN, LED_CH_R);
    ledcAttachPin(LED_GREEN_PIN, LED_CH_G);
    ledcAttachPin(LED_BLUE_PIN, LED_CH_B);

    pinMode(VIBRO_PIN, OUTPUT);
    digitalWrite(VIBRO_PIN, LOW);

    setColor(0, 0, 0);
}

void FeedbackManager::setColor(uint8_t r, uint8_t g, uint8_t b) {
    r_ = r; g_ = g; b_ = b;
    ledcWrite(LED_CH_R, r);
    ledcWrite(LED_CH_G, g);
    ledcWrite(LED_CH_B, b);
}

void FeedbackManager::setGestureColor(int gestureId) {
    switch (gestureId) {
        case GESTURE_NONE:        setColor(0, 0, 0);     break;
        case GESTURE_TILT_LEFT:
        case GESTURE_TILT_RIGHT:  setColor(0, 0, 255);   break;
        case GESTURE_TILT_UP:
        case GESTURE_TILT_DOWN:   setColor(0, 255, 255); break;
        case GESTURE_GRAB:        setColor(255, 0, 0);   break;
        case GESTURE_POINT:       setColor(0, 255, 0);   break;
        default:                  setColor(0, 0, 0);     break;
    }
}

void FeedbackManager::vibrateOnce() {
    digitalWrite(VIBRO_PIN, HIGH);
    vibroActive_ = true;
    vibroEndMs_ = millis() + VIBRO_PULSE_MS;
}

void FeedbackManager::onGestureChange(int gestureId) {
    if (gestureId != lastGesture_) {
        if (gestureId != GESTURE_NONE) {
            vibrateOnce();
        }
        lastGesture_ = gestureId;
    }
}

void FeedbackManager::update() {
    if (vibroActive_ && (long)(millis() - vibroEndMs_) >= 0) {
        digitalWrite(VIBRO_PIN, LOW);
        vibroActive_ = false;
    }
}
