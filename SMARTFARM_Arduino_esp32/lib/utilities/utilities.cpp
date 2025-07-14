#include "utilities.h"
#include <EEPROM.h>

namespace SmartFarmUtils {
    
    // ========== 온도 변환 함수들 ==========
    float celsiusToFahrenheit(float celsius) {
        return (celsius * 9.0 / 5.0) + 32.0;
    }
    
    float fahrenheitToCelsius(float fahrenheit) {
        return (fahrenheit - 32.0) * 5.0 / 9.0;
    }
    
    // ========== 범위 검증 함수들 ==========
    bool isInRange(float value, float min, float max) {
        return (value >= min && value <= max);
    }
    
    bool isValidTemperature(float temp) {
        return (!isnan(temp) && temp > -50.0 && temp < 100.0);
    }
    
    bool isValidHumidity(float humidity) {
        return (!isnan(humidity) && humidity >= 0.0 && humidity <= 100.0);
    }
    
    bool isValidLightLevel(int lightLevel) {
        return (lightLevel >= 0 && lightLevel <= 1023);
    }
    
    // ========== 문자열 유틸리티 ==========
    String padZero(int value, int width) {
        String result = String(value);
        while (result.length() < width) {
            result = "0" + result;
        }
        return result;
    }
    
    String formatFloat(float value, int decimals) {
        return String(value, decimals);
    }
    
    // ========== 시간 유틸리티 ==========
    unsigned long minutesToMillis(int minutes) {
        return minutes * 60000UL;
    }
    
    unsigned long hoursToMillis(int hours) {
        return hours * 3600000UL;
    }
    
    unsigned long daysToMillis(int days) {
        return days * 86400000UL;
    }
    
    // ========== 환경 상태 평가 ==========
    EnvironmentStatus evaluateTemperature(float temp, float min, float max) {
        if (!isValidTemperature(temp)) return CRITICAL;
        
        float range = max - min;
        float tolerance = range * 0.1; // 10% 여유
        
        if (temp >= min - tolerance && temp <= max + tolerance) {
            return OPTIMAL;
        } else if (temp >= min - (tolerance * 2) && temp <= max + (tolerance * 2)) {
            return WARNING;
        } else {
            return CRITICAL;
        }
    }
    
    EnvironmentStatus evaluateHumidity(float humidity, float min, float max) {
        if (!isValidHumidity(humidity)) return CRITICAL;
        
        float range = max - min;
        float tolerance = range * 0.1;
        
        if (humidity >= min - tolerance && humidity <= max + tolerance) {
            return OPTIMAL;
        } else if (humidity >= min - (tolerance * 2) && humidity <= max + (tolerance * 2)) {
            return WARNING;
        } else {
            return CRITICAL;
        }
    }
    
    EnvironmentStatus evaluateLightLevel(int lightLevel, int min, int max) {
        if (!isValidLightLevel(lightLevel)) return CRITICAL;
        
        int range = max - min;
        int tolerance = range * 0.1;
        
        if (lightLevel >= min - tolerance && lightLevel <= max + tolerance) {
            return OPTIMAL;
        } else if (lightLevel >= min - (tolerance * 2) && lightLevel <= max + (tolerance * 2)) {
            return WARNING;
        } else {
            return CRITICAL;
        }
    }
    
    EnvironmentStatus getOverallStatus(EnvironmentStatus temp, EnvironmentStatus humidity, EnvironmentStatus light) {
        if (temp == CRITICAL || humidity == CRITICAL || light == CRITICAL) {
            return CRITICAL;
        } else if (temp == WARNING || humidity == WARNING || light == WARNING) {
            return WARNING;
        } else {
            return OPTIMAL;
        }
    }
    
    // ========== 로깅 유틸리티 ==========
    void logInfo(const String& message) {
        Serial.print("[INFO] ");
        Serial.println(message);
    }
    
    void logWarning(const String& message) {
        Serial.print("[WARNING] ");
        Serial.println(message);
    }
    
    void logError(const String& message) {
        Serial.print("[ERROR] ");
        Serial.println(message);
    }
    
    void logDebug(const String& message) {
        Serial.print("[DEBUG] ");
        Serial.println(message);
    }
    
    // ========== 수학 유틸리티 ==========
    float mapFloat(float value, float fromMin, float fromMax, float toMin, float toMax) {
        return (value - fromMin) * (toMax - toMin) / (fromMax - fromMin) + toMin;
    }
    
    int constrainInt(int value, int min, int max) {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }
    
    float constrainFloat(float value, float min, float max) {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }
    
    // ========== PowerManager 구현 ==========
    PowerManager::PowerManager() : lowPowerMode(false), lastActivityTime(0) {}
    
