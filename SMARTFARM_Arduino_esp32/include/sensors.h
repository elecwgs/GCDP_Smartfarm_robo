#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <RTClib.h>
#include "config.h"

/**
 * @brief 센서 관리 클래스
 * 
 * DHT22 온습도 센서, LDR 조도 센서, RTC 모듈을 관리하는 클래스입니다.
 * 센서 데이터 읽기, 유효성 검사, 시간 관리 등의 기능을 제공합니다.
 */
class SensorManager {
private:
    // 센서 객체들
    DHT dht;                    ///< DHT22 온습도 센서 객체
    RTC_DS1307 rtc;            ///< DS1307 RTC 모듈 객체
    
    // 센서 데이터
    float temperature;          ///< 현재 온도 (°C)
    float humidity;            ///< 현재 습도 (%)
    int lightLevel;            ///< 현재 조도 (0-1023)
    DateTime currentTime;      ///< 현재 시간
    
    // 상태 관리
    unsigned long lastReadTime;    ///< 마지막 센서 읽기 시간
    unsigned long lastRTCRead;     ///< 마지막 RTC 읽기 시간
    int consecutiveErrors;         ///< 연속 오류 횟수
    bool isInitialized;           ///< 초기화 상태
    
    // 오류 추적
    bool dhtError;                ///< DHT 센서 오류 상태
    bool rtcError;                ///< RTC 모듈 오류 상태
    bool lightSensorError;        ///< 조도 센서 오류 상태
    
    // 통계 데이터 (선택사항)
    float tempSum;                ///< 온도 합계 (평균 계산용)
    float humiditySum;            ///< 습도 합계 (평균 계산용)
    long lightSum;                ///< 조도 합계 (평균 계산용)
    int readingCount;             ///< 읽기 횟수
    
public:
    /**
     * @brief 생성자
     */
    SensorManager();
    
    /**
     * @brief 소멸자
     */
    ~SensorManager();
    
    // ========== 초기화 및 설정 ==========
    /**
     * @brief 센서 매니저 초기화
     * @return true 성공, false 실패
     */
    bool init();
    
    /**
     * @brief 센서 재초기화 (오류 복구용)
     * @return true 성공, false 실패
     */
    bool reinitialize();
    
    /**
     * @brief RTC 시간 수동 설정
     * @param year 년도
     * @param month 월 (1-12)
     * @param day 일 (1-31)
     * @param hour 시간 (0-23)
     * @param minute 분 (0-59)
     * @param second 초 (0-59)
     * @return true 성공, false 실패
     */
    bool setRTCTime(int year, int month, int day, int hour, int minute, int second);
    
    // ========== 데이터 읽기 ==========
    /**
     * @brief 모든 센서 데이터 읽기
     * DHT22 온습도 센서와 조도 센서의 데이터를 읽습니다.
     */
    void readSensors();
    
    /**
     * @brief RTC 시간 읽기
     */
    void readRTC();
    
    /**
     * @brief DHT 센서만 읽기
     * @return true 성공, false 실패
     */
    bool readDHT();
    
    /**
     * @brief 조도 센서만 읽기
     * @return true 성공, false 실패
     */
    bool readLightSensor();
    
    /**
     * @brief 강제로 센서 읽기 (타이머 무시)
     */
    void forceRead();
    
    // ========== 데이터 접근자 (Getter) ==========
    /**
     * @brief 현재 온도 반환
     * @return 온도 (°C)
     */
    float getTemperature() const { return temperature; }
    
    /**
     * @brief 현재 습도 반환
     * @return 습도 (%)
     */
    float getHumidity() const { return humidity; }
    
    /**
     * @brief 현재 조도 반환
     * @return 조도 (0-1023)
     */
    int getLightLevel() const { return lightLevel; }
    
    /**
     * @brief 현재 시간 반환
     * @return DateTime 객체
     */
    DateTime getCurrentTime() const { return currentTime; }
    
