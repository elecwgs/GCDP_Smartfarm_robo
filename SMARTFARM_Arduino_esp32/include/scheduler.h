#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <Arduino.h>
#include <RTClib.h>
#include "config.h"

// 전방 선언
class SensorManager;
class ActuatorManager;

/**
 * @brief 작업 구조체
 * 
 * 스케줄된 작업의 정보를 저장하는 구조체입니다.
 */
struct ScheduledTask {
    int hour;                    ///< 실행 시간 (시)
    int minute;                  ///< 실행 시간 (분)
    ActuatorType actuatorType;   ///< 대상 액추에이터
    bool isActive;              ///< 활성화 상태
    unsigned long duration;      ///< 지속 시간 (ms)
    bool isRepeating;           ///< 반복 실행 여부
    bool isCompleted;           ///< 오늘 실행 완료 여부
    String description;          ///< 작업 설명
    int priority;               ///< 우선순위 (1-10, 높을수록 우선)
};

/**
 * @brief 환경 조건 구조체
 * 
 * 환경 기반 제어를 위한 조건 정보를 저장하는 구조체입니다.
 */
struct EnvironmentCondition {
    SensorType sensorType;      ///< 센서 타입
    float minValue;             ///< 최소값
    float maxValue;             ///< 최대값
    ActuatorType actuatorType;  ///< 대상 액추에이터
    bool actionOnHigh;          ///< 높을 때 동작 여부
    bool actionOnLow;           ///< 낮을 때 동작 여부
    unsigned long cooldownTime; ///< 쿨다운 시간 (ms)
    unsigned long lastAction;   ///< 마지막 동작 시간
    bool isEnabled;             ///< 활성화 상태
};

/**
 * @brief 스케줄러 클래스
 * 
 * 시간 기반 및 환경 기반 자동 제어를 관리하는 클래스입니다.
 * 정시 스케줄링, 조건부 제어, 작업 관리 등의 기능을 제공합니다.
 */
class Scheduler {
private:
    // 일일 작업 완료 상태
    bool todayWatering1Done;        ///< 오늘 1차 물주기 완료
    bool todayWatering2Done;        ///< 오늘 2차 물주기 완료
    bool todayLinearMove1Done;      ///< 오늘 1차 식물이동 완료
    bool todayLinearMove2Done;      ///< 오늘 2차 식물이동 완료
    
    // 날짜 관리
    int lastDay;                    ///< 마지막 확인 날짜
    int lastMonth;                  ///< 마지막 확인 월
    int lastYear;                   ///< 마지막 확인 년
    
    // 스케줄된 작업들
    static const int MAX_SCHEDULED_TASKS = 20;
    ScheduledTask scheduledTasks[MAX_SCHEDULED_TASKS];
    int taskCount;                  ///< 현재 등록된 작업 수
    
    // 환경 조건들
    static const int MAX_ENV_CONDITIONS = 15;
    EnvironmentCondition envConditions[MAX_ENV_CONDITIONS];
    int conditionCount;             ///< 현재 등록된 조건 수
    
    // 제어 모드 및 상태
    ControlMode currentMode;        ///< 현재 제어 모드
    bool schedulerEnabled;          ///< 스케줄러 활성화 상태
    bool timeBasedControlEnabled;   ///< 시간 기반 제어 활성화
    bool environmentControlEnabled; ///< 환경 기반 제어 활성화
    
    // 통계 및 모니터링
    int totalTasksExecuted;         ///< 총 실행된 작업 수
    int totalEnvironmentActions;    ///< 총 환경 기반 동작 수
    unsigned long lastUpdateTime;   ///< 마지막 업데이트 시간
    
    // 안전 및 오류 관리
    int consecutiveFailures;        ///< 연속 실패 횟수
    bool emergencyMode;            ///< 비상 모드 상태
    unsigned long lastEmergencyTime; ///< 마지막 비상 상황 시간
    
    // 적응형 제어 (향후 확장용)
    bool adaptiveControlEnabled;    ///< 적응형 제어 활성화
    float learningRate;            ///< 학습률 (0.0-1.0)
    
public:
    /**
     * @brief 생성자
     */
    Scheduler();
    
    /**
     * @brief 소멸자
     */
    ~Scheduler();
    