    void PowerManager::init() {
        updateActivity();
        logInfo("전력 관리자 초기화 완료");
    }
    
    void PowerManager::updateActivity() {
        lastActivityTime = millis();
        if (lowPowerMode) {
            exitLowPowerMode();
        }
    }
    
    bool PowerManager::shouldEnterLowPower() {
        return (millis() - lastActivityTime > hoursToMillis(1)) && !lowPowerMode;
    }
    
    void PowerManager::enterLowPowerMode() {
        lowPowerMode = true;
        logInfo("저전력 모드 진입");
    }
    
    void PowerManager::exitLowPowerMode() {
        lowPowerMode = false;
        logInfo("저전력 모드 해제");
    }
    
    // ========== DataStorage 구현 ==========
    DataStorage::DataStorage() {}
    
    void DataStorage::init() {
        // EEPROM 초기화는 자동으로 됨
        logInfo("데이터 저장소 초기화 완료");
    }
    
    void DataStorage::saveConfig(float tempMin, float tempMax, float humidityMin, float humidityMax) {
        int addr = CONFIG_START_ADDR;
        EEPROM.put(addr, tempMin); addr += sizeof(float);
        EEPROM.put(addr, tempMax); addr += sizeof(float);
        EEPROM.put(addr, humidityMin); addr += sizeof(float);
        EEPROM.put(addr, humidityMax);
        logInfo("설정 저장 완료");
    }
    
    void DataStorage::loadConfig(float& tempMin, float& tempMax, float& humidityMin, float& humidityMax) {
        int addr = CONFIG_START_ADDR;
        EEPROM.get(addr, tempMin); addr += sizeof(float);
        EEPROM.get(addr, tempMax); addr += sizeof(float);
        EEPROM.get(addr, humidityMin); addr += sizeof(float);
        EEPROM.get(addr, humidityMax);
        logInfo("설정 로드 완료");
    }
    
    void DataStorage::saveDailyStats(float avgTemp, float avgHumidity, int avgLight) {
        int addr = DATA_START_ADDR;
        EEPROM.put(addr, avgTemp); addr += sizeof(float);
        EEPROM.put(addr, avgHumidity); addr += sizeof(float);
        EEPROM.put(addr, avgLight);
        logInfo("일일 통계 저장 완료");
    }
    
    void DataStorage::loadDailyStats(float& avgTemp, float& avgHumidity, int& avgLight) {
        int addr = DATA_START_ADDR;
        EEPROM.get(addr, avgTemp); addr += sizeof(float);
        EEPROM.get(addr, avgHumidity); addr += sizeof(float);
        EEPROM.get(addr, avgLight);
        logInfo("일일 통계 로드 완료");
    }
    
    void DataStorage::saveErrorLog(const String& error) {
        // 간단한 오류 로그 저장 (실제로는 더 복잡한 구조 필요)
        int addr = DATA_START_ADDR + 50;
        for (int i = 0; i < error.length() && i < 50; i++) {
            EEPROM.write(addr + i, error.charAt(i));
        }
        EEPROM.write(addr + error.length(), '\0');
        logInfo("오류 로그 저장: " + error);
    }
    
    String DataStorage::loadErrorLog() {
        String error = "";
        int addr = DATA_START_ADDR + 50;
        char c;
        for (int i = 0; i < 50; i++) {
            c = EEPROM.read(addr + i);
            if (c == '\0') break;
            error += c;
        }
        return error;
    }
    
    void DataStorage::clearAllData() {
        for (int i = 0; i < 512; i++) {
            EEPROM.write(i, 0);
        }
        logInfo("모든 저장 데이터 삭제 완료");
    }
    
    // ========== StatusLED 구현 ==========
    StatusLED::StatusLED(int ledPin) : pin(ledPin), lastBlinkTime(0), ledState(false), blinkPattern(OFF) {}
    
    void StatusLED::init() {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
        logInfo("상태 LED 초기화 완료 (핀 " + String(pin) + ")");
    }
    
    void StatusLED::setPattern(Pattern pattern) {
        blinkPattern = pattern;
        lastBlinkTime = millis();
        
        if (pattern == OFF) {
            digitalWrite(pin, LOW);
            ledState = false;
        } else if (pattern == ON) {
            digitalWrite(pin, HIGH);
            ledState = true;
        }
    }
    
