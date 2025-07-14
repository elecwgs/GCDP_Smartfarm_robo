#ifndef SMARTFARM_H
#define SMARTFARM_H

#include <Arduino.h>
#include "config.h"
#include "sensors.h"
#include "actuators.h"
#include "scheduler.h"
#include "communication.h"

/**
 * @brief 시스템 통계 구조체
 * 
 * 시스템 운영 통계를 저장하는 구조체입니다.
 */
struct SystemStats {
    unsigned long uptime;              ///< 시스템 가동 시간 (ms)
    unsigned long totalCycles;        ///< 총 루프 실행 횟수
    int successfulOperations;         ///< 성공한 작업 수
    int failedOperations;            ///< 실패한 작업 수
    float averageLoopTime;           ///< 평균 루프 실행 시간 (ms)
    float maxLoopTime;               ///< 최대 루프 실행 시간 (ms)
    int memoryUsage;                 ///< 메모리 사용량 (bytes)
    int freeMemory;                  ///< 여유 메모리 (bytes)
    unsigned long lastResetTime;     ///< 마지막 리셋 시간
    String resetReason;              ///< 리셋 이유
};

/**
 * @brief 시스템 건강 상태 구조체
 */
struct SystemHealth {
    bool sensorsHealthy;             ///< 센서 건강 상태
    bool actuatorsHealthy;           ///< 액추에이터 건강 상태
    bool communicationHealthy;       ///< 통신 건강 상태
    bool schedulerHealthy;           ///< 스케줄러 건강 상태
    int overallHealthScore;          ///< 전체 건강 점수 (0-100)
    String healthReport;             ///< 건강 상태 보고서
    unsigned long lastHealthCheck;   ///< 마지막 건강 체크 시간
};

/**
 * @brief 스마트팜 메인 시스템 클래스
 * 
 * 모든 서브시스템을 통합 관리하는 메인 클래스입니다.
 * 센서 관리, 액추에이터 제어, 스케줄링, 통신 등 모든 기능을 조율합니다.
 */
class SmartFarm {
private:
    // 서브시스템 매니저들
    SensorManager sensors;              ///< 센서 매니저
    ActuatorManager actuators;          ///< 액추에이터 매니저
    Scheduler scheduler;                ///< 스케줄러
    CommunicationManager communication; ///< 통신 매니저
    
    // 시스템 상태
    SystemState currentState;           ///< 현재 시스템 상태
    ControlMode operationMode;          ///< 운영 모드
    bool systemInitialized;            ///< 시스템 초기화 완료
    bool emergencyMode;                ///< 비상 모드 상태
    bool maintenanceMode;              ///< 유지보수 모드 상태
    
    // 타이머 관리
    unsigned long lastSensorRead;       ///< 마지막 센서 읽기 시간
    unsigned long lastESP32Send;        ///< 마지막 ESP32 전송 시간
    unsigned long lastRTCRead;          ///< 마지막 RTC 읽기 시간
    unsigned long lastStatusPrint;      ///< 마지막 상태 출력 시간
    unsigned long lastHealthCheck;      ///< 마지막 건강 체크 시간
    unsigned long systemStartTime;      ///< 시스템 시작 시간
    
    // 성능 모니터링
    unsigned long loopStartTime;        ///< 루프 시작 시간
    unsigned long maxLoopDuration;      ///< 최대 루프 실행 시간
    unsigned long totalLoopTime;        ///< 총 루프 시간
    unsigned long loopCount;            ///< 루프 실행 횟수
    
    // 오류 관리
    int consecutiveErrors;             ///< 연속 오류 횟수
    unsigned long lastErrorTime;       ///< 마지막 오류 시간
    String lastErrorMessage;           ///< 마지막 오류 메시지
    bool autoRecoveryEnabled;          ///< 자동 복구 활성화
    int recoveryAttempts;              ///< 복구 시도 횟수
    
    // 시스템 통계 및 건강 상태
    SystemStats stats;                 ///< 시스템 통계
    SystemHealth health;               ///< 시스템 건강 상태
    
    // 설정 및 프로파일
    EnvironmentProfile currentProfile;  ///< 현재 환경 프로파일
    bool adaptiveControlEnabled;       ///< 적응형 제어 활성화
    
    // 보안 및 안전
    bool safetySystemEnabled;          ///< 안전 시스템 활성화
    unsigned long lastSecurityCheck;   ///< 마지막 보안 체크 시간
    String securityToken;              ///< 보안 토큰
    
public:
    /**
     * @brief 생성자
     */
    SmartFarm();
    
    /**
     * @brief 소멸자
     */
    ~SmartFarm();
    
    // ========== 시스템 초기화 및 설정 ==========
    /**
     * @brief 시스템 초기화
     * @return true 성공, false 실패
     */
    bool init();
    
    /**
     * @brief 시스템 재초기화
     * @return true 성공, false 실패
     */
    bool reinitialize();
    
    /**
     * @brief 공장 초기화 (모든 설정 리셋)
     * @return true 성공, false 실패
     */
    bool factoryReset();
    
    /**
     * @brief 시스템 설정 로드
     * @return true 성공, false 실패
     */
    bool loadConfiguration();
    
    /**
     * @brief 시스템 설정 저장
     * @return true 성공, false 실패
     */
    bool saveConfiguration();
    
    /**
     * @brief 환경 프로파일 설정
     * @param profile 환경 프로파일
     */
    void setEnvironmentProfile(const EnvironmentProfile& profile);
    
