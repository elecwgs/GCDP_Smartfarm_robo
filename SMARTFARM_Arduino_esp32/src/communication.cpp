#include "communication.h"
#include "config.h"

CommunicationManager::CommunicationManager() 
    : esp32Serial(ESP32_RX_PIN, ESP32_TX_PIN), lastSendTime(0) {
}

void CommunicationManager::init() {
    esp32Serial.begin(9600);
    Serial.println("✅ 통신 매니저 초기화 완료 (ESP32 연결)");
}

void CommunicationManager::update() {
    // 추후 ESP32로부터 명령 수신 처리 등을 여기에 구현
}

void CommunicationManager::sendToESP32(const SensorManager& sensors, 
                                       const ActuatorManager& actuators, 
                                       const Scheduler& scheduler) {
    String jsonData = createJsonData(sensors, actuators, scheduler);
    
    // ESP32로 전송
    esp32Serial.println(jsonData);
    
    Serial.println("📡 ESP32로 데이터 전송 완료");
    Serial.println("   전송 데이터: " + jsonData);
}

void CommunicationManager::printSystemStatus(const SensorManager& sensors, 
                                            const ActuatorManager& actuators, 
                                            const Scheduler& scheduler) {
    DateTime now = sensors.getCurrentTime();
    
    Serial.println("\n========== 시스템 상태 ==========");
    Serial.print("현재 시간: ");
    Serial.println(sensors.formatTime(now));
    Serial.print("현재 날짜: ");
    Serial.println(sensors.formatDate(now));
    
    // 센서 데이터
    Serial.println("\n--- 센서 데이터 ---");
    Serial.print("온도: ");
    Serial.print(sensors.getTemperature());
    Serial.print("°C (최적: ");
    Serial.print(OPTIMAL_TEMP_MIN);
    Serial.print("-");
    Serial.print(OPTIMAL_TEMP_MAX);
    Serial.println("°C)");
    
    Serial.print("습도: ");
    Serial.print(sensors.getHumidity());
    Serial.print("% (최적: ");
    Serial.print(OPTIMAL_HUMIDITY_MIN);
    Serial.print("-");
    Serial.print(OPTIMAL_HUMIDITY_MAX);
    Serial.println("%)");
    
    Serial.print("조도: ");
    Serial.print(sensors.getLightLevel());
    Serial.print(" (최적: ");
    Serial.print(OPTIMAL_LIGHT_MIN);
    Serial.print("-");
    Serial.print(OPTIMAL_LIGHT_MAX);
    Serial.println(")");
    
    // 액추에이터 상태
    Serial.println("\n--- 액추에이터 상태 ---");
    Serial.print("워터펌프: ");
    Serial.println(actuators.isPumpActive() ? "ON" : "OFF");
    Serial.print("팬: ");
    Serial.println(actuators.isFanActive() ? "ON" : "OFF");
    Serial.print("LED 조명: ");
    Serial.println(actuators.isLEDActive() ? "ON" : "OFF");
    Serial.print("물주기 진행: ");
    Serial.println(actuators.isWateringActive() ? "진행중" : "대기");
    
    // 스케줄 상태
    Serial.println("\n--- 오늘 완료된 작업 ---");
    Serial.print("1차 물주기 (08:00): ");
    Serial.println(scheduler.isWatering1Done() ? "완료" : "대기");
    Serial.print("2차 물주기 (18:00): ");
    Serial.println(scheduler.isWatering2Done() ? "완료" : "대기");
    Serial.print("1차 식물이동 (10:00): ");
    Serial.println(scheduler.isLinearMove1Done() ? "완료" : "대기");
    Serial.print("2차 식물이동 (15:00): ");
    Serial.println(scheduler.isLinearMove2Done() ? "완료" : "대기");
    
    // 환경 상태 평가
    Serial.println("\n--- 환경 상태 평가 ---");
    bool tempOK = (sensors.getTemperature() >= OPTIMAL_TEMP_MIN && 
                   sensors.getTemperature() <= OPTIMAL_TEMP_MAX);
    bool humidityOK = (sensors.getHumidity() >= OPTIMAL_HUMIDITY_MIN && 
                       sensors.getHumidity() <= OPTIMAL_HUMIDITY_MAX);
    bool lightOK = (sensors.getLightLevel() >= OPTIMAL_LIGHT_MIN && 
                    sensors.getLightLevel() <= OPTIMAL_LIGHT_MAX);
    
    Serial.print("온도 상태: ");
    Serial.println(tempOK ? "✅ 적정" : "⚠️ 주의");
    Serial.print("습도 상태: ");
    Serial.println(humidityOK ? "✅ 적정" : "⚠️ 주의");
    Serial.print("조도 상태: ");
    Serial.println(lightOK ? "✅ 적정" : "⚠️ 주의");
    
    Serial.print("전체 환경: ");
    if (tempOK && humidityOK && lightOK) {
        Serial.println("🌱 최적 상태");
    } else {
        Serial.println("⚠️ 조정 필요");
    }
    
    Serial.println("================================\n");
}

