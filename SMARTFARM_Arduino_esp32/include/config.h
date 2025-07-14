#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ========== 시스템 정보 ==========
#define SYSTEM_NAME "스마트팜 상추재배 시스템"
#define SYSTEM_VERSION "2.0.0"

const unsigned long SERIAL_BAUD = 9600;
const unsigned long COMM_TIMEOUT = 5000;

#define HARDWARE_VERSION "v1.2"
#define BUILD_DATE __DATE__
#define BUILD_TIME __TIME__

// ========== 핀 정의 ==========
// DHT22 온습도 센서
#define DHT_PIN 2
#define DHT_TYPE DHT22

// 조도 센서 (LDR)
#define LIGHT_SENSOR_PIN A1

// 워터펌프 (DC380 펌프모터 2개)
#define PUMP1_PIN 3
#define PUMP2_PIN 4

// 팬 4개 (냉각/순환용)
#define FAN1_PIN 5
#define FAN2_PIN 6
#define FAN3_PIN 7
#define FAN4_PIN 8

// LED 조명 (성장등)
#define LED_PIN 9

// 스텝모터 (CNC 리니어 가이드) - 드라이버 연결
#define STEP_PIN 10
#define DIR_PIN 11
#define ENABLE_PIN 12

// 상태 LED (내장 LED)
#define STATUS_LED_PIN LED_BUILTIN

// ESP32 통신 (유선)
#define ESP32_RX_PIN A2
#define ESP32_TX_PIN A3

// 비상 정지 버튼 (선택사항)
#define EMERGENCY_STOP_PIN A0

// ========== 상추 재배 최적 환경 설정 ==========
const float OPTIMAL_TEMP_MIN = 15.0;    // 최적 온도 하한 (°C)
const float OPTIMAL_TEMP_MAX = 20.0;    // 최적 온도 상한 (°C)
const float OPTIMAL_HUMIDITY_MIN = 60.0; // 최적 습도 하한 (%)
const float OPTIMAL_HUMIDITY_MAX = 80.0; // 최적 습도 상한 (%)
const int OPTIMAL_LIGHT_MIN = 300;       // 조도 하한 (0-1023)
const int OPTIMAL_LIGHT_MAX = 600;       // 조도 상한 (0-1023)

// ========== 환경 임계값 설정 ==========
const float TEMP_TOLERANCE = 2.0;       // 온도 허용 오차 (°C)
const float HUMIDITY_TOLERANCE = 5.0;   // 습도 허용 오차 (%)
const int LIGHT_TOLERANCE = 50;         // 조도 허용 오차

const float TEMP_CRITICAL_LOW = 5.0;    // 온도 위험 하한 (°C)
const float TEMP_CRITICAL_HIGH = 35.0;  // 온도 위험 상한 (°C)
const float HUMIDITY_CRITICAL_LOW = 30.0; // 습도 위험 하한 (%)
const float HUMIDITY_CRITICAL_HIGH = 95.0; // 습도 위험 상한 (%)
const int LIGHT_CRITICAL_LOW = 100;     // 조도 위험 하한
const int LIGHT_CRITICAL_HIGH = 900;    // 조도 위험 상한

// ========== 시간 기반 제어 설정 ==========
// LED 조명 시간 (24시간 기준)
const int LED_ON_HOUR = 6;     // 오전 6시
const int LED_OFF_HOUR = 22;   // 오후 10시
const int LED_DAILY_HOURS = 16; // 일일 조명 시간

// 물주기 시간 (매일 2번: 오전 8시, 오후 6시)
const int WATERING_HOUR_1 = 8;   // 오전 8시
const int WATERING_HOUR_2 = 18;  // 오후 6시
const int WATERING_MINUTE = 0;   // 정각
const unsigned long WATERING_DURATION = 30000UL; // 30초간 물주기

// 선형 액추에이터 이동 시간 (매일 오전 10시, 오후 3시)
const int LINEAR_MOVE_HOUR_1 = 10;  // 오전 10시
const int LINEAR_MOVE_HOUR_2 = 15;  // 오후 3시
const int LINEAR_MOVE_MINUTE = 0;   // 정각

