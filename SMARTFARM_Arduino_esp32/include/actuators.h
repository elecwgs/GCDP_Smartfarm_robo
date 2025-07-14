#ifndef ACTUATORS_H
#define ACTUATORS_H

#include <Arduino.h>
#include <Stepper.h>
#include "config.h"

/**
 * @brief 액추에이터 관리 클래스
 * 
 * 워터펌프, 팬, LED 조명, 스텝모터 등 모든 액추에이터를 관리하는 클래스입니다.
 * 안전한 제어, 상태 모니터링, 오류 처리 등의 기능을 제공합니다.
 */
class ActuatorManager {
private:
    // 하드웨어 객체
    Stepper stepper;                ///< 스텝모터 객체
    
    // 액추에이터 상태
    bool pumpStatus;               ///< 워터펌프 상태 (true: ON, false: OFF)
    bool fanStatus;                ///< 팬 상태 (true: ON, false: OFF)
    bool ledStatus;                ///< LED 조명 상태 (true: ON, false: OFF)
    int ledBrightness;            ///< LED 밝기 (0-255, PWM 제어용)
    
    // 물주기 관리
    bool wateringActive;           ///< 물주기 진행 상태
    unsigned long wateringStartTime; ///< 물주기 시작 시간
    unsigned long wateringDuration; ///< 물주기 지속 시간
    int wateringCycles;           ///< 오늘 물주기 횟수
    
    // 스텝모터 관리
    int currentPosition;          ///< 현재 스텝모터 위치
    int targetPosition;           ///< 목표 스텝모터 위치
    bool stepperMoving;          ///< 스텝모터 이동 중 여부
    bool stepperEnabled;         ///< 스텝모터 활성화 상태
    
    // 안전 및 오류 관리
    bool emergencyMode;          ///< 비상 모드 상태
    bool safetyLock;            ///< 안전 잠금 상태
    unsigned long lastActivityTime; ///< 마지막 동작 시간
    int errorCount[4];          ///< 각 액추에이터별 오류 횟수 [pump, fan, led, stepper]
    
    // 상태 모니터링
    bool isInitialized;         ///< 초기화 완료 상태
    unsigned long totalRunTime[4]; ///< 각 액추에이터 총 가동 시간
    unsigned long lastRunTime[4];  ///< 각 액추에이터 마지막 가동 시간
    
    // PWM 제어 (LED 밝기 조절용)
    bool pwmEnabled;            ///< PWM 제어 활성화 여부
    int fadeSpeed;              ///< LED 페이드 속도
    
    // 자동 제어 설정
    bool autoControlEnabled;    ///< 자동 제어 활성화
    ControlMode controlMode;    ///< 현재 제어 모드

public:
    /**
     * @brief 생성자
     */
    ActuatorManager();
    
    /**
     * @brief 소멸자
     */
    // ~ActuatorManager();
    
    // ========== 초기화 및 설정 ==========
    /**
     * @brief 액추에이터 매니저 초기화
     * @return true 성공, false 실패
     */
    bool init();
    
    /**
     * @brief 모든 액추에이터 리셋 (OFF 상태로)
     */
    void resetAll();
    
    /**
     * @brief 제어 모드 설정
     * @param mode 제어 모드
     */
    void setControlMode(ControlMode mode);
    
    /**
     * @brief 안전 잠금 설정/해제
     * @param lock true: 잠금, false: 해제
     */
    void setSafetyLock(bool lock);
    
    // ========== 워터펌프 제어 ==========
    /**
     * @brief 워터펌프 제어
     * @param activate true: 켜기, false: 끄기
     */
    void activatePumps(bool activate);
    
    /**
     * @brief 개별 워터펌프 제어
     * @param pumpNumber 펌프 번호 (1 또는 2)
     * @param activate true: 켜기, false: 끄기
     */
    void activatePump(int pumpNumber, bool activate);
    
    /**
     * @brief 정시 물주기 시작
     * @param duration 물주기 지속 시간 (ms), 0이면 기본값 사용
     */
    void startWatering(unsigned long duration = 0);
    