    // ========== 초기화 및 설정 ==========
    /**
     * @brief 스케줄러 초기화
     * @return true 성공, false 실패
     */
    bool init();
    
    /**
     * @brief 기본 스케줄 로드
     */
    void loadDefaultSchedule();
    
    /**
     * @brief 기본 환경 조건 로드
     */
    void loadDefaultEnvironmentConditions();
    
    /**
     * @brief 제어 모드 설정
     * @param mode 제어 모드
     */
    void setControlMode(ControlMode mode);
    
    /**
     * @brief 스케줄러 활성화/비활성화
     * @param enabled true: 활성화, false: 비활성화
     */
    void setEnabled(bool enabled);
    
    // ========== 메인 업데이트 함수 ==========
    /**
     * @brief 스케줄러 업데이트 (메인 루프에서 호출)
     * @param currentTime 현재 시간
     */
    void update(const DateTime& currentTime);
    
    /**
     * @brief 시간 기반 제어 실행
     * @param currentTime 현재 시간
     * @param actuators 액추에이터 매니저 참조
     */
    void timeBasedControl(const DateTime& currentTime, ActuatorManager& actuators);
    
    /**
     * @brief 환경 기반 제어 실행
     * @param sensors 센서 매니저 참조
     * @param actuators 액추에이터 매니저 참조
     */
    void environmentalControl(const SensorManager& sensors, ActuatorManager& actuators);
    
    // ========== 작업 관리 ==========
    /**
     * @brief 새로운 작업 추가
     * @param hour 실행 시간 (시)
     * @param minute 실행 시간 (분)
     * @param actuatorType 대상 액추에이터
     * @param duration 지속 시간 (ms)
     * @param description 작업 설명
     * @param priority 우선순위 (1-10)
     * @return 작업 ID (실패 시 -1)
     */
    int addTask(int hour, int minute, ActuatorType actuatorType, 
                unsigned long duration, const String& description, int priority = 5);
    
    /**
     * @brief 작업 제거
     * @param taskId 작업 ID
     * @return true 성공, false 실패
     */
    bool removeTask(int taskId);
    
    /**
     * @brief 작업 활성화/비활성화
     * @param taskId 작업 ID
     * @param active true: 활성화, false: 비활성화
     * @return true 성공, false 실패
     */
    bool setTaskActive(int taskId, bool active);
    
    /**
     * @brief 작업 수정
     * @param taskId 작업 ID
     * @param task 새로운 작업 정보
     * @return true 성공, false 실패
     */
    bool modifyTask(int taskId, const ScheduledTask& task);
    
    /**
     * @brief 모든 작업 리스트 출력
     */
    void printAllTasks();
    
    // ========== 환경 조건 관리 ==========
    /**
     * @brief 환경 조건 추가
     * @param sensorType 센서 타입
     * @param minValue 최소값
     * @param maxValue 최대값
     * @param actuatorType 대상 액추에이터
     * @param actionOnHigh 높을 때 동작 여부
     * @param actionOnLow 낮을 때 동작 여부
     * @param cooldownTime 쿨다운 시간 (ms)
     * @return 조건 ID (실패 시 -1)
     */
    int addEnvironmentCondition(SensorType sensorType, float minValue, float maxValue,
                               ActuatorType actuatorType, bool actionOnHigh, bool actionOnLow,
                               unsigned long cooldownTime = 300000);
    
    /**
     * @brief 환경 조건 제거
     * @param conditionId 조건 ID
     * @return true 성공, false 실패
     */
    bool removeEnvironmentCondition(int conditionId);
    
    /**
     * @brief 환경 조건 활성화/비활성화
     * @param conditionId 조건 ID
     * @param enabled true: 활성화, false: 비활성화
     * @return true 성공, false 실패
     */
    bool setEnvironmentConditionEnabled(int conditionId, bool enabled);
    
    /**
     * @brief 모든 환경 조건 리스트 출력
     */
    void printAllEnvironmentConditions();
    
    // ========== 일일 작업 리셋 ==========
    /**
     * @brief 일일 작업 상태 리셋 (새로운 날 시작 시)
     */
    void resetDailyTasks();
    
    /**
     * @brief 월별 작업 리셋 (새로운 달 시작 시)
     */
    void resetMonthlyTasks();
    
    /**
     * @brief 연간 작업 리셋 (새해 시작 시)
     */
    void resetYearlyTasks();
    
