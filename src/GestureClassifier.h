#pragma once

#include <stdint.h>

class GestureClassifier {
public:
    int classify(float roll, float pitch, float flex1, float flex2);

    int getCurrent() const { return current_; }
    const char* getCurrentName() const { return name(current_); }

    static const char* name(int gestureId);

private:
    int rawClassify(float roll, float pitch, float flex1, float flex2) const;
    bool isFingersRelaxed(float flex1, float flex2) const;

    int current_ = 0;        // confirmed gesture
    int candidate_ = 0;      // pending gesture
    int stableCount_ = 0;    // consecutive readings matching candidate
};
