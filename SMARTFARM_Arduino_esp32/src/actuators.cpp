#include "actuators.h"
#include "config.h"

ActuatorManager::ActuatorManager() 
    : stepper(200, STEP_PIN, DIR_PIN), pumpStatus(false), fanStatus(false), 
      ledStatus(false), wateringActive(false), wateringStartTime(0) {
}

void ActuatorManager::init() {
    // 핀 모드 설정
    pinMode(PUMP1_PIN, OUTPUT);
    pinMode(PUMP2_PIN, OUTPUT);
    pinMode(FAN1_PIN, OUTPUT);
    pinMode(FAN2_PIN, OUTPUT);
    pinMode(FAN3_PIN, OUTPUT);
    pinMode(FAN4_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(ENABLE_PIN, OUTPUT);
    
    // 초기 상태 설정 (모든 장치 OFF)
    digitalWrite(PUMP1_PIN, LOW);
    digitalWrite(PUMP2_PIN, LOW);
    digitalWrite(FAN1_PIN, LOW);
    digitalWrite(FAN2_PIN, LOW);
    digitalWrite(FAN3_PIN, LOW);
    digitalWrite(FAN4_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(ENABLE_PIN, HIGH); // 스텝모터 활성화
    
    // 스텝모터 설정
    stepper.setSpeed(60); // RPM
    
    Serial.println("✅ 액추에이터 매니저 초기화 완료");
}

void ActuatorManager::activatePumps(bool activate) {
    if (activate) {
        digitalWrite(PUMP1_PIN, HIGH);
        digitalWrite(PUMP2_PIN, HIGH);
        pumpStatus = true;
        Serial.println("💧 워터펌프 ON");
    } else {
        digitalWrite(PUMP1_PIN, LOW);
        digitalWrite(PUMP2_PIN, LOW);
        pumpStatus = false;
        Serial.println("💧 워터펌프 OFF");
    }
}

void ActuatorManager::activateFans(bool activate) {
    if (activate) {
        digitalWrite(FAN1_PIN, HIGH);
        digitalWrite(FAN2_PIN, HIGH);
        digitalWrite(FAN3_PIN, HIGH);
        digitalWrite(FAN4_PIN, HIGH);
        fanStatus = true;
        Serial.println("🌀 팬 ON");
    } else {
        digitalWrite(FAN1_PIN, LOW);
        digitalWrite(FAN2_PIN, LOW);
        digitalWrite(FAN3_PIN, LOW);
        digitalWrite(FAN4_PIN, LOW);
        fanStatus = false;
        Serial.println("🌀 팬 OFF");
    }
}

void ActuatorManager::activateLED(bool activate) {
    if (activate) {
        digitalWrite(LED_PIN, HIGH);
        ledStatus = true;
        Serial.println("💡 LED 조명 ON");
    } else {
        digitalWrite(LED_PIN, LOW);
        ledStatus = false;
        Serial.println("💡 LED 조명 OFF");
    }
}

void ActuatorManager::moveLinearActuator() {
    Serial.println("🔄 선형 액추에이터 이동 시작...");
    
    // 스텝모터 활성화
    digitalWrite(ENABLE_PIN, HIGH);
    delay(100);
    
    // 앞으로 100스텝 이동
    Serial.println("   → 전진 이동 중...");
    for (int i = 0; i < 100; i++) {
        stepper.step(1);
        delay(10);
    }
    
    delay(3000); // 3초 대기
    
    // 뒤로 100스텝 이동 (원위치)
    Serial.println("   → 후진 이동 중...");
    for (int i = 0; i < 100; i++) {
        stepper.step(-1);
        delay(10);
    }
    
    Serial.println("✅ 선형 액추에이터 이동 완료");
}

void ActuatorManager::startWatering() {
    if (!wateringActive) {
        activatePumps(true);
        wateringActive = true;
        wateringStartTime = millis();
        Serial.println("⏰ 정기 물주기 시작");
    }
}

void ActuatorManager::stopWatering() {
    if (wateringActive) {
        activatePumps(false);
        wateringActive = false;
        Serial.println("⏰ 물주기 중지");
    }
}

bool ActuatorManager::checkWateringDuration() {
    if (wateringActive && (millis() - wateringStartTime >= WATERING_DURATION)) {
        stopWatering();
        return true; // 물주기 완료됨
    }
    return false; // 물주기 진행 중 또는 비활성
}

void ActuatorManager::emergencyStop() {
    activatePumps(false);
    activateFans(false);
    activateLED(false);
    digitalWrite(ENABLE_PIN, LOW); // 스텝모터 비활성화
    wateringActive = false;
    
    Serial.println("🚨 비상 정지 작동!");
    Serial.println("   모든 액추에이터 정지됨");
}