    /**
     * @brief 운영 모드 설정
     * @param mode 운영 모드
     */
    void setOperationMode(ControlMode mode);
    
    // ========== 메인 실행 루프 ==========
    /**
     * @brief 메인 시스템 루프 (Arduino loop에서 호출)
     */
    void run();
    
    /**
     * @brief 단일 사이클 실행 (테스트용)
     * @return 실행 시간 (ms)
     */
    unsigned long runSingleCycle();
    
    /**
     * @brief 시스템 업데이트 (내부 호출)
     */
    void update();
    
    // ========== 상태 관리 ==========
    /**
     * @brief 현재 시스템 상태 확인
     * @return 시스템 상태
     */
    SystemState getCurrentState() const { return currentState; }
    
    /**
     * @brief 시스템 상태 변경
     * @param newState 새로운 상태
     */
    void setState(SystemState newState);
    
    /**
     * @brief 시스템 초기화 완료 여부
     * @return true: 초기화 완료, false: 미완료
     */
    bool isInitialized() const { return systemInitialized; }
    
    /**
     * @brief 비상 모드 상태 확인
     * @return true: 비상 모드, false: 정상 모드
     */
    bool isEmergencyMode() const { return emergencyMode; }
    
    /**
     * @brief 유지보수 모드 상태 확인
     * @return true: 유지보수 모드, false: 정상 모드
     */
    bool isMaintenanceMode() const { return maintenanceMode; }
    
    /**
     * @brief 시스템 가동 시간 확인
     * @return 가동 시간 (ms)
     */
    unsigned long getUptime() const;
    
    // ========== 서브시스템 접근자 ==========
    /**
     * @brief 센서 매니저 참조 반환
     * @return 센서 매니저 참조
     */
    SensorManager& getSensors() { return sensors; }
    
    /**
     * @brief 액추에이터 매니저 참조 반환
     * @return 액추에이터 매니저 참조
     */
    ActuatorManager& getActuators() { return actuators; }
    
    /**
     * @brief 스케줄러 참조 반환
     * @return 스케줄러 참조
     */
    Scheduler& getScheduler() { return scheduler; }
    
    /**
     * @brief 통신 매니저 참조 반환
     * @return 통신 매니저 참조
     */
    CommunicationManager& getCommunication() { return communication; }
    
    // ========== 비상 및 안전 기능 ==========
    /**
     * @brief 비상 정지 (모든 시스템 즉시 중지)
     */
    void emergencyStop();
    
    /**
     * @brief 비상 모드 진입
     * @param reason 비상 상황 원인
     */
    void enterEmergencyMode(const String& reason);
    
    /**
     * @brief 비상 모드 해제
     */
    void exitEmergencyMode();
    
    /**
     * @brief 유지보수 모드 진입
     */
    void enterMaintenanceMode();
    
    /**
     * @brief 유지보수 모드 해제
     */
    void exitMaintenanceMode();
    
    /**
     * @brief 안전 체크 수행
     * @return true: 안전, false: 위험
     */
    bool performSafetyCheck();
    
    /**
     * @brief 시스템 무결성 검사
     * @return true: 정상, false: 문제 발견
     */
    bool checkSystemIntegrity();
    
    // ========== 진단 및 모니터링 ==========
    /**
     * @brief 시스템 진단 수행
     * @return true: 정상, false: 문제 발견
     */
    bool performDiagnostics();
    
    /**
     * @brief 건강 체크 수행
     * @return 건강 점수 (0-100)
     */
    int performHealthCheck();
    
    /**
     * @brief 성능 분석 수행
     * @return 성능 보고서 JSON
     */
    String performPerformanceAnalysis();
    
    /**
     * @brief 메모리 사용량 분석
     * @return 메모리 사용량 (bytes)
     */
    int analyzeMemoryUsage();
    
    /**
     * @brief 시스템 로그 분석
     * @return 로그 분석 결과
     */
    String analyzeSystemLogs();
    
    // ========== 통계 및 보고서 ==========
    /**
     * @brief 시스템 통계 확인
     * @return 시스템 통계 구조체
     */
    SystemStats getSystemStats() const { return stats; }
    
    /**
     * @brief 시스템 건강 상태 확인
     * @return 시스템 건강 상태 구조체
     */
    SystemHealth getSystemHealth() const { return health; }
    
    /**
     * @brief 일일 보고서 생성
     * @return 일일 보고서 JSON
     */
    String generateDailyReport();
    
    /**
     * @brief 주간 보고서 생성
     * @return 주간 보고서 JSON
     */
    String generateWeeklyReport();
    
    /**
     * @brief 성능 보고서 생성
     * @return 성능 보고서 JSON
     */
    String generatePerformanceReport();
    
    /**
     * @brief 환경 보고서 생성
     * @return 환경 보고서 JSON
     */
    String generateEnvironmentReport();
    
    // ========== 설정 관리 ==========
    /**
     * @brief 설정을 JSON으로 내보내기
     * @return 설정 JSON
     */
    String exportSettings();
    
    /**
     * @brief JSON에서 설정 가져오기
     * @param settingsJSON 설정 JSON
     * @return true: 성공, false: 실패
     */
    bool importSettings(const String& settingsJSON);
    
    /**
     * @brief 설정 백업 생성
     * @return true: 성공, false: 실패
     */
    bool createSettingsBackup();
    
    /**
     * @brief 설정 백업 복원
     * @return true: 성공, false: 실패
     */
    bool restoreSettings