    // ========== 시간 기반 조건 확인 ==========
    /**
     * @brief LED 조명 시간 확인
     * @param currentTime 현재 시간
     * @return true: 조명 시간, false: 소등 시간
     */
    bool isTimeForLED(const DateTime& currentTime) const;
    
    /**
     * @brief 물주기 시간 확인
     * @param currentTime 현재 시간
     * @return true: 물주기 시간, false: 아님
     */
    bool isTimeToWater(const DateTime& currentTime);
    
    /**
     * @brief 식물 이동 시간 확인
     * @param currentTime 현재 시간
     * @return true: 이동 시간, false: 아님
     */
    bool isTimeToMovePlant(const DateTime& currentTime);
    
    /**
     * @brief 야간 모드 시간 확인
     * @param currentTime 현재 시간
     * @return true: 야간, false: 주간
     */
    bool isNightTime(const DateTime& currentTime) const;
    
    /**
     * @brief 특정 작업 실행 시간 확인
     * @param taskId 작업 ID
     * @param currentTime 현재 시간
     * @return true: 실행 시간, false: 아님
     */
    bool isTimeForTask(int taskId, const DateTime& currentTime);
    
    // ========== 상태 확인 함수들 ==========
    /**
     * @brief 1차 물주기 완료 상태
     * @return true: 완료, false: 미완료
     */
    bool isWatering1Done() const { return todayWatering1Done; }
    
    /**
     * @brief 2차 물주기 완료 상태
     * @return true: 완료, false: 미완료
     */
    bool isWatering2Done() const { return todayWatering2Done; }
    
    /**
     * @brief 1차 식물이동 완료 상태
     * @return true: 완료, false: 미완료
     */
    bool isLinearMove1Done() const { return todayLinearMove1Done; }
    
    /**
     * @brief 2차 식물이동 완료 상태
     * @return true: 완료, false: 미완료
     */
    bool isLinearMove2Done() const { return todayLinearMove2Done; }
    
    /**
     * @brief 스케줄러 활성화 상태
     * @return true: 활성화, false: 비활성화
     */
    bool isEnabled() const { return schedulerEnabled; }
    
    /**
     * @brief 현재 제어 모드
     * @return 제어 모드
     */
    ControlMode getControlMode() const { return currentMode; }
    
    /**
     * @brief 비상 모드 상태
     * @return true: 비상 모드, false: 정상 모드
     */
    bool isEmergencyMode() const { return emergencyMode; }
    
    // ========== 통계 및 모니터링 ==========
    /**
     * @brief 총 실행된 작업 수
     * @return 작업 실행 횟수
     */
    int getTotalTasksExecuted() const { return totalTasksExecuted; }
    
    /**
     * @brief 총 환경 기반 동작 수
     * @return 환경 동작 횟수
     */
    int getTotalEnvironmentActions() const { return totalEnvironmentActions; }
    
    /**
     * @brief 연속 실패 횟수
     * @return 실패 횟수
     */
    int getConsecutiveFailures() const { return consecutiveFailures; }
    
    /**
     * @brief 등록된 작업 수
     * @return 작업 수
     */
    int getTaskCount() const { return taskCount; }
    
    /**
     * @brief 등록된 환경 조건 수
     * @return 조건 수
     */
    int getConditionCount() const { return conditionCount; }
    
    /**
     * @brief 오늘 완료된 작업 비율
     * @return 완료율 (0.0-1.0)
     */
    float getTodayCompletionRate() const;
    
    // ========== 고급 스케줄링 기능 ==========
    /**
     * @brief 조건부 작업 추가 (특정 조건 만족 시에만 실행)
     * @param condition 조건 함수 포인터
     * @param task 실행할 작업
     * @return 작업 ID
     */
    int addConditionalTask(bool (*condition)(), const ScheduledTask& task);
    
    /**
     * @brief 지연 작업 추가 (지정 시간 후 실행)
     * @param delay 지연 시간 (ms)
     * @param task 실행할 작업
     * @return 작업 ID
     */
    int addDelayedTask(unsigned long delay, const ScheduledTask& task);
    
    /**
     * @brief 반복 작업 추가 (지정 간격으로 반복)
     * @param interval 반복 간격 (ms)
     * @param task 실행할 작업
     * @param maxRepeats 최대 반복 횟수 (0: 무한)
     * @return 작업 ID
     */
    int addRepeatingTask(unsigned long interval, const ScheduledTask& task, int maxRepeats = 0);
    