// 긴급 물주기 설정
const float EMERGENCY_WATERING_HUMIDITY = 50.0;  // 습도 50% 이하 시 긴급 물주기
const unsigned long EMERGENCY_WATERING_DURATION = 5000UL; // 5초간
const unsigned long EMERGENCY_WATERING_COOLDOWN = 1800000UL; // 30분 쿨다운

// ========== 타이머 간격 설정 ==========
const unsigned long SENSOR_INTERVAL = 2000;      // 2초마다 센서 읽기
const unsigned long ESP32_SEND_INTERVAL = 10000; // 10초마다 ESP32로 전송
const unsigned long RTC_READ_INTERVAL = 1000;    // 1초마다 RTC 읽기
const unsigned long STATUS_PRINT_INTERVAL = 5000; // 5초마다 상태 출력
const unsigned long WATCHDOG_TIMEOUT = 30000;    // 30초 감시견 타임아웃

// ========== 스텝모터 설정 ==========
const int STEPPER_STEPS_PER_REV = 200;  // 200스텝/회전
const int STEPPER_SPEED_RPM = 60;       // 60 RPM
const int STEPPER_MAX_STEPS = 1000;     // 최대 이동 스텝 수
const int STEPPER_MOVE_STEPS = 100;     // 기본 이동 스텝 수
const unsigned long STEPPER_REST_TIME = 3000; // 이동 후 대기 시간 (ms)

// ========== 통신 설정 ==========
//const unsigned long SERIAL_BAUD = 9600;   // 시리얼 통신 속도
const unsigned long ESP32_BAUD = 9600;    // ESP32 통신 속도
//const unsigned long COMM_TIMEOUT = 5000;  // 통신 타임아웃 (ms)

// ========== 안전 설정 ==========
const int MAX_CONSECUTIVE_ERRORS = 5;     // 연속 오류 허용 횟수
const unsigned long ERROR_RECOVERY_DELAY = 5000; // 오류 복구 대기 시간 (ms)
const int AUTO_RECOVERY_ATTEMPTS = 3;     // 자동 복구 시도 횟수

// 비상 정지 조건
const float EMERGENCY_TEMP_HIGH = 35.0;   // 비상 정지 온도 상한
const float EMERGENCY_TEMP_LOW = 0.0;     // 비상 정지 온도 하한
const float EMERGENCY_HUMIDITY_HIGH = 95.0; // 비상 정지 습도 상한

// ========== 메모리 관리 ==========
const int MAX_STRING_LENGTH = 128;        // 최대 문자열 길이
const int JSON_BUFFER_SIZE = 512;         // JSON 버퍼 크기
const int NOTIFICATION_BUFFER_SIZE = 10;  // 알림 버퍼 크기

// ========== 데이터 로깅 설정 ==========
const bool DATA_LOGGING_ENABLED = true;   // 데이터 로깅 활성화
const unsigned long LOG_INTERVAL = 60000; // 1분마다 로그 저장
const int LOG_RETENTION_DAYS = 7;         // 로그 보관 일수
const bool BACKUP_TO_EEPROM = true;       // EEPROM 백업 활성화

// ========== EEPROM 주소 맵 ==========
const int EEPROM_CONFIG_START = 0;        // 설정 시작 주소
const int EEPROM_CONFIG_SIZE = 100;       // 설정 영역 크기
const int EEPROM_DATA_START = 100;        // 데이터 시작 주소
const int EEPROM_DATA_SIZE = 200;         // 데이터 영역 크기
const int EEPROM_LOG_START = 300;         // 로그 시작 주소
const int EEPROM_LOG_SIZE = 200;          // 로그 영역 크기

// ========== 디버그 설정 ==========
#ifdef DEBUG
    #define DEBUG_PRINT(x) Serial.print(x)
    #define DEBUG_PRINTLN(x) Serial.println(x)
    #define DEBUG_PRINTF(format, ...) Serial.printf(format, __VA_ARGS__)
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
    #define DEBUG_PRINTF(format, ...)
#endif

// 로그 레벨 정의
#define LOG_LEVEL_ERROR   0
#define LOG_LEVEL_WARNING 1
#define LOG_LEVEL_INFO    2
#define LOG_LEVEL_DEBUG   3
#define LOG_LEVEL_TRACE   4