    /**
     * @brief 물주기 중지
     */
    void stopWatering();
    
    /**
     * @brief 긴급 물주기 (습도 부족 시)
     * @param duration 물주기 지속 시간 (ms)
     */
    void emergencyWatering(unsigned long duration);
    
    /**
     * @brief 물주기 지속 시간 확인 및 자동 중지
     * @return true: 물주기 완료됨, false: 진행 중 또는 비활성
     */
    bool checkWateringDuration();
    
    // ========== 팬 제어 ==========
    /**
     * @brief 모든 팬 제어
     * @param activate true: 켜기, false: 끄기
     */
    void activateFans(bool activate);
    
    /**
     * @brief 개별 팬 제어
     * @param fanNumber 팬 번호 (1-4)
     * @param activate true: 켜기, false: 끄기
     */
    void activateFan(int fanNumber, bool activate);
    
    /**
     * @brief 팬 속도 제어 (PWM 사용 시)
     * @param speed 속도 (0-255)
     */
    void setFanSpeed(int speed);
    
    /**
     * @brief 순환 팬 제어 (교대로 동작)
     * @param interval 교대 간격 (ms)
     */
    void activateRotatingFans(unsigned long interval);
    
    // ========== LED 조명 제어 ==========
    /**
     * @brief LED 조명 제어
     * @param activate true: 켜기, false: 끄기
     */
    void activateLED(bool activate);
    
    /**
     * @brief LED 밝기 제어 (PWM)
     * @param brightness 밝기 (0-255)
     */
    void setLEDBrightness(int brightness);
    
    /**
     * @brief LED 서서히 켜기/끄기 (페이드 효과)
     * @param targetBrightness 목표 밝기 (0-255)
     * @param fadeTime 페이드 시간 (ms)
     */
    void fadeLED(int targetBrightness, unsigned long fadeTime);
    
    /**
     * @brief LED 깜빡임 효과
     * @param count 깜빡임 횟수
     * @param interval 깜빡임 간격 (ms)
     */
    void blinkLED(int count, unsigned long interval);
    
    /**
     * @brief 일몰/일출 시뮬레이션
     * @param sunrise true: 일출, false: 일몰
     * @param duration 전환 시간 (ms)
     */
    void simulateSunriseSunset(bool sunrise, unsigned long duration);
    
    // ========== 스텝모터 제어 ==========
    /**
     * @brief 스텝모터 활성화/비활성화
     * @param enable true: 활성화, false: 비활성화
     */
    void enableStepper(bool enable);
    
    /**
     * @brief 스텝모터 속도 설정
     * @param rpm 분당 회전수
     */
    void setStepperSpeed(int rpm);
    
    /**
     * @brief 상대 위치로 이동
     * @param steps 이동할 스텝 수 (음수: 반대 방향)
     */
    void moveStepperRelative(int steps);
    
    /**
     * @brief 절대 위치로 이동
     * @param position 목표 위치
     */
    void moveStepperAbsolute(int position);
    
    /**
     * @brief 정해진 패턴으로 식물 이동
     */
    void moveLinearActuator();
    
    /**
     * @brief 홈 위치로 복귀
     */
    void returnToHome();
    
    /**
     * @brief 현재 위치를 홈으로 설정
     */
    void setHomePosition();
    
    // ========== 상태 확인 함수들 ==========
    /**
     * @brief 워터펌프 상태 확인
     * @return true: 작동 중, false: 정지
     */
    bool isPumpActive() const { return pumpStatus; }
    
    /**
     * @brief 팬 상태 확인
     * @return true: 작동 중, false: 정지
     */
    bool isFanActive() const { return fanStatus; }
    
    /**
     * @brief LED 조명 상태 확인
     * @return true: 켜짐, false: 꺼짐
     */
    bool isLEDActive() const { return ledStatus; }
    
    /**
     * @brief LED 밝기 확인
     * @return 현재 밝기 (0-255)
     */
    int getLEDBrightness() const { return ledBrightness; }
    