String CommunicationManager::createJsonData(const SensorManager& sensors, 
                                           const ActuatorManager& actuators, 
                                           const Scheduler& scheduler) {
    DateTime now = sensors.getCurrentTime();
    
    String jsonData = "{";
    
    // 센서 데이터
    jsonData += "\"temperature\":" + String(sensors.getTemperature(), 2) + ",";
    jsonData += "\"humidity\":" + String(sensors.getHumidity(), 2) + ",";
    jsonData += "\"lightLevel\":" + String(sensors.getLightLevel()) + ",";
    
    // 액추에이터 상태
    jsonData += "\"pumpStatus\":" + String(actuators.isPumpActive() ? "true" : "false") + ",";
    jsonData += "\"fanStatus\":" + String(actuators.isFanActive() ? "true" : "false") + ",";
    jsonData += "\"ledStatus\":" + String(actuators.isLEDActive() ? "true" : "false") + ",";
    jsonData += "\"wateringActive\":" + String(actuators.isWateringActive() ? "true" : "false") + ",";
    
    // 시간 정보
    jsonData += "\"currentTime\":\"" + sensors.formatTime(now) + "\",";
    jsonData += "\"currentDate\":\"" + sensors.formatDate(now) + "\",";
    jsonData += "\"hour\":" + String(now.hour()) + ",";
    jsonData += "\"minute\":" + String(now.minute()) + ",";
    jsonData += "\"second\":" + String(now.second()) + ",";
    
    // 스케줄 상태
    jsonData += "\"watering1Done\":" + String(scheduler.isWatering1Done() ? "true" : "false") + ",";
    jsonData += "\"watering2Done\":" + String(scheduler.isWatering2Done() ? "true" : "false") + ",";
    jsonData += "\"linearMove1Done\":" + String(scheduler.isLinearMove1Done() ? "true" : "false") + ",";
    jsonData += "\"linearMove2Done\":" + String(scheduler.isLinearMove2Done() ? "true" : "false") + ",";
    
    // 환경 상태 평가
    bool tempOK = (sensors.getTemperature() >= OPTIMAL_TEMP_MIN && 
                   sensors.getTemperature() <= OPTIMAL_TEMP_MAX);
    bool humidityOK = (sensors.getHumidity() >= OPTIMAL_HUMIDITY_MIN && 
                       sensors.getHumidity() <= OPTIMAL_HUMIDITY_MAX);
    bool lightOK = (sensors.getLightLevel() >= OPTIMAL_LIGHT_MIN && 
                    sensors.getLightLevel() <= OPTIMAL_LIGHT_MAX);
    
    jsonData += "\"tempStatus\":\"" + String(tempOK ? "OK" : "WARNING") + "\",";
    jsonData += "\"humidityStatus\":\"" + String(humidityOK ? "OK" : "WARNING") + "\",";
    jsonData += "\"lightStatus\":\"" + String(lightOK ? "OK" : "WARNING") + "\",";
    jsonData += "\"overallStatus\":\"" + String((tempOK && humidityOK && lightOK) ? "OPTIMAL" : "NEEDS_ATTENTION") + "\",";
    
    // 타임스탬프
    jsonData += "\"timestamp\":" + String(now.unixtime()) + ",";
    jsonData += "\"uptime\":" + String(millis());
    
    jsonData += "}";
    
    return jsonData;
}