#ifndef LOG_LEVEL
    #define LOG_LEVEL LOG_LEVEL_INFO
#endif

// ========== 시스템 상태 열거형 ==========
enum SystemState {
    STATE_INITIALIZING = 0,
    STATE_NORMAL = 1,
    STATE_WARNING = 2,
    STATE_ERROR = 3,
    STATE_EMERGENCY = 4,
    STATE_MAINTENANCE = 5
};

enum EnvironmentStatus {
    ENV_OPTIMAL = 0,
    ENV_WARNING = 1,
    ENV_CRITICAL = 2
};

enum ActuatorState {
    ACTUATOR_OFF = 0,
    ACTUATOR_ON = 1,
    ACTUATOR_ERROR = 2,
    ACTUATOR_MAINTENANCE = 3
};

// ========== 센서 타입 정의 ==========
enum SensorType {
    SENSOR_TEMPERATURE = 0,
    SENSOR_HUMIDITY = 1,
    SENSOR_LIGHT = 2,
    SENSOR_RTC = 3
};

// ========== 액추에이터 타입 정의 ==========
enum ActuatorType {
    ACTUATOR_PUMP = 0,
    ACTUATOR_FAN = 1,
    ACTUATOR_LED = 2,
    ACTUATOR_STEPPER = 3
};

// ========== 알림 우선순위 ==========
enum NotificationPriority {
    PRIORITY_LOW = 1,
    PRIORITY_NORMAL = 2,
    PRIORITY_HIGH = 3,
    PRIORITY_CRITICAL = 4
};

// ========== 제어 모드 ==========
enum ControlMode {
    MODE_AUTOMATIC = 0,
    MODE_MANUAL = 1,
    MODE_SCHEDULE_ONLY = 2,
    MODE_ENVIRONMENT_ONLY = 3,
    MODE_EMERGENCY = 4
};

// ========== 시즌 설정 (향후 확장용) ==========
enum Season {
    SEASON_SPRING = 0,
    SEASON_SUMMER = 1,
    SEASON_AUTUMN = 2,
    SEASON_WINTER = 3
};

// ========== 작물 타입 (향후 확장용) ==========
enum CropType {
    CROP_LETTUCE = 0,
    CROP_SPINACH = 1,
    CROP_KALE = 2,
    CROP_HERBS = 3
};

// ========== 매크로 함수들 ==========
// 범위 체크 매크로
#define IN_RANGE(value, min, max) ((value) >= (min) && (value) <= (max))

// 절댓값 매크로
#define ABS(x) ((x) < 0 ? -(x) : (x))

// 최대/최소값 매크로
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// 제한 매크로
#define CONSTRAIN(value, min, max) ((value) < (min) ? (min) : ((value) > (max) ? (max) : (value)))

// 시간 변환 매크로
#define MINUTES_TO_MS(minutes) ((minutes) * 60000UL)
#define HOURS_TO_MS(hours) ((hours) * 3600000UL)
#define DAYS_TO_MS(days) ((days) * 86400000UL)

// 온도 변환 매크로
#define CELSIUS_TO_FAHRENHEIT(c) (((c) * 9.0 / 5.0) + 32.0)
#define FAHRENHEIT_TO_CELSIUS(f) (((f) - 32.0) * 5.0 / 9.0)

// ========== 문자열 상수 ==========
// 시스템 메시지
const char* const SYSTEM_STARTUP_MSG = "🌱 스마트팜 시스템 시작";
const char* const SYSTEM_READY_MSG = "✅ 시스템 준비 완료";
const char* const SYSTEM_ERROR_MSG = "❌ 시스템 오류";
const char* const SYSTEM_WARNING_MSG = "⚠️ 시스템 경고";

// 센서 메시지
const char* const SENSOR_INIT_MSG = "📊 센서 초기화";
const char* const SENSOR_READ_MSG = "📈 센서 데이터 읽기";
const char* const SENSOR_ERROR_MSG = "❌ 센서 오류";

// 액추에이터 메시지
const char* const ACTUATOR_ON_MSG = "🔛 액추에이터 켜짐";
const char* const ACTUATOR_OFF_MSG = "🔲 액추에이터 꺼짐";
const char* const ACTUATOR_ERROR_MSG = "❌ 액추에이터 오류";

