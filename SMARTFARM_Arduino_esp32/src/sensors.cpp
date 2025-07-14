#include "sensors.h"
#include "config.h"

SensorManager::SensorManager() 
    : dht(DHT_PIN, DHT_TYPE), temperature(0), humidity(0), lightLevel(0), lastReadTime(0) {
}

bool SensorManager::init() {
    // DHT 센서 초기화
    dht.begin();
    
    // I2C 통신 초기화 (RTC용)
    Wire.begin();
    
    // RTC 초기화
    if (!rtc.begin()) {
        Serial.println("❌ RTC 모듈을 찾을 수 없습니다!");
        return false;
    }
    
    // RTC가 작동 중이 아니면 현재 시간으로 설정
    if (!rtc.isrunning()) {
        Serial.println("⚠️ RTC가 작동하지 않습니다. 컴파일 시간으로 설정합니다.");
        // 컴파일 시간으로 RTC 설정 (필요시 주석 해제)
        // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        
        // 수동 시간 설정 예시 (년, 월, 일, 시, 분, 초)
        rtc.adjust(DateTime(2025, 7, 14, 12, 0, 0));
    }
    
    Serial.println("✅ 센서 매니저 초기화 완료");
    return true;
}

void SensorManager::readSensors() {
    unsigned long currentTime = millis();
    
    // 너무 자주 읽지 않도록 제한
    if (currentTime - lastReadTime < 1000) {
        return;
    }
    
    // 온습도 센서 읽기
    float newTemp = dht.readTemperature();
    float newHumidity = dht.readHumidity();
    
    // 센서 오류 체크
    if (!isnan(newTemp) && !isnan(newHumidity)) {
        temperature = newTemp;
        humidity = newHumidity;
    } else {
        Serial.println("❌ DHT 센서 읽기 오류!");
    }
    
    // 조도 센서 읽기
    lightLevel = analogRead(LIGHT_SENSOR_PIN);
    
    lastReadTime = currentTime;
}

void SensorManager::readRTC() {
    currentTime = rtc.now();
}

bool SensorManager::isValidReading() const {
    return (!isnan(temperature) && !isnan(humidity) && 
            temperature > -40 && temperature < 80 &&
            humidity >= 0 && humidity <= 100);
}

String SensorManager::formatTime(DateTime dt) const {
    String timeString = "";
    if (dt.hour() < 10) timeString += "0";
    timeString += String(dt.hour()) + ":";
    if (dt.minute() < 10) timeString += "0";
    timeString += String(dt.minute()) + ":";
    if (dt.second() < 10) timeString += "0";
    timeString += String(dt.second());
    return timeString;
}

String SensorManager::formatDate(DateTime dt) const {
    String dateString = "";
    dateString += String(dt.year()) + "-";
    if (dt.month() < 10) dateString += "0";
    dateString += String(dt.month()) + "-";
    if (dt.day() < 10) dateString += "0";
    dateString += String(dt.day());
    return dateString;
}