    /**
     * @brief 물주기 진행 상태 확인
     * @return true: 진행 중, false: 정지
     */
    bool isWateringActive() const { return wateringActive; }
    
    /**
     * @brief 스텝모터 이동 상태 확인
     * @return true: 이동 중, false: 정지
     */
    bool isStepperMoving() const { return stepperMoving; }
    
    /**
     * @brief 현재 스텝모터 위치 확인
     * @return 현재 위치 (스텝 수)
     */
    int getCurrentPosition() const { return currentPosition; }
    
    /**
     * @brief 비상 모드 상태 확인
     * @return true: 비상 모드, false: 정상 모드
     */
    bool isEmergencyMode() const { return emergencyMode; }
    
    /**
     * @brief 안전 잠금 상태 확인
     * @return true: 잠금, false: 해제
     */
    bool isSafetyLocked() const { return safetyLock; }
    
    /**
     * @brief 오늘 물주기 횟수 확인
     * @return 물주기 횟수
     */
    int getWateringCycles() const { return wateringCycles; }
    
    // ========== 통계 및 모니터링 ==========
    /**
     * @brief 액추에이터 가동 시간 확인
     * @param actuatorType 액추에이터 타입
     * @return 총 가동 시간 (ms)
     */
    unsigned long getRunTime(ActuatorType actuatorType) const;
    
    /**
     * @brief 액추에이터 오류 횟수 확인
     * @param actuatorType 액추에이터 타입
     * @return 오류 횟수
     */
    int getErrorCount(ActuatorType actuatorType) const;
    
    /**
     * @brief 모든 액추에이터 상태 요약
     * @return 상태 문자열
     */
    String getStatusSummary() const;
    
    /**
     * @brief 물주기 통계 리셋 (새로운 날)
     */
    void resetDailyStats();
    
    // ========== 안전 및 보호 기능 ==========
    /**
     * @brief 비상 정지 (모든 액추에이터 즉시 중지)
     */
    void emergencyStop();
    
    /**
     * @brief 안전 체크 수행
     * @return true: 안전, false: 위험
     */
    bool performSafetyCheck();
    
    /**
     * @brief 과부하 보호 확인
     * @param actuatorType 확인할 액추에이터 타입
     * @return true: 정상, false: 과부하
     */
    bool checkOverload(ActuatorType actuatorType);
    
    /**
     * @brief 자동 복구 시도
     * @param actuatorType 복구할 액추에이터 타입
     * @return true: 성공, false: 실패
     */
    bool attemptRecovery(ActuatorType actuatorType);
    
    // ========== 자동 제어 ==========
    /**
     * @brief 자동 제어 활성화/비활성화
     * @param enable true: 활성화, false: 비활성화
     */
    void setAutoControl(bool enable);
    
    /**
     * @brief 온도 기반 자동 팬 제어
     * @param temperature 현재 온도
     */
    void autoFanControl(float temperature);
    
    /**
     * @brief 습도 기반 자동 물주기 제어
     * @param humidity 현재 습도
     */
    void autoWateringControl(float humidity);
    
    /**
     * @brief 조도 기반 자동 LED 제어
     * @param lightLevel 현재 조도
     * @param currentHour 현재 시간
     */
    void autoLEDControl(int lightLevel, int currentHour);
    
    // ========== 유지보수 및 진단 ==========
    /**
     * @brief 액추에이터 진단 수행
     * @return true: 정상, false: 문제 발견
     */
    bool performDiagnostics();
    
    /**
     * @brief 개별 액추에이터 테스트
     * @param actuatorType 테스트할 액추에이터 타입
     * @return true: 정상, false: 오류
     */
    bool testActuator(ActuatorType actuatorType);
    
    /**
     * @brief 전체 시스템 테스트 시퀀스
     */
    void runSystemTest();
    
    /**
     * @brief 액추에이터 상태 출력
     */
    void printStatus();
    
    /**
     * @brief 상세 진단 정보 출력
     */
    void printDiagnostics();
    
