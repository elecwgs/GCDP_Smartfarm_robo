#include <unity.h>
#include <Arduino.h>
#include "sensors.h"
#include "config.h"

// 테스트용 센서 매니저
SensorManager sensors;

// 테스트 시작 전 초기화
void setUp(void) {
    // 센서 매니저 초기화
    sensors.init();
    
    // 테스트 시작 메시지
    Serial.println("🧪 센서 테스트 시작");
}

// 테스트 종료 후 정리
void tearDown(void) {
    // 정리 작업 (필요 시)
    delay(100);
}

// 테스트 1: 센서 초기화 확인
void test_sensor_initialization(void) {
    Serial.println("   테스트: 센서 초기화");
    
    // 센서 매니저가 정상적으로 초기화되었는지 확인
    bool initResult = sensors.init();
    TEST_ASSERT_TRUE_MESSAGE(initResult, "센서 초기화 실패");
    
    Serial.println("   ✅ 센서 초기화 성공");
}

// 테스트 2: DHT 센서 데이터 읽기
void test_dht_sensor_reading(void) {
    Serial.println("   테스트: DHT 센서 데이터 읽기");
    
    // 센서 데이터 읽기
    sensors.readSensors();
    
    // 온도 값 확인
    float temperature = sensors.getTemperature();
    TEST_ASSERT_FALSE_MESSAGE(isnan(temperature), "온도 읽기 실패 (NaN)");
    TEST_ASSERT_GREATER_THAN_MESSAGE(-50.0, temperature, "온도 값이 너무 낮음");
    TEST_ASSERT_LESS_THAN_MESSAGE(80.0, temperature, "온도 값이 너무 높음");
    
    // 습도 값 확인
    float humidity = sensors.getHumidity();
    TEST_ASSERT_FALSE_MESSAGE(isnan(humidity), "습도 읽기 실패 (NaN)");
    TEST_ASSERT_GREATER_THAN_OR_EQUAL_MESSAGE(0.0, humidity, "습도 값이 0% 미만");
    TEST_ASSERT_LESS_THAN_OR_EQUAL_MESSAGE(100.0, humidity, "습도 값이 100% 초과");
    
    Serial.print("   📊 온도: ");
    Serial.print(temperature);
    Serial.print("°C, 습도: ");
    Serial.print(humidity);
    Serial.println("%");
    
    Serial.println("   ✅ DHT 센서 읽기 성공");
}

// 테스트 3: 조도 센서 읽기
void test_light_sensor_reading(void) {
    Serial.println("   테스트: 조도 센서 읽기");
    
    // 센서 데이터 읽기
    sensors.readSensors();
    
    // 조도 값 확인
    int lightLevel = sensors.getLightLevel();
    TEST_ASSERT_GREATER_THAN_OR_EQUAL_MESSAGE(0, lightLevel, "조도 값이 0 미만");
    TEST_ASSERT_LESS_THAN_OR_EQUAL_MESSAGE(1023, lightLevel, "조도 값이 1023 초과");
    
    Serial.print("   💡 조도: ");
    Serial.println(lightLevel);
    
    Serial.println("   ✅ 조도 센서 읽기 성공");
}

// 테스트 4: RTC 기능 확인
void test_rtc_functionality(void) {
    Serial.println("   테스트: RTC 기능");
    
    // RTC 시간 읽기
    sensors.readRTC();
    DateTime now = sensors.getCurrentTime();
    
    // 연도 확인 (2000년 이후)
    TEST_ASSERT_GREATER_THAN_MESSAGE(2000, now.year(), "RTC 연도가 잘못됨");
    TEST_ASSERT_LESS_THAN_MESSAGE(2100, now.year(), "RTC 연도가 너무 큼");
    
    // 월 확인 (1-12)
    TEST_ASSERT_GREATER_THAN_OR_EQUAL_MESSAGE(1, now.month(), "RTC 월이 1 미만");
    TEST_ASSERT_LESS_THAN_OR_EQUAL_MESSAGE(12, now.month(), "RTC 월이 12 초과");
    
    // 일 확인 (1-31)
    TEST_ASSERT_GREATER_THAN_OR_EQUAL_MESSAGE(1, now.day(), "RTC 일이 1 미만");
    TEST_ASSERT_LESS_THAN_OR_EQUAL_MESSAGE(31, now.day(), "RTC 일이 31 초과");
    
    // 시간 확인 (0-23)
    TEST_ASSERT_GREATER_THAN_OR_EQUAL_MESSAGE(0, now.hour(), "RTC 시간이 0 미만");
    TEST_ASSERT_LESS_THAN_MESSAGE(24, now.hour(), "RTC 시간이 24 이상");
    
    // 분 확인 (0-59)
    TEST_ASSERT_GREATER_THAN_OR_EQUAL_MESSAGE(0, now.minute(), "RTC 분이 0 미만");
    TEST_ASSERT_LESS_THAN_MESSAGE(60, now.minute(), "RTC 분이 60 이상");
    
    Serial.print("   🕐 현재 시간: ");
    Serial.println(sensors.formatTime(now));
    Serial.print("   📅 현재 날짜: ");
    Serial.println(sensors.formatDate(now));
    
    Serial.println("   ✅ RTC 기능 정상");
}

