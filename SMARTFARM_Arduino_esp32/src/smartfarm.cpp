#include "smartfarm.h"
#include "config.h"

SmartFarm::SmartFarm() 
    : lastSensorRead(0), lastESP32Send(0), lastRTCRead(0) {
}

bool SmartFarm::init() {
    printStartupMessage();
    
    Serial.println("🔧 시스템 초기화 중...");
    
    // 각 모듈 순차적으로 초기화
    Serial.println("   📡 센서 모듈 초기화...");
    if (!sensors.init()) {
        Serial.println("❌ 센서 초기화 실패");
        return false;
    }
    
    Serial.println("   🔧 액추에이터 모듈 초기화...");
    actuators.init();
    
    Serial.println("   ⏰ 스케줄러 초기화...");
    scheduler.init();
    
    Serial.println("   📡 통신 모듈 초기화...");
    communication.init();
    
    // 초기 센서 읽기
    Serial.println("   📊 초기 센서 데이터 읽기...");
    sensors.readRTC();
    sensors.readSensors();
    
    // 시작 상태 출력
    Serial.println("\n🎯 초기 상태:");
    DateTime now = sensors.getCurrentTime();
    Serial.print("   현재 시간: ");
    Serial.println(sensors.formatTime(now));
    Serial.print("   현재 날짜: ");
    Serial.println(sensors.formatDate(now));
    Serial.print("   현재 온도: ");
    Serial.print(sensors.getTemperature());
    Serial.println("°C");
    Serial.print("   현재 습도: ");
    Serial.print(sensors.getHumidity());
    Serial.println("%");
    Serial.print("   현재 조도: ");
    Serial.println(sensors.getLightLevel());
    
    // 초기 스케줄 업데이트
    scheduler.update(sensors.getCurrentTime());
    
    Serial.println("\n✅ 모든 모듈 초기화 완료!");
    Serial.println("🚀 스마트팜 시스템 가동 시작\n");
    
    return true;
}

void SmartFarm::run() {
    unsigned long currentTime = millis();
    
    // RTC 시간 읽기
    if (currentTime - lastRTCRead >= RTC_READ_INTERVAL) {
        sensors.readRTC();
        lastRTCRead = currentTime;
    }
    
    // 센서 데이터 읽기 및 제어
    if (currentTime - lastSensorRead >= SENSOR_INTERVAL) {
        sensors.readSensors();
        
        // 센서 데이터가 유효한 경우에만 제어 실행
        if (sensors.isValidReading()) {
            scheduler.update(sensors.getCurrentTime());
            scheduler.timeBasedControl(sensors.getCurrentTime(), actuators);
            scheduler.environmentalControl(sensors, actuators);
            communication.printSystemStatus(sensors, actuators, scheduler);
        } else {
            Serial.println("⚠️ 센서 데이터 오류 - 제어 건너뜀");
        }
        
        lastSensorRead = currentTime;
    }
    
    // ESP32 통신
    if (currentTime - lastESP32Send >= ESP32_SEND_INTERVAL) {
        communication.sendToESP32(sensors, actuators, scheduler);
        lastESP32Send = currentTime;
    }
    
    // 물주기 지속 시간 체크
    if (actuators.checkWateringDuration()) {
        Serial.println("⏰ 정기 물주기 완료");
    }
    
    // 통신 업데이트 (ESP32로부터 명령 수신 등)
    communication.update();
    
    delay(100);
}

void SmartFarm::printStartupMessage() {
    Serial.println("========================================");
    Serial.println("🌱 스마트팜 상추재배 시스템 v2.0");
    Serial.println("========================================");
    Serial.println("📋 시스템 사양:");
    Serial.println("   - Arduino Uno 기반");
    Serial.println("   - RTC 모듈 (실시간 시간 관리)");
    Serial.println("   - DHT22 온습도 센서");
    Serial.println("   - 조도 센서 (LDR)");
    Serial.println("   - 워터펌프 2개");
    Serial.println("   - 냉각팬 4개");
    Serial.println("   - LED 성장등");
    Serial.println("   - 스텝모터 (선형 액추에이터)");
    Serial.println("   - ESP32 통신");
    Serial.println("");
    Serial.println("⏰ 제어 스케줄:");
    Serial.println("   - LED 조명: 06:00-22:00 (16시간)");
    Serial.println("   - 물주기: 08:00, 18:00 (30초간)");
    Serial.println("   - 식물이동: 10:00, 15:00");
    Serial.println("   - 환경제어: 실시간 모니터링");
    Serial.println("");
    Serial.println("🎯 최적 환경 기준:");
    Serial.print("   - 온도: ");
    Serial.print(OPTIMAL_TEMP_MIN);
    Serial.print("°C ~ ");
    Serial.print(OPTIMAL_TEMP_MAX);
    Serial.println("°C");
    Serial.print("   - 습도: ");
    Serial.print(OPTIMAL_HUMIDITY_MIN);
    Serial.print("% ~ ");
    Serial.print(OPTIMAL_HUMIDITY_MAX);
    Serial.println("%");
    Serial.print("   - 조도: ");
    Serial.print(OPTIMAL_LIGHT_MIN);
    Serial.print(" ~ ");
    Serial.println(OPTIMAL_LIGHT_MAX);
    Serial.println("========================================");
}

void SmartFarm::emergencyStop() {
    Serial.println("🚨 비상 정지 신호 수신!");
    actuators.emergencyStop();
    
    // 모든 타이머 정지
    lastSensorRead = 0;
    lastESP32Send = 0;
    lastRTCRead = 0;
    
    Serial.println("⏹️ 시스템 완전 정지");
    Serial.println("   재시작하려면 리셋 버튼을 누르세요.");
}