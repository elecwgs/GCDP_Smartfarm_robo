// Stepper.h - 간단한 커스텀 스텝모터 라이브러리 (4핀/2핀 지원)
#ifndef SIMPLE_STEPPER_H
#define SIMPLE_STEPPER_H

#include <Arduino.h>

class SimpleStepper {
public:
    // 생성자(4핀: pin1~pin4) or (2핀: step/direction)
    SimpleStepper(int stepsPerRev, int pin1, int pin2, int pin3 = -1, int pin4 = -1)
        : stepsPerRevolution(stepsPerRev),
          stepPins{pin1, pin2, pin3, pin4},
          use4Wire(pin3 != -1 && pin4 != -1),
          currentStep(0),
          dir(1),
          stepDelayMs(10) {}

    void begin() {
        if (use4Wire) {
            for (int i = 0; i < 4; i++) pinMode(stepPins[i], OUTPUT);
        } else {
            pinMode(stepPins[0], OUTPUT); // STEP
            pinMode(stepPins[1], OUTPUT); // DIR
        }
    }
    void setSpeed(int rpm) {
        // 스텝모터 속도 설정(RPM → delay)
        stepDelayMs = 60000L / (stepsPerRevolution * rpm);
    }
    void step(int steps) {
        if (use4Wire) {
            int absSteps = abs(steps);
            int direction = (steps > 0) ? 1 : -1;
            for (int i = 0; i < absSteps; i++) {
                currentStep += direction;
                if (currentStep >= 4) currentStep = 0;
                if (currentStep < 0) currentStep = 3;
                step4Wire(currentStep);
                delay(stepDelayMs);
            }
        } else {
            int absSteps = abs(steps);
            digitalWrite(stepPins[1], steps > 0 ? HIGH : LOW); // DIR
            for (int i = 0; i < absSteps; i++) {
                digitalWrite(stepPins[0], HIGH); // STEP
                delayMicroseconds(500);
                digitalWrite(stepPins[0], LOW);
                delay(stepDelayMs);
            }
        }
    }
private:
    int stepsPerRevolution;
    int stepPins[4]; // 0:STEP 1:DIR or 0~3:coil
    bool use4Wire;
    int currentStep;
    int dir;
    int stepDelayMs;

    // 4선 방식 스텝모터 파형 제어
    void step4Wire(int stepNum) {
        const int seq[4][4] = {
            {1,0,1,0},
            {0,1,1,0},
            {0,1,0,1},
            {1,0,0,1}
        };
        for (int i = 0; i < 4; i++) {
            digitalWrite(stepPins[i], seq[stepNum][i]);
        }
    }
};

#endif // SIMPLE_STEPPER_H