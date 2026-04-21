#include <Arduino.h>
#include <Wire.h>
#include <ArduinoJson.h>

#include "config.h"
#include "IMUHandler.h"
#include "FlexReader.h"
#include "GestureClassifier.h"
#include "ServoController.h"
#include "FeedbackManager.h"
#include "WebManager.h"

static IMUHandler          imu;
static FlexReader          flex;
static GestureClassifier   classifier;
static ServoController     servos;
static FeedbackManager     feedback;
static WebManager          web;

static unsigned long lastLoopMicros = 0;
static unsigned long lastTelemetryMs = 0;
static unsigned long lastDebugMs = 0;

static void errorBlink(uint8_t r, uint8_t g, uint8_t b, const char *msg) {
    Serial.println(msg);
    for (;;) {
        feedback.setColor(r, g, b);
        delay(250);
        feedback.setColor(0, 0, 0);
        delay(250);
    }
}

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("\n=== GestureOS booting ===");

    feedback.init();
    feedback.setColor(32, 0, 32); // purple = boot

    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(400000);

    if (!imu.init()) {
        errorBlink(255, 0, 0, "[FATAL] MPU-6050 not responding on I2C");
    }
    Serial.println("[OK] MPU-6050");

    flex.init();
    Serial.println("[OK] Flex sensors");

    servos.init();
    Serial.println("[OK] Servos");

    if (!web.init()) {
        errorBlink(255, 128, 0, "[FATAL] WebManager init failed");
    }
    Serial.printf("Dashboard: http://%s\n", web.getIP().toString().c_str());

    feedback.setColor(0, 0, 0);
    lastLoopMicros = micros();
}

void loop() {
    unsigned long now = micros();
    float dt = (now - lastLoopMicros) / 1'000'000.0f;
    if (dt <= 0 || dt > 0.2f) dt = LOOP_INTERVAL_MS / 1000.0f;
    lastLoopMicros = now;

    web.loop();

    imu.update(dt);
    flex.update();

    float roll  = imu.getRoll();
    float pitch = imu.getPitch();
    float f1    = flex.getFlex1();
    float f2    = flex.getFlex2();

    int gesture = classifier.classify(roll, pitch, f1, f2);

    if (gesture != GESTURE_NONE) {
        feedback.setGestureColor(gesture);
        servos.setPan(roll);
        servos.setTilt(pitch);
    } else {
        feedback.setColor(0, 0, 0);
    }
    feedback.onGestureChange(gesture);
    feedback.update();
    servos.update();

    unsigned long ms = millis();
    if (ms - lastTelemetryMs >= (1000 / TELEMETRY_HZ)) {
        lastTelemetryMs = ms;

        StaticJsonDocument<512> doc;
        doc["roll"]      = roll;
        doc["pitch"]     = pitch;
        doc["flex1"]     = f1;
        doc["flex2"]     = f2;
        doc["gesture"]   = GestureClassifier::name(gesture);
        doc["gestureId"] = gesture;
        doc["accelX"]    = imu.getAccelX();
        doc["accelY"]    = imu.getAccelY();
        doc["accelZ"]    = imu.getAccelZ();
        doc["gyroX"]     = imu.getGyroX();
        doc["gyroY"]     = imu.getGyroY();
        doc["gyroZ"]     = imu.getGyroZ();
        doc["servoPan"]  = servos.getPanAngle();
        doc["servoTilt"] = servos.getTiltAngle();
        doc["timestamp"] = ms;

        String out;
        serializeJson(doc, out);
        web.broadcast(out);
    }

#ifdef DEBUG_SERIAL
    if (ms - lastDebugMs >= 100) {
        lastDebugMs = ms;
        Serial.printf("R:%.1f P:%.1f F1:%.0f F2:%.0f G:%s\n",
                      roll, pitch, f1, f2, GestureClassifier::name(gesture));
    }
#endif

    // Hold loop period
    unsigned long elapsed = micros() - now;
    long wait = (long)(LOOP_INTERVAL_MS * 1000) - (long)elapsed;
    if (wait > 1000) delayMicroseconds(wait);
}