// 스케줄 메시지
const char* const SCHEDULE_START_MSG = "⏰ 스케줄 시작";
const char* const SCHEDULE_COMPLETE_MSG = "✅ 스케줄 완료";
const char* const SCHEDULE_SKIP_MSG = "⏭️ 스케줄 건너뜀";

// ========== 환경 프로파일 ==========
// 기본 프로파일 (상추)
struct EnvironmentProfile {
    float tempMin;
    float tempMax;
    float humidityMin;
    float humidityMax;
    int lightMin;
    int lightMax;
    int ledOnHour;
    int ledOffHour;
    int wateringHour1;
    int wateringHour2;
    unsigned long wateringDuration;
};

// 기본 상추 프로파일
const EnvironmentProfile DEFAULT_LETTUCE_PROFILE = {
    OPTIMAL_TEMP_MIN,      // tempMin
    OPTIMAL_TEMP_MAX,      // tempMax
    OPTIMAL_HUMIDITY_MIN,  // humidityMin
    OPTIMAL_HUMIDITY_MAX,  // humidityMax
    OPTIMAL_LIGHT_MIN,     // lightMin
    OPTIMAL_LIGHT_MAX,     // lightMax
    LED_ON_HOUR,          // ledOnHour
    LED_OFF_HOUR,         // ledOffHour
    WATERING_HOUR_1,      // wateringHour1
    WATERING_HOUR_2,      // wateringHour2
    WATERING_DURATION     // wateringDuration
};

// ========== 하드웨어 검증 ==========
// 핀 번호 유효성 검사
#if DHT_PIN < 0 || DHT_PIN > 13
    #error "DHT_PIN이 유효하지 않습니다"
#endif

#if LIGHT_SENSOR_PIN < A0 || LIGHT_SENSOR_PIN > A5
    #error "LIGHT_SENSOR_PIN이 유효하지 않습니다"
#endif

// PWM 핀 확인 (Arduino Uno: 3, 5, 6, 9, 10, 11)
#if LED_PIN != 3 && LED_PIN != 5 && LED_PIN != 6 && LED_PIN != 9 && LED_PIN != 10 && LED_PIN != 11
    #warning "LED_PIN이 PWM 핀이 아닙니다. 밝기 조절이 불가능할 수 있습니다."
#endif

// ========== 컴파일 타임 설정 ==========
// 플랫폼 별 설정
#ifdef ARDUINO_AVR_UNO
    #define PLATFORM_NAME "Arduino Uno"
    #define MAX_MEMORY 2048
    #define WATCHDOG_AVAILABLE true
#elif defined(ESP32)
    #define PLATFORM_NAME "ESP32"
    #define MAX_MEMORY 520000
    #define WATCHDOG_AVAILABLE true
    #define WIFI_AVAILABLE true
    #define BLUETOOTH_AVAILABLE true
#else
    #define PLATFORM_NAME "Unknown"
    #define MAX_MEMORY 1024
    #define WATCHDOG_AVAILABLE false
#endif

// ========== 버전 호환성 ==========
#if ARDUINO < 10800
    #error "Arduino IDE 1.8.0 이상이 필요합니다"
#endif

// ========== 기능 플래그 ==========
#define FEATURE_ADAPTIVE_CONTROL true    // 적응형 제어 활성화
#define FEATURE_LEARNING_MODE false      // 학습 모드 (향후 구현)
#define FEATURE_REMOTE_CONTROL true      // 원격 제어 활성화
#define FEATURE_AUTO_BACKUP true         // 자동 백업 활성화
#define FEATURE_POWER_MANAGEMENT true    // 전력 관리 활성화
#define FEATURE_SEASONAL_ADJUSTMENT false // 계절별 조정 (향후 구현)

// ========== 성능 최적화 설정 ==========
#define OPTIMIZE_MEMORY true             // 메모리 최적화
#define OPTIMIZE_SPEED false             // 속도 최적화 (메모리와 트레이드오프)
#define USE_PROGMEM true                 // PROGMEM 사용 (문자열 플래시 저장)

// ========== 끝 ==========
#endif // CONFIG_H