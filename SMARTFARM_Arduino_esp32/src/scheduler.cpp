#include "scheduler.h"
#include "config.h"

Scheduler::Scheduler() 
    : todayWatering1Done(false), todayWatering2Done(false), 
      todayLinearMove1Done(false), todayLinearMove2Done(false), lastDay(-1) {
}

void Scheduler::init() {
    Serial.println(" 스케줄러 초기화 완료");
}

void Scheduler::update(const DateTime& currentTime) {
    // 날짜가 변경되었으면 일일 작업 리셋
    if (lastDay != currentTime.day()) {
        resetDailyTasks();
        lastDay = currentTime.day();
        Serial.println("새로운 날 - 일일 작업 리셋");
    }
}

void Scheduler::resetDailyTasks() {
    todayWatering1Done = false;
    todayWatering2Done = false;
    todayLinearMove1Done = false;
    todayLinearMove2Done = false;
    
    Serial.println("일일 작업 상태 리셋:");
    Serial.println("   - 물주기 1차: 대기");
    Serial.println("   - 물주기 2차: 대기");
    Serial.println("   - 식물이동 1차: 대기");
    Serial.println("   - 식물이동 2차: 대기");
}

void Scheduler::timeBasedControl(const DateTime& currentTime, ActuatorManager& actuators) {
    // ========== 시간 기반 LED 제어 ==========
    if (isTimeForLED(currentTime)) {
        if (!actuators.isLEDActive()) {
            actuators.activateLED(true);
            Serial.println(" 정기 LED 켜기 (06:00-22:00)");
        }
    } else {
        if (actuators.isLEDActive()) {
            actuators.activateLED(false);
            Serial.println(" 정기 LED 끄기 (야간 모드)");
        }
    }
    
    // ========== 정기 물주기 제어 ==========
    if (isTimeToWater(currentTime)) {
        actuators.startWatering();
    }
    
    // ========== 정기 식물 이동 제어 ==========
    if (isTimeToMovePlant(currentTime)) {
        actuators.moveLinearActuator();
    }
}

void Scheduler::environmentalControl(const SensorManager& sensors, ActuatorManager& actuators) {
    float temperature = sensors.getTemperature();
    float humidity = sensors.getHumidity();
    int lightLevel = sensors.getLightLevel();
    
    // ========== 온도 기반 팬 제어 ==========
    if (temperature > OPTIMAL_TEMP_MAX) {
        if (!actuators.isFanActive()) {
            actuators.activateFans(true);
            Serial.println(" 고온 감지 - 팬 가동");
        }
    } else if (temperature < OPTIMAL_TEMP_MIN) {
        if (actuators.isFanActive()) {
            actuators.activateFans(false);
            Serial.println(" 저온 감지 - 팬 정지 (보온)");
        }
    } else {
        // 적정 온도 범위 내에서는 습도에 따라 팬 제어
        if (humidity > OPTIMAL_HUMIDITY_MAX && !actuators.isFanActive()) {
            actuators.activateFans(true);
            Serial.println("고습도 감지 - 제습용 팬 가동");
        } else if (humidity <= OPTIMAL_HUMIDITY_MAX && actuators.isFanActive() && temperature <= OPTIMAL_TEMP_MAX) {
            actuators.activateFans(false);
            Serial.println("습도 정상 - 팬 정지");
        }
    }
    
    // ========== 습도 기반 추가 물주기 ==========
    if (humidity < OPTIMAL_HUMIDITY_MIN) {
        if (!actuators.isWateringActive() && !actuators.isPumpActive()) {
            Serial.println("저습도 감지 - 추가 물주기 (5초)");
            actuators.activatePumps(true);
            delay(5000); // 5초간 물주기
            actuators.activatePumps(false);
        }
    }
    
    // ========== 조도 기반 추가 LED 제어 ==========
    DateTime currentTime = sensors.getCurrentTime();
    
    // 낮 시간대에 조도가 부족하면 LED 강제 켜기
    if (isTimeForLED(currentTime) && lightLevel < OPTIMAL_LIGHT_MIN) {
        if (!actuators.isLEDActive()) {
            actuators.activateLED(true);
            Serial.println("조도 부족 - LED 강제 켜기");
        }
    }
    // 밤 시간대에 조도가 과도하면 LED 끄기 (절전)
    else if (!isTimeForLED(currentTime) && lightLevel > OPTIMAL_LIGHT_MAX) {
        if (actuators.isLEDActive()) {
            actuators.activateLED(false);
            Serial.println("야간 과도 조도 - LED 끄기");
        }
    }
}

bool Scheduler::isTimeForLED(const DateTime& currentTime) const {
    int currentHour = currentTime.hour();
    return (currentHour >= LED_ON_HOUR && currentHour < LED_OFF_HOUR);
}

bool Scheduler::isTimeToWater(const DateTime& currentTime) {
    int currentHour = currentTime.hour();
    int currentMinute = currentTime.minute();
    
    // 오전 8시 물주기
    if (currentHour == WATERING_HOUR_1 && currentMinute == WATERING_MINUTE && !todayWatering1Done) {
        todayWatering1Done = true;
        Serial.println(" 1차 물주기 시간 (08:00)");
        return true;
    }
    
    // 오후 6시 물주기
    if (currentHour == WATERING_HOUR_2 && currentMinute == WATERING_MINUTE && !todayWatering2Done) {
        todayWatering2Done = true;
        Serial.println("2차 물주기 시간 (18:00)");
        return true;
    }
    
    return false;
}

bool Scheduler::isTimeToMovePlant(const DateTime& currentTime) {
    int currentHour = currentTime.hour();
    int currentMinute = currentTime.minute();
    
    // 오전 10시 이동
    if (currentHour == LINEAR_MOVE_HOUR_1 && currentMinute == LINEAR_MOVE_MINUTE && !todayLinearMove1Done) {
        todayLinearMove1Done = true;
        Serial.println("1차 식물 이동 시간 (10:00)");
        return true;
    }
    
    // 오후 3시 이동
    if (currentHour == LINEAR_MOVE_HOUR_2 && currentMinute == LINEAR_MOVE_MINUTE && !todayLinearMove2Done) {
        todayLinearMove2Done = true;
        Serial.println(" 2차 식물 이동 시간 (15:00)");
        return true;
    }
    
    return false;
}