    /**
     * @brief 우선순위 기반 작업 실행
     */
    void executeHighPriorityTasks();
    
    /**
     * @brief 작업 큐 최적화
     */
    void optimizeTaskQueue();
    
    // ========== 계절별 및 적응형 제어 ==========
    /**
     * @brief 계절별 스케줄 적용
     * @param season 계절
     */
    void applySeasonalSchedule(Season season);
    
    /**
     * @brief 적응형 제어 활성화/비활성화
     * @param enabled true: 활성화, false: 비활성화
     */
    void setAdaptiveControl(bool enabled);
    
    /**
     * @brief 학습률 설정 (적응형 제어용)
     * @param rate 학습률 (0.0-1.0)
     */
    void setLearningRate(float rate);
    
    /**
     * @brief 환경 데이터 학습 및 스케줄 최적화
     * @param sensors 센서 매니저 참조
     */
    void learnAndAdapt(const SensorManager& sensors);
    
    // ========== 비상 상황 처리 ==========
    /**
     * @brief 비상 모드 활성화
     * @param reason 비상 상황 원인
     */
    void enterEmergencyMode(const String& reason);
    
    /**
     * @brief 비상 모드 해제
     */
    void exitEmergencyMode();
    
    /**
     * @brief 비상 스케줄 실행
     * @param actuators 액추에이터 매니저 참조
     */
    void executeEmergencySchedule(ActuatorManager& actuators);
    
    /**
     * @brief 시스템 복구 시도
     * @param sensors 센서 매니저 참조
     * @param actuators 액추에이터 매니저 참조
     * @return true: 복구 성공, false: 복구 실패
     */
    bool attemptSystemRecovery(const SensorManager& sensors, ActuatorManager& actuators);
    
    // ========== 외부 트리거 ==========
    /**
     * @brief 수동 작업 트리거
     * @param taskId 실행할 작업 ID
     * @param actuators 액추에이터 매니저 참조
     * @return true: 성공, false: 실패
     */
    bool triggerTask(int taskId, ActuatorManager& actuators);
    
    /**
     * @brief 즉시 물주기 실행
     * @param actuators 액추에이터 매니저 참조
     * @param duration 지속 시간 (0이면 기본값)
     */
    void triggerImmediateWatering(ActuatorManager& actuators, unsigned long duration = 0);
    
    /**
     * @brief 즉시 식물 이동 실행
     * @param actuators 액추에이터 매니저 참조
     */
    void triggerImmediatePlantMovement(ActuatorManager& actuators);
    
    // ========== 설정 저장/로드 ==========
    /**
     * @brief 스케줄 설정을 EEPROM에 저장
     * @return true: 성공, false: 실패
     */
    bool saveScheduleToEEPROM();
    
    /**
     * @brief EEPROM에서 스케줄 설정 로드
     * @return true: 성공, false: 실패
     */
    bool loadScheduleFromEEPROM();
    
    /**
     * @brief 스케줄 설정 초기화 (공장 기본값)
     */
    void resetToDefaults();
    
    /**
     * @brief 설정을 JSON 형태로 내보내기
     * @return JSON 문자열
     */
    String exportSettingsToJSON();
    
    /**
     * @brief JSON에서 설정 가져오기
     * @param jsonData JSON 문자열
     * @return true: 성공, false: 실패
     */
    bool importSettingsFromJSON(const String& jsonData);
    
    // ========== 디버그 및 진단 ==========
    /**
     * @brief 스케줄러 상태 출력
     */
    void printSchedulerStatus();
    
    /**
     * @brief 상세 진단 정보 출력
     */
    void printDiagnostics();
    
    /**
     * @brief 작업 실행 로그 출력
     */
    void printExecutionLog();
    
    /**
     * @brief 성능 통계 출력
     */
    void printPerformanceStats();
    
    /**
     * @brief 메모리 사용량 분석
     * @return 사용 중인 메모리 바이트 수
     */
    int analyzeMemoryUsage();
    
private:
    // ========== 내부 헬퍼 함수들 ==========
    /**
     * @brief 작업 실행
     * @param taskId 작업 ID
     * @param actuators 액추에이터 매니저 참조
     * @return true: 성공, false: 실패
     */
    bool executeTask(int taskId, ActuatorManager& actuators);
    
