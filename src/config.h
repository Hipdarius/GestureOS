#pragma once

// ===== WiFi Access Point =====
#define WIFI_SSID "GestureOS"
#define WIFI_PASSWORD "gesture123"

// ===== I2C (MPU-6050) =====
#define SDA_PIN 21
#define SCL_PIN 22
#define MPU_ADDR 0x68

// ===== Flex Sensors =====
#define FLEX_PIN_1 34
#define FLEX_PIN_2 35
#define FLEX_RAW_MIN 500
#define FLEX_RAW_MAX 3500

// ===== Servos =====
#define SERVO_PAN_PIN 18
#define SERVO_TILT_PIN 19
#define SERVO_MIN_ANGLE 0
#define SERVO_MAX_ANGLE 180

// ===== Vibration Motor =====
#define VIBRO_PIN 16
#define VIBRO_PULSE_MS 100

// ===== RGB LED =====
#define LED_RED_PIN 25
#define LED_GREEN_PIN 26
#define LED_BLUE_PIN 27
#define LED_PWM_FREQ 5000
#define LED_PWM_RES 8
#define LED_CH_R 0
#define LED_CH_G 1
#define LED_CH_B 2

// ===== Timing =====
#define LOOP_INTERVAL_MS 10
#define WEBSOCKET_PORT 81
#define HTTP_PORT 80
#define TELEMETRY_HZ 50

// ===== Complementary Filter =====
#define COMP_FILTER_ALPHA 0.96f

// ===== Gesture IDs =====
#define GESTURE_NONE        0
#define GESTURE_TILT_LEFT   1
#define GESTURE_TILT_RIGHT  2
#define GESTURE_TILT_UP     3
#define GESTURE_TILT_DOWN   4
#define GESTURE_GRAB        5
#define GESTURE_POINT       6

// ===== Gesture Thresholds =====
#define GESTURE_TILT_LEFT_ROLL    -30.0f
#define GESTURE_TILT_RIGHT_ROLL    30.0f
#define GESTURE_TILT_UP_PITCH     -30.0f
#define GESTURE_TILT_DOWN_PITCH    30.0f
#define GESTURE_GRAB_FLEX_THRESH   70.0f
#define GESTURE_POINT_FLEX1_MAX    30.0f
#define GESTURE_POINT_FLEX2_MIN    60.0f
#define GESTURE_NEUTRAL_ROLL_MAX   15.0f
#define GESTURE_NEUTRAL_PITCH_MAX  15.0f

// Hysteresis: relaxed thresholds for exit
#define GESTURE_HYSTERESIS 10.0f

// Debouncing: required consecutive readings to confirm
#define GESTURE_DEBOUNCE_COUNT 2

// ===== Debug =====
// #define DEBUG_SERIAL