    void StatusLED::update() {
        unsigned long currentTime = millis();
        unsigned long interval = 0;
        
        switch (blinkPattern) {
            case OFF:
            case ON:
                return; // 고정 상태, 업데이트 불필요
                
            case SLOW_BLINK:
                interval = 1000;
                break;
                
            case FAST_BLINK:
                interval = 200;
                break;
                
            case DOUBLE_BLINK:
                // 복잡한 패턴은 상태 머신으로 구현 가능
                interval = 300;
                break;
                
            case SOS:
                // SOS 패턴 구현
                interval = 150;
                break;
        }
        
        if (currentTime - lastBlinkTime >= interval) {
            ledState = !ledState;
            digitalWrite(pin, ledState);
            lastBlinkTime = currentTime;
        }
    }
    
    // ========== SystemWatchdog 구현 ==========
    SystemWatchdog::SystemWatchdog(unsigned long timeout) 
        : lastHeartbeat(0), timeoutMs(timeout), enabled(false) {}
    
    void SystemWatchdog::init() {
        heartbeat();
        enabled = true;
        logInfo("시스템 감시견 초기화 (타임아웃: " + String(timeoutMs/1000) + "초)");
    }
    
    void SystemWatchdog::heartbeat() {
        lastHeartbeat = millis();
    }
    
    void SystemWatchdog::enable() {
        enabled = true;
        heartbeat();
        logInfo("시스템 감시견 활성화");
    }
    
    void SystemWatchdog::disable() {
        enabled = false;
        logInfo("시스템 감시견 비활성화");
    }
    
    bool SystemWatchdog::isTimeout() {
        if (!enabled) return false;
        return (millis() - lastHeartbeat > timeoutMs);
    }
    
    void SystemWatchdog::reset() {
        if (isTimeout()) {
            logError("시스템 감시견 타임아웃 감지! 시스템 리셋 필요");
            // 실제 환경에서는 하드웨어 리셋 수행
        }
        heartbeat();
    }
    
    // ========== NotificationSystem 구현 ==========
    NotificationSystem::NotificationSystem() : notificationCount(0) {}
    
    void NotificationSystem::init() {
        clearNotifications();
        logInfo("알림 시스템 초기화 완료");
    }
    
    void NotificationSystem::addNotification(const String& message, int priority) {
        if (notificationCount < MAX_NOTIFICATIONS) {
            notifications[notificationCount].message = message;
            notifications[notificationCount].timestamp = millis();
            notifications[notificationCount].priority = priority;
            notificationCount++;
            
            String priorityStr;
            switch (priority) {
                case 1: priorityStr = "낮음"; break;
                case 2: priorityStr = "보통"; break;
                case 3: priorityStr = "높음"; break;
                case 4: priorityStr = "긴급"; break;
                default: priorityStr = "알 수 없음"; break;
            }
            
            logInfo("알림 추가 [" + priorityStr + "]: " + message);
        } else {
            logWarning("알림 버퍼 가득참 - 이전 알림 무시됨");
            // 가장 오래된 알림 삭제하고 새 알림 추가
            for (int i = 0; i < MAX_NOTIFICATIONS - 1; i++) {
                notifications[i] = notifications[i + 1];
            }
            notifications[MAX_NOTIFICATIONS - 1].message = message;
            notifications[MAX_NOTIFICATIONS - 1].timestamp = millis();
            notifications[MAX_NOTIFICATIONS - 1].priority = priority;
        }
    }
    
    void NotificationSystem::addWarning(const String& message) {
        addNotification("⚠️ " + message, 3);
    }
    
    void NotificationSystem::addError(const String& message) {
        addNotification("❌ " + message, 4);
    }
    
    void NotificationSystem::addCritical(const String& message) {
        addNotification("🚨 " + message, 4);
    }
    
    String NotificationSystem::getLatestNotification() {
        if (notificationCount > 0) {
            return notifications[notificationCount - 1].message;
        }
        return "";
    }
    
    void NotificationSystem::clearNotifications() {
        notificationCount = 0;
        logInfo("모든 알림 삭제됨");
    }
    
    void NotificationSystem::printAllNotifications() {
        Serial.println("\n========== 알림 목록 ==========");
        if (notificationCount == 0) {
            Serial.println("알림이 없습니다.");
        } else {
            for (int i = 0; i < notificationCount; i++) {
                Serial.print("[");
                Serial.print(i + 1);
                Serial.print("] ");
                
                switch (notifications[i].priority) {
                    case 1: Serial.print("[낮음] "); break;
                    case 2: Serial.print("[보통] "); break;
                    case 3: Serial.print("[높음] "); break;
                    case 4: Serial.print("[긴급] "); break;
                }
                
                Serial.print(notifications[i].message);
                Serial.print(" (");
                Serial.print(notifications[i].timestamp / 1000);
                Serial.println("초 전)");
            }
        }
        Serial.println("================================\n");
    }
}