    /**
     * @brief 환경 조건 평가
     * @param conditionId 조건 ID
     * @param sensors 센서 매니저 참조
     * @return true: 조건 만족, false: 조건 불만족
     */
    bool evaluateEnvironmentCondition(int conditionId, const SensorManager& sensors);
    
    /**
     * @brief 환경 기반 액션 실행
     * @param conditionId 조건 ID
     * @param actuators 액추에이터 매니저 참조
     * @return true: 성공, false: 실패
     */
    bool executeEnvironmentAction(int conditionId, ActuatorManager& actuators);
    
    /**
     * @brief 시간 비교 (분 단위)
     * @param currentTime 현재 시간
     * @param targetHour 목표 시간
     * @param targetMinute 목표 분
     * @return true: 일치, false: 불일치
     */
    bool isTimeMatch(const DateTime& currentTime, int targetHour, int targetMinute);
    
    /**
     * @brief 쿨다운 확인
     * @param lastActionTime 마지막 동작 시간
     * @param cooldownTime 쿨다운 시간 (ms)
     * @return true: 쿨다운 완료, false: 쿨다운 중
     */
    bool isCooldownComplete(unsigned long lastActionTime, unsigned long cooldownTime);
    
    /**
     * @brief 작업 유효성 검사
     * @param task 검사할 작업
     * @return true: 유효, false: 무효
     */
    bool validateTask(const ScheduledTask& task);
    
    /**
     * @brief 환경 조건 유효성 검사
     * @param condition 검사할 조건
     * @return true: 유효, false: 무효
     */
    bool validateEnvironmentCondition(const EnvironmentCondition& condition);
    
    /**
     * @brief 작업 충돌 검사
     * @param newTask 새로운 작업
     * @return true: 충돌 없음, false: 충돌 있음
     */
    bool checkTaskConflict(const ScheduledTask& newTask);
    
    /**
     * @brief 리소스 사용량 확인
     * @param actuatorType 액추에이터 타입
     * @return true: 사용 가능, false: 사용 불가
     */
    bool checkResourceAvailability(ActuatorType actuatorType);
    
    /**
     * @brief 실행 로그 기록
     * @param taskId 작업 ID
     * @param success 성공 여부
     * @param timestamp 실행 시간
     */
    void logExecution(int taskId, bool success, unsigned long timestamp);
    
    /**
     * @brief 오류 처리
     * @param errorCode 오류 코드
     * @param taskId 관련 작업 ID
     */
    void handleSchedulerError(int errorCode, int taskId = -1);
    
    /**
     * @brief 통계 업데이트
     * @param success 성공 여부
     * @param isEnvironmentAction 환경 기반 동작 여부
     */
    void updateStatistics(bool success, bool isEnvironmentAction = false);
    
    // ========== 실행 로그 ==========
    struct ExecutionLog {
        int taskId;
        unsigned long timestamp;
        bool success;
        String description;
    };
    
    static const int MAX_LOG_ENTRIES = 50;
    ExecutionLog executionLog[MAX_LOG_ENTRIES];
    int logIndex;
    
    // ========== 성능 모니터링 ==========
    unsigned long totalExecutionTime;    ///< 총 실행 시간
    unsigned long maxExecutionTime;      ///< 최대 실행 시간
    unsigned long minExecutionTime;      ///< 최소 실행 시간
    int performanceSamples;              ///< 성능 샘플 수
    
    // ========== 백업 데이터 ==========
    ScheduledTask backupTasks[MAX_SCHEDULED_TASKS];
    EnvironmentCondition backupConditions[MAX_ENV_CONDITIONS];
    bool hasBackup;                      ///< 백업 데이터 존재 여부
    
    // ========== 동적 스케줄링 ==========
    struct DynamicTask {
        unsigned long executeTime;        ///< 실행 시간 (밀리초)
        ScheduledTask task;              ///< 작업 정보
        bool isActive;                   ///< 활성 상태
    };
    
    static const int MAX_DYNAMIC_TASKS = 10;
    DynamicTask dynamicTasks[MAX_DYNAMIC_TASKS];
    int dynamicTaskCount;
};

#endif // SCHEDULER_H