    // ========== 설정 및 캘리브레이션 ==========
    /**
     * @brief 워터펌프 유량 캘리브레이션
     * @param testDuration 테스트 시간 (ms)
     * @return 분당 유량 (ml/min)
     */
    float calibratePumpFlow(unsigned long testDuration);
    
    /**
     * @brief 스텝모터 캘리브레이션
     * @return true: 성공, false: 실패
     */
    bool calibrateStepper();
    
    /**
     * @brief LED 밝기 캘리브레이션
     */
    void calibrateLED();
    
    // ========== 고급 제어 기능 ==========
    /**
     * @brief 시퀀스 제어 (여러 액추에이터 순차 동작)
     * @param sequence 동작 시퀀스 배열
     * @param length 시퀀스 길이
     */
    void executeSequence(const int sequence[], int length);
    
    /**
     * @brief 타이머 기반 제어
     * @param actuatorType 액추에이터 타입
     * @param duration 동작 시간 (ms)
     */
    void timedControl(ActuatorType actuatorType, unsigned long duration);
    
    /**
     * @brief 조건부 제어 (특정 조건 만족 시 동작)
     * @param condition 조건 함수 포인터
     * @param action 실행할 동작
     */
    void conditionalControl(bool (*condition)(), void (ActuatorManager::*action)());
    
private:
    // ========== 내부 헬퍼 함수들 ==========
    /**
     * @brief 핀 상태 설정
     * @param pin 핀 번호
     * @param state 상태 (HIGH/LOW)
     */
    void setPinState(int pin, int state);
    
    /**
     * @brief PWM 출력 설정
     * @param pin 핀 번호
     * @param value PWM 값 (0-255)
     */
    void setPWMOutput(int pin, int value);
    
    /**
     * @brief 오류 처리
     * @param actuatorType 오류 발생 액추에이터
     * @param errorCode 오류 코드
     */
    void handleError(ActuatorType actuatorType, int errorCode);
    
    /**
     * @brief 가동 시간 업데이트
     * @param actuatorType 액추에이터 타입
     * @param isActive 현재 동작 상태
     */
    void updateRunTime(ActuatorType actuatorType, bool isActive);
    
    /**
     * @brief 액추에이터 상태 검증
     * @param actuatorType 액추에이터 타입
     * @return true: 정상, false: 오류
     */
    bool validateActuatorState(ActuatorType actuatorType);
    
    /**
     * @brief 전력 소비량 모니터링
     * @return 현재 전력 소비량 (추정값)
     */
    float estimatePowerConsumption();
    
    /**
     * @brief 스텝모터 위치 업데이트
     * @param steps 이동한 스텝 수
     */
    void updateStepperPosition(int steps);
    
    /**
     * @brief 비동기 스텝모터 제어
     */
    void asyncStepperControl();
    
    // ========== 타이머 및 스케줄링 ==========
    unsigned long lastUpdate;          ///< 마지막 업데이트 시간
    unsigned long fanRotationTimer;    ///< 팬 교대 타이머
    unsigned long ledFadeTimer;        ///< LED 페이드 타이머
    unsigned long stepperMoveTimer;    ///< 스텝모터 이동 타이머
    
    // ========== 설정값들 ==========
    int maxPumpRunTime;               ///< 최대 펌프 연속 가동 시간
    int maxFanRunTime;                ///< 최대 팬 연속 가동 시간
    int stepperAcceleration;          ///< 스텝모터 가속도
    float pumpFlowRate;               ///< 펌프 유량 (ml/min)
    bool maintenanceMode;             ///< 유지보수 모드
    
    // ========== 보호 기능 ==========
    unsigned long pumpProtectionTimer; ///< 펌프 보호 타이머
    unsigned long fanProtectionTimer;  ///< 팬 보호 타이머
    bool thermalProtection;           ///< 열 보호 활성화
    int maxConcurrentActuators;       ///< 동시 동작 액추에이터 최대 수
};

#endif // ACTUATORS_H