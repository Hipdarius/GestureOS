#include <unity.h>

#include "../src/config.h"
#include "../src/GestureClassifier.h"
#include "../src/GestureClassifier.cpp"

// Helper: run classify() enough times to pass the debounce window.
static int settle(GestureClassifier &c, float r, float p, float f1, float f2) {
    int last = 0;
    for (int i = 0; i < GESTURE_DEBOUNCE_COUNT + 1; ++i) {
        last = c.classify(r, p, f1, f2);
    }
    return last;
}

void test_none_neutral() {
    GestureClassifier c;
    TEST_ASSERT_EQUAL_INT(GESTURE_NONE, settle(c, 0, 0, 10, 10));
}

void test_tilt_left() {
    GestureClassifier c;
    TEST_ASSERT_EQUAL_INT(GESTURE_TILT_LEFT, settle(c, -45, 0, 10, 10));
}

void test_tilt_right() {
    GestureClassifier c;
    TEST_ASSERT_EQUAL_INT(GESTURE_TILT_RIGHT, settle(c, 45, 0, 10, 10));
}

void test_tilt_up() {
    GestureClassifier c;
    TEST_ASSERT_EQUAL_INT(GESTURE_TILT_UP, settle(c, 0, -45, 10, 10));
}

void test_tilt_down() {
    GestureClassifier c;
    TEST_ASSERT_EQUAL_INT(GESTURE_TILT_DOWN, settle(c, 0, 45, 10, 10));
}

void test_grab() {
    GestureClassifier c;
    TEST_ASSERT_EQUAL_INT(GESTURE_GRAB, settle(c, 0, 0, 85, 85));
}

void test_point() {
    GestureClassifier c;
    TEST_ASSERT_EQUAL_INT(GESTURE_POINT, settle(c, 0, 0, 15, 80));
}

void test_debounce_requires_persistence() {
    GestureClassifier c;
    // Single reading should not flip state (if debounce count > 1)
    int g = c.classify(-45, 0, 10, 10);
    if (GESTURE_DEBOUNCE_COUNT > 1) {
        TEST_ASSERT_EQUAL_INT(GESTURE_NONE, g);
    }
    // Still NONE; now confirm
    for (int i = 0; i < GESTURE_DEBOUNCE_COUNT; ++i) {
        g = c.classify(-45, 0, 10, 10);
    }
    TEST_ASSERT_EQUAL_INT(GESTURE_TILT_LEFT, g);
}

void test_grab_priority_over_tilt() {
    GestureClassifier c;
    // Strong tilt AND fist — GRAB wins
    TEST_ASSERT_EQUAL_INT(GESTURE_GRAB, settle(c, -60, 0, 85, 85));
}

void test_threshold_boundary_not_left() {
    GestureClassifier c;
    // Exactly at -30 should NOT trigger TILT_LEFT (condition is strict <)
    TEST_ASSERT_NOT_EQUAL(GESTURE_TILT_LEFT, settle(c, -30, 0, 10, 10));
}

void test_transition_between_gestures() {
    GestureClassifier c;
    TEST_ASSERT_EQUAL_INT(GESTURE_TILT_LEFT,  settle(c, -45, 0, 10, 10));
    TEST_ASSERT_EQUAL_INT(GESTURE_TILT_RIGHT, settle(c,  45, 0, 10, 10));
    TEST_ASSERT_EQUAL_INT(GESTURE_GRAB,       settle(c,   0, 0, 85, 85));
    TEST_ASSERT_EQUAL_INT(GESTURE_NONE,       settle(c,   0, 0, 10, 10));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_none_neutral);
    RUN_TEST(test_tilt_left);
    RUN_TEST(test_tilt_right);
    RUN_TEST(test_tilt_up);
    RUN_TEST(test_tilt_down);
    RUN_TEST(test_grab);
    RUN_TEST(test_point);
    RUN_TEST(test_debounce_requires_persistence);
    RUN_TEST(test_grab_priority_over_tilt);
    RUN_TEST(test_threshold_boundary_not_left);
    RUN_TEST(test_transition_between_gestures);
    return UNITY_END();
}

void setUp() {}
void tearDown() {}