    /**
     * @brief 화씨 온도 반환
     * @return 온도 (°F)
     */
    float getTemperatureFahrenheit() const;
    
    /**
     * @brief 절대 습도 반환 (g/m³)
     * @return 절대 습도
     */
    float getAbsoluteHumidity() const;
    
    /**
     * @brief 이슬점 온도 반환
     * @return 이슬점 온도 (°C)
     */
    float getDewPoint() const;
    
    /**
     * @brief 열 지수 반환 (체감 온도)
     * @return 열 지수 (°C)
     */
    float getHeatIndex() const;
    
    // ========== 평균값 및 통계 ==========
    /**
     * @brief 평균 온도 반환
     * @return 평균 온도 (°C)
     */
    float getAverageTemperature() const;
    
    /**
     * @brief 평균 습도 반환
     * @return 평균 습도 (%)
     */
    float getAverageHumidity() const;
    
    /**
     * @brief 평균 조도 반환
     * @return 평균 조도
     */
    int getAverageLight() const;
    
    /**
     * @brief 통계 데이터 리셋
     */
    void resetStatistics();
    
    /**
     * @brief 읽기 횟수 반환
     * @return 총 읽기 횟수
     */
    int getReadingCount() const { return readingCount; }
    
    // ========== 유효성 검사 ==========
    /**
     * @brief 센서 데이터 유효성 확인
     * @return true 유효, false 무효
     */
    bool isValidReading() const;
    
    /**
     * @brief 온도 데이터 유효성 확인
     * @return true 유효, false 무효
     */
    bool isValidTemperature() const;
    
    /**
     * @brief 습도 데이터 유효성 확인
     * @return true 유효, false 무효
     */
    bool isValidHumidity() const;
    
    /**
     * @brief 조도 데이터 유효성 확인
     * @return true 유효, false 무효
     */
    bool isValidLightLevel() const;
    
    /**
     * @brief RTC 시간 유효성 확인
     * @return true 유효, false 무효
     */
    bool isValidRTCTime() const;
    
    // ========== 상태 확인 ==========
    /**
     * @brief 초기화 상태 확인
     * @return true 초기화됨, false 미초기화
     */
    bool isInitializedSuccessfully() const { return isInitialized; }
    
    /**
     * @brief DHT 센서 오류 상태 확인
     * @return true 오류, false 정상
     */
    bool isDHTError() const { return dhtError; }
    
    /**
     * @brief RTC 모듈 오류 상태 확인
     * @return true 오류, false 정상
     */
    bool isRTCError() const { return rtcError; }
    
    /**
     * @brief 조도 센서 오류 상태 확인
     * @return true 오류, false 정상
     */
    bool isLightSensorError() const { return lightSensorError; }
    
    /**
     * @brief 연속 오류 횟수 반환
     * @return 연속 오류 횟수
     */
    int getConsecutiveErrors() const { return consecutiveErrors; }
    
    /**
     * @brief 전체 시스템 상태 확인
     * @return SystemState 열거형 값
     */
    SystemState getSystemState() const;
    
    // ========== 시간 포맷팅 ==========
    /**
     * @brief 시간을 문자열로 포맷팅 (HH:MM:SS)
     * @param dt DateTime 객체
     * @return 포맷된 시간 문자열
     */
    String formatTime(DateTime dt) const;
    
    /**
     * @brief 날짜를 문자열로 포맷팅 (YYYY-MM-DD)
     * @param dt DateTime 객체
     * @return 포맷된 날짜 문자열
     */
    String formatDate(DateTime dt) const;
    
    /**
     * @brief 날짜와 시간을 문자열로 포맷팅
     * @param dt DateTime 객체
     * @return 포맷된 날짜시간 문자열
     */
    String formatDateTime(DateTime dt) const;
    
    /**
     * @brief Unix 타임스탬프로 변환
     * @param dt DateTime 객체
     * @return Unix 타임스탬프
     */
    unsigned long getUnixTimestamp(DateTime dt) const;
    