// 테스트 5: 센서 데이터 유효성 검증
void test_sensor_data_validation(void) {
    Serial.println("   테스트: 센서 데이터 유효성");
    
    // 센서 데이터 읽기
    sensors.readSensors();
    
    // 데이터 유효성 확인
    bool isValid = sensors.isValidReading();
    TEST_ASSERT_TRUE_MESSAGE(isValid, "센서 데이터가 유효하지 않음");
    
    // 개별 센서 값 재확인
    float temp = sensors.getTemperature();
    float humidity = sensors.getHumidity();
    int light = sensors.getLightLevel();
    
    TEST_ASSERT_FALSE_MESSAGE(isnan(temp), "온도 데이터 무효");
    TEST_ASSERT_FALSE_MESSAGE(isnan(humidity), "습도 데이터 무효");
    TEST_ASSERT_NOT_EQUAL_MESSAGE(-1, light, "조도 데이터 무효");
    
    Serial.println("   ✅ 센서 데이터 유효성 확인");
}

// 테스트 6: 연속 읽기 안정성
void test_continuous_reading_stability(void) {
    Serial.println("   테스트: 연속 읽기 안정성");
    
    float tempSum = 0;
    float humiditySum = 0;
    int lightSum = 0;
    int validReadings = 0;
    
    // 10회 연속 읽기
    for (int i = 0; i < 10; i++) {
        sensors.readSensors();
        
        if (sensors.isValidReading()) {
            tempSum += sensors.getTemperature();
            humiditySum += sensors.getHumidity();
            lightSum += sensors.getLightLevel();
            validReadings++;
        }
        
        delay(100); // 100ms 간격
    }
    
    // 최소 80% 이상의 읽기가 성공해야 함
    TEST_ASSERT_GREATER_THAN_OR_EQUAL_MESSAGE(8, validReadings, "연속 읽기 실패율 너무 높음");
    
    if (validReadings > 0) {
        float avgTemp = tempSum / validReadings;
        float avgHumidity = humiditySum / validReadings;
        int avgLight = lightSum / validReadings;
        
        Serial.print("   📊 평균값 - 온도: ");
        Serial.print(avgTemp);
        Serial.print("°C, 습도: ");
        Serial.print(avgHumidity);
        Serial.print("%, 조도: ");
        Serial.println(avgLight);
    }
    
    Serial.println("   ✅ 연속 읽기 안정성 확인");
}

// 테스트 7: 시간 포맷팅 기능
void test_time_formatting(void) {
    Serial.println("   테스트: 시간 포맷팅");
    
    sensors.readRTC();
    DateTime now = sensors.getCurrentTime();
    
    // 시간 포맷 확인
    String timeStr = sensors.formatTime(now);
    TEST_ASSERT_EQUAL_MESSAGE(8, timeStr.length(), "시간 포맷 길이 오류"); // HH:MM:SS = 8자
    TEST_ASSERT_EQUAL_MESSAGE(':', timeStr.charAt(2), "시간 포맷 구분자 오류");
    TEST_ASSERT_EQUAL_MESSAGE(':', timeStr.charAt(5), "시간 포맷 구분자 오류");
    
    // 날짜 포맷 확인
    String dateStr = sensors.formatDate(now);
    TEST_ASSERT_EQUAL_MESSAGE(10, dateStr.length(), "날짜 포맷 길이 오류"); // YYYY-MM-DD = 10자
    TEST_ASSERT_EQUAL_MESSAGE('-', dateStr.charAt(4), "날짜 포맷 구분자 오류");
    TEST_ASSERT_EQUAL_MESSAGE('-', dateStr.charAt(7), "날짜 포맷 구분자 오류");
    
    Serial.print("   🕐 포맷된 시간: ");
    Serial.println(timeStr);
    Serial.print("   📅 포맷된 날짜: ");
    Serial.println(dateStr);
    
    Serial.println("   ✅ 시간 포맷팅 정상");
}

