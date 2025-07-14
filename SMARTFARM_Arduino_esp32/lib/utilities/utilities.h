#ifndef UTILITIES_H
#define UTILITIES_H

#include <Arduino.h>

namespace SmartFarmUtils {
    
    // 온도 변환 함수들
    float celsiusToFahrenheit(float celsius);
    float fahrenheitToCelsius(float fahrenheit);
    
    // 범위 검증 함수들
    bool isInRange(float value, float min, float max);
    bool isValidTemperature(float temp);
    bool isValidHumidity(float humidity);
    bool isValidLightLevel(int lightLevel);
    
    // 문자열 유틸리티
    String padZero(int value, int width = 2);
    String formatFloat(float value, int decimals = 2);
    
    // 시간 유틸리티
    unsigned long minutesToMillis(int minutes);
    unsigned long hoursToMillis(int hours);
    unsigned long daysToMillis(int days);
    
    // 환경 상태 평가
    enum EnvironmentStatus {
        OPTIMAL,
        WARNING,
        CRITICAL
    };
    
    EnvironmentStatus evaluateTemperature(float temp, float min, float max);
    EnvironmentStatus evaluateHumidity(float humidity, float min, float max);
    EnvironmentStatus evaluateLightLevel(int lightLevel, int min, int max);
    EnvironmentStatus getOverallStatus(EnvironmentStatus temp, EnvironmentStatus humidity, EnvironmentStatus light);
    
    // 로깅 유틸리티
    void logInfo(const String& message);
    void logWarning(const String& message);
    void logError(const String& message);
    void logDebug(const String& message);
    
    // 수학 유틸리티
    float mapFloat(float value, float fromMin, float fromMax, float toMin, float toMax);
    int constrainInt(int value, int min, int max);
    float constrainFloat(float value, float min, float max);
    
    // 배터리/전력 관리 (향후 확장용)
    class PowerManager {
    private:
        bool lowPowerMode;
        unsigned long lastActivityTime;
        
    public:
        PowerManager();
        void init();
        void updateActivity();
        bool shouldEnterLowPower();
        void enterLowPowerMode();
        void exitLowPowerMode();
        bool isLowPowerMode() const { return lowPowerMode; }
    };
    
    // 데이터 저장소 (EEPROM 활용)
    class DataStorage {
    private:
        static const int CONFIG_START_ADDR = 0;
        static const int DATA_START_ADDR = 100;
        
    public:
        DataStorage();
        void init();
        
        // 설정 저장/로드
        void saveConfig(float tempMin, float tempMax, float humidityMin, float humidityMax);
        void loadConfig(float& tempMin, float& tempMax, float& humidityMin, float& humidityMax);
        
        // 통계 데이터 저장/로드
        void saveDailyStats(float avgTemp, float avgHumidity, int avgLight);
        void loadDailyStats(float& avgTemp, float& avgHumidity, int& avgLight);
        
        // 오류 로그
        void saveErrorLog(const String& error);
        String loadErrorLog();
        
        void clearAllData();
    };
    
    // 상태 LED 관리 (시스템 상태 표시용)
    class StatusLED {
    private:
        int pin;
        unsigned long lastBlinkTime;
        bool ledState;
        int blinkPattern;
        
    public:
        enum Pattern {
            OFF = 0,
            ON = 1,
            SLOW_BLINK = 2,    // 1초 간격
            FAST_BLINK = 3,    // 0.2초 간격
            DOUBLE_BLINK = 4,  // 짧은 깜빡임 2번
            SOS = 5            // SOS 패턴
        };
        
        StatusLED(int ledPin);
        void init();
        void setPattern(Pattern pattern);
        void update();
    };
    
    // 시스템 감시견 (Watchdog)
    class SystemWatchdog {
    private:
        unsigned long lastHeartbeat;
        unsigned long timeoutMs;
        bool enabled;
        
    public:
        SystemWatchdog(unsigned long timeout = 30000); // 기본 30초
        void init();
        void heartbeat();
        void enable();
        void disable();
        bool isTimeout();
        void reset();
    };
    
    // 알림 시스템
    class NotificationSystem {
    private:
        struct Notification {
            String message;
            unsigned long timestamp;
            int priority; // 1=낮음, 2=보통, 3=높음, 4=긴급
        };
        
        static const int MAX_NOTIFICATIONS = 10;
        Notification notifications[MAX_NOTIFICATIONS];
        int notificationCount;
        
    public:
        NotificationSystem();
        void init();
        void addNotification(const String& message, int priority = 2);
        void addWarning(const String& message);
        void addError(const String& message);
        void addCritical(const String& message);
        String getLatestNotification();
        int getNotificationCount() const { return notificationCount; }
        void clearNotifications();
        void printAllNotifications();
    };
}

#endif