    // ========== 환경 상태 평가 ==========
    /**
     * @brief 온도 상태 평가
     * @return EnvironmentStatus 열거형 값
     */
    EnvironmentStatus evaluateTemperatureStatus() const;
    
    /**
     * @brief 습도 상태 평가
     * @return EnvironmentStatus 열거형 값
     */
    EnvironmentStatus evaluateHumidityStatus() const;
    
    /**
     * @brief 조도 상태 평가
     * @return EnvironmentStatus 열거형 값
     */
    EnvironmentStatus evaluateLightStatus() const;
    
    /**
     * @brief 전체 환경 상태 평가
     * @return EnvironmentStatus 열거형 값
     */
    EnvironmentStatus evaluateOverallStatus() const;
    
    // ========== 캘리브레이션 ==========
    /**
     * @brief 센서 캘리브레이션 수행
     * @return true 성공, false 실패
     */
    bool calibrateSensors();
    
    /**
     * @brief 조도 센서 캘리브레이션 (어둠)
     * @return 어둠 상태의 조도 값
     */
    int calibrateLightSensorDark();
    
    /**
     * @brief 조도 센서 캘리브레이션 (밝음)
     * @return 밝음 상태의 조도 값
     */
    int calibrateLightSensorBright();
    
    // ========== 디버그 및 진단 ==========
    /**
     * @brief 센서 진단 정보 출력
     */
    void printDiagnostics() const;
    
    /**
     * @brief 센서 상태 요약 출력
     */
    void printSensorStatus() const;
    
    /**
     * @brief 오류 정보 출력
     */
    void printErrorInfo() const;
    
    /**
     * @brief 통계 정보 출력
     */
    void printStatistics() const;
    
    // ========== 저전력 모드 ==========
    /**
     * @brief 저전력 모드 진입
     */
    void enterLowPowerMode();
    
    /**
     * @brief 저전력 모드 해제
     */
    void exitLowPowerMode();
    
    /**
     * @brief 저전력 모드 상태 확인
     * @return true 저전력 모드, false 일반 모드
     */
    bool isLowPowerMode() const;
    
private:
    // ========== 내부 헬퍼 함수들 ==========
    /**
     * @brief 센서 오류 처리
     * @param sensorType 센서 타입
     */
    void handleSensorError(SensorType sensorType);
    
    /**
     * @brief 오류 카운터 증가
     */
    void incrementErrorCounter();
    
    /**
     * @brief 오류 카운터 리셋
     */
    void resetErrorCounter();
    
    /**
     * @brief DHT 센서 상태 확인
     * @return true 정상, false 오류
     */
    bool checkDHTStatus();
    
    /**
     * @brief RTC 배터리 상태 확인
     * @return true 정상, false 저전압
     */
    bool checkRTCBattery();
    
    /**
     * @brief 센서 데이터 필터링 (노이즈 제거)
     * @param newValue 새로운 값
     * @param oldValue 이전 값
     * @param threshold 변화 임계값
     * @return 필터링된 값
     */
    float filterSensorData(float newValue, float oldValue, float threshold);
    
    /**
     * @brief 조도 센서 데이터 스무딩
     * @param readings 읽기 값들의 배열
     * @param count 배열 크기
     * @return 스무딩된 값
     */
    int smoothLightData(int readings[], int count);
    
    // ========== 저전력 관련 ==========
    bool lowPowerMode;             ///< 저전력 모드 상태
    unsigned long lastActivity;   ///< 마지막 활동 시간
    
    // ========== 캘리브레이션 데이터 ==========
    int lightSensorMin;           ///< 조도 센서 최소값 (어둠)
    int lightSensorMax;           ///< 조도 센서 최대값 (밝음)
    float temperatureOffset;      ///< 온도 센서 오프셋
    float humidityOffset;         ///< 습도 센서 오프셋
    bool isCalibrated;           ///< 캘리브레이션 완료 상태
};

#endif // SENSORS_H