// 테스트 8: 오류 상황 처리
void test_error_handling(void) {
    Serial.println("   테스트: 오류 상황 처리");
    
    // 센서 연결이 불안정한 상황을 시뮬레이션하기 어려우므로
    // 기본적인 오류 처리 로직만 확인
    
    // 여러 번 읽기 시도
    bool anyValidReading = false;
    for (int i = 0; i < 5; i++) {
        sensors.readSensors();
        if (sensors.isValidReading()) {
            anyValidReading = true;
            break;
        }
        delay(500);
    }
    
    // 최소 한 번은 성공적인 읽기가 있어야 함
    TEST_ASSERT_TRUE_MESSAGE(anyValidReading, "모든 센서 읽기 시도 실패");
    
    Serial.println("   ✅ 오류 처리 확인");
}

// 테스트 9: 메모리 사용량 확인
void test_memory_usage(void) {
    Serial.println("   테스트: 메모리 사용량");
    
    // 시작 시 메모리 확인
    #ifdef __AVR__
    extern int __heap_start, *__brkval; 
    int freeMemoryStart = (int)&__heap_start - (int)__brkval;
    #endif
    
    // 센서 여러 번 읽기
    for (int i = 0; i < 20; i++) {
        sensors.readSensors();
        sensors.readRTC();
        delay(10);
    }
    
    #ifdef __AVR__
    int freeMemoryEnd = (int)&__heap_start - (int)__brkval;
    
    // 메모리 누수가 심하지 않아야 함 (100바이트 이하)
    int memoryDiff = freeMemoryStart - freeMemoryEnd;
    TEST_ASSERT_LESS_THAN_MESSAGE(100, abs(memoryDiff), "메모리 누수 의심");
    
    Serial.print("   💾 메모리 변화: ");
    Serial.print(memoryDiff);
    Serial.println(" bytes");
    #endif
    
    Serial.println("   ✅ 메모리 사용량 확인");
}

// 테스트 10: 성능 벤치마크
void test_performance_benchmark(void) {
    Serial.println("   테스트: 성능 벤치마크");
    
    unsigned long startTime = micros();
    
    // 100회 센서 읽기
    for (int i = 0; i < 100; i++) {
        sensors.readSensors();
    }
    
    unsigned long endTime = micros();
    unsigned long totalTime = endTime - startTime;
    unsigned long avgTime = totalTime / 100;
    
    // 평균 읽기 시간이 100ms 이하여야 함
    TEST_ASSERT_LESS_THAN_MESSAGE(100000, avgTime, "센서 읽기 속도 너무 느림");
    
    Serial.print("   ⚡ 평균 읽기 시간: ");
    Serial.print(avgTime);
    Serial.println(" μs");
    
    Serial.println("   ✅ 성능 벤치마크 통과");
}

// 메인 테스트 실행 함수
int main(int argc, char **argv) {
    // Arduino 환경에서는 setup() 호출
    #ifdef ARDUINO
    init();
    #endif
    
    delay(2000); // 시리얼 안정화 대기
    
    UNITY_BEGIN();
    
    Serial.println("\n🧪 센서 매니저 테스트 스위트 시작");
    Serial.println("=" + String("=", 50));
    
    RUN_TEST(test_sensor_initialization);
    RUN_TEST(test_dht_sensor_reading);
    RUN_TEST(test_light_sensor_reading);
    RUN_TEST(test_rtc_functionality);
    RUN_TEST(test_sensor_data_validation);
    RUN_TEST(test_continuous_reading_stability);
    RUN_TEST(test_time_formatting);
    RUN_TEST(test_error_handling);
    RUN_TEST(test_memory_usage);
    RUN_TEST(test_performance_benchmark);
    
    Serial.println("=" + String("=", 50));
    Serial.println("🏁 센서 테스트 완료");
    
    UNITY_END();
    
    return 0;
}

// Arduino 환경에서는 loop() 함수 필요
#ifdef ARDUINO
void setup() {
    main(0, NULL);
}

void loop() {
    // 테스트는 한 번만 실행
    delay(1000);
}
#endif