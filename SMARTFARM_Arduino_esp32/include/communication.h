#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "config.h"

// 전방 선언
class SensorManager;
class ActuatorManager;
class Scheduler;

/**
 * @brief 메시지 구조체
 * 
 * 송수신되는 메시지의 정보를 저장하는 구조체입니다.
 */
struct Message {
    String command;              ///< 명령어
    String data;                ///< 데이터
    unsigned long timestamp;     ///< 타임스탬프
    int priority;               ///< 우선순위 (1-5)
    bool requiresResponse;      ///< 응답 필요 여부
    int retryCount;            ///< 재시도 횟수
    String source;             ///< 발신자
    String destination;        ///< 수신자
};

/**
 * @brief 통신 프로토콜 열거형
 */
enum CommunicationProtocol {
    PROTOCOL_SERIAL = 0,        ///< 시리얼 통신
    PROTOCOL_ESP32 = 1,         ///< ESP32 통신
    PROTOCOL_WIFI = 2,          ///< WiFi 통신
    PROTOCOL_BLUETOOTH = 3      ///< 블루투스 통신
};

/**
 * @brief 통신 상태 열거형
 */
enum CommunicationStatus {
    COMM_DISCONNECTED = 0,      ///< 연결 끊김
    COMM_CONNECTING = 1,        ///< 연결 중
    COMM_CONNECTED = 2,         ///< 연결됨
    COMM_ERROR = 3,            ///< 오류
    COMM_TIMEOUT = 4           ///< 타임아웃
};

/**
 * @brief 통신 매니저 클래스
 * 
 * 모든 통신 기능을 관리하는 클래스입니다.
 * ESP32 통신, 시리얼 통신, 데이터 전송, 명령 수신 등의 기능을 제공합니다.
 */
class CommunicationManager {
private:
    // 통신 객체
    SoftwareSerial esp32Serial;     ///< ESP32 통신용 시리얼
    
    // 통신 상태
    CommunicationStatus esp32Status; ///< ESP32 연결 상태
    CommunicationStatus serialStatus; ///< 시리얼 연결 상태
    unsigned long lastSendTime;     ///< 마지막 전송 시간
    unsigned long lastReceiveTime;  ///< 마지막 수신 시간
    unsigned long lastHeartbeat;    ///< 마지막 하트비트 시간
    
    // 메시지 큐
    static const int MAX_MESSAGE_QUEUE = 20;
    Message sendQueue[MAX_MESSAGE_QUEUE];     ///< 송신 큐
    Message receiveQueue[MAX_MESSAGE_QUEUE];  ///< 수신 큐
    int sendQueueSize;              ///< 송신 큐 크기
    int receiveQueueSize;           ///< 수신 큐 크기
    int sendQueueIndex;             ///< 송신 큐 인덱스
    int receiveQueueIndex;          ///< 수신 큐 인덱스
    
    // 통신 설정
    unsigned long sendInterval;     ///< 전송 간격 (ms)
    unsigned long timeoutPeriod;    ///< 타임아웃 시간 (ms)
    int maxRetryCount;             ///< 최대 재시도 횟수
    bool compressionEnabled;       ///< 압축 활성화
    bool encryptionEnabled;        ///< 암호화 활성화
    
    // 통계 및 모니터링
    unsigned long totalBytesSent;   ///< 총 송신 바이트
    unsigned long totalBytesReceived; ///< 총 수신 바이트
    int successfulTransmissions;   ///< 성공한 전송 횟수
    int failedTransmissions;       ///< 실패한 전송 횟수
    int totalCommands;             ///< 총 명령 수신 횟수
    
    // 오류 관리
    int consecutiveErrors;         ///< 연속 오류 횟수
    unsigned long lastErrorTime;   ///< 마지막 오류 시간
    String lastErrorMessage;       ///< 마지막 오류 메시지
    
    // 버퍼 관리
    String inputBuffer;            ///< 입력 버퍼
    String outputBuffer;           ///< 출력 버퍼
    static const int BUFFER_SIZE = 512; ///< 버퍼 크기
    
public:
    /**
     * @brief 생성자
     */
    CommunicationManager();
    
    /**
     * @brief 소멸자
     */
    ~CommunicationManager();
    
    // ========== 초기화 및 설정 ==========
    /**
     * @brief 통신 매니저 초기화
     * @return true 성공, false 실패
     */
    bool init();
    
    /**
     * @brief 통신 설정
     * @param sendInterval 전송 간격 (ms)
     * @param timeout 타임아웃 시간 (ms)
     * @param maxRetries 최대 재시도 횟수
     */
    void configure(unsigned long sendInterval, unsigned long timeout, int maxRetries);
    
    /**
     * @brief ESP32 연결 시도
     * @return true 성공, false 실패
     */
    bool connectESP32();
    
    /**
     * @brief ESP32 연결 해제
     */
    void disconnectESP32();
    
    /**
     * @brief 연결 상태 확인
     * @return ESP32 연결 상태
     */
    CommunicationStatus getESP32Status() const { return esp32Status; }
    
    // ========== 메인 업데이트 ==========
    /**
     * @brief 통신 매니저 업데이트 (메인 루프에서 호출)
     */
    void update();
    
    /**
     * @brief 수신 메시지 처리
     */
    void processIncomingMessages();
    
    /**
     * @brief 송신 큐 처리
     */
    void processSendQueue();
    
    /**
     * @brief 하트비트 전송
     */
    void sendHeartbeat();
    
    // ========== 데이터 전송 ==========
    /**
     * @brief ESP32로 센서 데이터 전송
     * @param sensors 센서 매니저 참조
     * @param actuators 액추에이터 매니저 참조
     * @param scheduler 스케줄러 참조
     * @return true 성공, false 실패
     */
    bool sendToESP32(const SensorManager& sensors, 
                     const ActuatorManager& actuators, 
                     const Scheduler& scheduler);
    
    /**
     * @brief JSON 데이터 전송
     * @param jsonData JSON 형태의 데이터
     * @param protocol 사용할 프로토콜
     * @return true 성공, false 실패
     */
    bool sendJSON(const String& jsonData, CommunicationProtocol protocol = PROTOCOL_ESP32);
    
    /**
     * @brief 일반 메시지 전송
     * @param message 전송할 메시지
     * @param priority 우선순위 (1-5)
     * @param requiresResponse 응답 필요 여부
     * @return true 성공, false 실패
     */
    bool sendMessage(const String& message, int priority = 3, bool requiresResponse = false);
    
    /**
     * @brief 명령 전송
     * @param command 명령어
     * @param data 명령 데이터
     * @param protocol 사용할 프로토콜
     * @return true 성공, false 실패
     */
    bool sendCommand(const String& command, const String& data, 
                     CommunicationProtocol protocol = PROTOCOL_ESP32);
    
    /**
     * @brief 알림 전송
     * @param notification 알림 메시지
     * @param priority 우선순위
     * @return true 성공, false 실패
     */
    bool sendNotification(const String& notification, NotificationPriority priority);
    
    // ========== 데이터 수신 ==========
    /**
     * @brief 메시지 수신 대기
     * @param timeout 대기 시간 (ms)
     * @return 수신된 메시지 (없으면 빈 문자열)
     */
    String receiveMessage(unsigned long timeout = 1000);
    
    /**
     * @brief 명령 수신 확인
     * @return true: 명령 있음, false: 명령 없음
     */
    bool hasIncomingCommand();
    
    /**
     * @brief 다음 명령 가져오기
     * @return 명령 메시지 구조체
     */
    Message getNextCommand();
    
    /**
     * @brief 특정 명령 대기
     * @param command 대기할 명령어
     * @param timeout 대기 시간 (ms)
     * @return true: 명령 수신, false: 타임아웃
     */
    bool waitForCommand(const String& command, unsigned long timeout = 5000);
    
    // ========== JSON 데이터 생성 ==========
    /**
     * @brief 시스템 상태 JSON 생성
     * @param sensors 센서 매니저 참조
     * @param actuators 액추에이터 매니저 참조
     * @param scheduler 스케줄러 참조
     * @return JSON 문자열
     */
    String createSystemStatusJSON(const SensorManager& sensors, 
                                 const ActuatorManager& actuators, 
                                 const Scheduler& scheduler);
    
    /**
     * @brief 센서 데이터 JSON 생성
     * @param sensors 센서 매니저 참조
     * @return JSON 문자열
     */
    String createSensorDataJSON(const SensorManager& sensors);
    
    /**
     * @brief 액추에이터 상태 JSON 생성
     * @param actuators 액추에이터 매니저 참조
     * @return JSON 문자열
     */
    String createActuatorStatusJSON(const ActuatorManager& actuators);
    
    /**
     * @brief 스케줄 정보 JSON 생성
     * @param scheduler 스케줄러 참조
     * @return JSON 문자열
     */
    String createScheduleJSON(const Scheduler& scheduler);
    
    /**
     * @brief 알림 JSON 생성
     * @param message 알림 메시지
     * @param priority 우선순위
     * @param timestamp 타임스탬프
     * @return JSON 문자열
     */
    String createNotificationJSON(const String& message, int priority, unsigned long timestamp);
    
    // ========== 상태 출력 및 로깅 ==========
    /**
     * @brief 시스템 상태 출력
     * @param sensors 센서 매니저 참조
     * @param actuators 액추에이터 매니저 참조
     * @param scheduler 스케줄러 참조
     */
    void printSystemStatus(const SensorManager& sensors, 
                          const ActuatorManager& actuators, 
                          const Scheduler& scheduler);
    
    /**
     * @brief 통신 상태 출력
     */
    void printCommunicationStatus();
    
    /**
     * @brief 통신 통계 출력
     */
    void printCommunicationStats();
    
    /**
     * @brief 오류 로그 출력
     */
    void printErrorLog();
    
    /**
     * @brief 디버그 정보 출력
     * @param message 디버그 메시지
     * @param level 로그 레벨 (0-4)
     */
    void printDebug(const String& message, int level = LOG_LEVEL_INFO);
    
    /**
     * @brief 상세 센서 정보 출력
     * @param sensors 센서 매니저 참조
     */
    void printDetailedSensorInfo(const SensorManager& sensors);
    
    /**
     * @brief 상세 액추에이터 정보 출력
     * @param actuators 액추에이터 매니저 참조
     */
    void printDetailedActuatorInfo(const ActuatorManager& actuators);
    
    // ========== 명령 처리 ==========
    /**
     * @brief 수신된 명령 처리
     * @param command 명령어
     * @param data 명령 데이터
     * @param actuators 액추에이터 매니저 참조
     * @param scheduler 스케줄러 참조
     * @return true: 처리 성공, false: 처리 실패
     */
    bool processCommand(const String& command, const String& data,
                       ActuatorManager& actuators, Scheduler& scheduler);
    
    /**
     * @brief LED 제어 명령 처리
     * @param data 명령 데이터 ("on", "off", "brightness:255")
     * @param actuators 액추에이터 매니저 참조
     * @return true: 성공, false: 실패
     */
    bool handleLEDCommand(const String& data, ActuatorManager& actuators);
    
    /**
     * @brief 펌프 제어 명령 처리
     * @param data 명령 데이터 ("on", "off", "duration:30")
     * @param actuators 액추에이터 매니저 참조
     * @return true: 성공, false: 실패
     */
    bool handlePumpCommand(const String& data, ActuatorManager& actuators);
    
    /**
     * @brief 팬 제어 명령 처리
     * @param data 명령 데이터 ("on", "off", "speed:128")
     * @param actuators 액추에이터 매니저 참조
     * @return true: 성공, false: 실패
     */
    bool handleFanCommand(const String& data, ActuatorManager& actuators);
    
    /**
     * @brief 스케줄 제어 명령 처리
     * @param data 명령 데이터
     * @param scheduler 스케줄러 참조
     * @return true: 성공, false: 실패
     */
    bool handleScheduleCommand(const String& data, Scheduler& scheduler);
    
    /**
     * @brief 시스템 명령 처리 (재시작, 리셋 등)
     * @param data 명령 데이터
     * @return true: 성공, false: 실패
     */
    bool handleSystemCommand(const String& data);
    
    /**
     * @brief 응답 메시지 전송
     * @param originalCommand 원본 명령
     * @param success 성공 여부
     * @param message 응답 메시지
     * @return true: 성공, false: 실패
     */
    bool sendResponse(const String& originalCommand, bool success, const String& message);
    
    // ========== 원격 제어 ==========
    /**
     * @brief 원격 제어 활성화/비활성화
     * @param enabled true: 활성화, false: 비활성화
     */
    void setRemoteControlEnabled(bool enabled);
    
    /**
     * @brief 원격 제어 상태 확인
     * @return true: 활성화, false: 비활성화
     */
    bool isRemoteControlEnabled() const { return remoteControlEnabled; }
    
    /**
     * @brief 인증 토큰 설정
     * @param token 인증 토큰
     */
    void setAuthToken(const String& token);
    
    /**
     * @brief 명령 인증 확인
     * @param token 제공된 토큰
     * @return true: 인증 성공, false: 인증 실패
     */
    bool authenticateCommand(const String& token);
    
    // ========== 데이터 압축 및 암호화 ==========
    /**
     * @brief 데이터 압축
     * @param data 원본 데이터
     * @return 압축된 데이터
     */
    String compressData(const String& data);
    
    /**
     * @brief 데이터 압축 해제
     * @param compressedData 압축된 데이터
     * @return 원본 데이터
     */
    String decompressData(const String& compressedData);
    
    /**
     * @brief 데이터 암호화 (간단한 XOR)
     * @param data 원본 데이터
     * @param key 암호화 키
     * @return 암호화된 데이터
     */
    String encryptData(const String& data, const String& key);
    
    /**
     * @brief 데이터 복호화
     * @param encryptedData 암호화된 데이터
     * @param key 복호화 키
     * @return 원본 데이터
     */
    String decryptData(const String& encryptedData, const String& key);
    
    // ========== 파일 전송 (향후 확장용) ==========
    /**
     * @brief 파일 전송 시작
     * @param filename 파일명
     * @param fileSize 파일 크기
     * @return true: 성공, false: 실패
     */
    bool startFileTransfer(const String& filename, unsigned long fileSize);
    
    /**
     * @brief 파일 청크 전송
     * @param chunkData 청크 데이터
     * @param chunkIndex 청크 인덱스
     * @return true: 성공, false: 실패
     */
    bool sendFileChunk(const String& chunkData, int chunkIndex);
    
    /**
     * @brief 파일 전송 완료
     * @return true: 성공, false: 실패
     */
    bool completeFileTransfer();
    
    // ========== 네트워크 관리 (ESP32용) ==========
    /**
     * @brief WiFi 연결 상태 확인
     * @return true: 연결됨, false: 연결 안됨
     */
    bool isWiFiConnected();
    
    /**
     * @brief 네트워크 신호 강도 확인
     * @return 신호 강도 (dBm)
     */
    int getSignalStrength();
    
    /**
     * @brief IP 주소 확인
     * @return IP 주소 문자열
     */
    String getIPAddress();
    
    /**
     * @brief 네트워크 재연결 시도
     * @return true: 성공, false: 실패
     */
    bool reconnectNetwork();
    
    // ========== 시간 동기화 ==========
    /**
     * @brief NTP 서버에서 시간 동기화
     * @return true: 성공, false: 실패
     */
    bool synchronizeTime();
    
    /**
     * @brief 시간 동기화 상태 확인
     * @return true: 동기화됨, false: 동기화 안됨
     */
    bool isTimeSynchronized();
    
    /**
     * @brief 마지막 동기화 시간 확인
     * @return 마지막 동기화 시간 (Unix 타임스탬프)
     */
    unsigned long getLastSyncTime();
    
    // ========== 통계 및 성능 ==========
    /**
     * @brief 전송 성공률 계산
     * @return 성공률 (0.0-1.0)
     */
    float getTransmissionSuccessRate() const;
    
    /**
     * @brief 평균 응답 시간 계산
     * @return 평균 응답 시간 (ms)
     */
    unsigned long getAverageResponseTime() const;
    
    /**
     * @brief 대역폭 사용량 계산
     * @return 초당 바이트 수
     */
    float getBandwidthUsage() const;
    
    /**
     * @brief 통계 리셋
     */
    void resetStatistics();
    
    /**
     * @brief 성능 보고서 생성
     * @return 성능 보고서 JSON
     */
    String generatePerformanceReport();
    
    // ========== 오류 처리 및 복구 ==========
    /**
     * @brief 통신 오류 처리
     * @param errorCode 오류 코드
     * @param errorMessage 오류 메시지
     */
    void handleCommunicationError(int errorCode, const String& errorMessage);
    
    /**
     * @brief 자동 복구 시도
     * @return true: 복구 성공, false: 복구 실패
     */
    bool attemptAutoRecovery();
    
    /**
     * @brief 연결 복구 시도
     * @param protocol 복구할 프로토콜
     * @return true: 성공, false: 실패
     */
    bool recoverConnection(CommunicationProtocol protocol);
    
    /**
     * @brief 버퍼 오버플로우 처리
     */
    void handleBufferOverflow();
    
    /**
     * @brief 타임아웃 처리
     */
    void handleTimeout();
    
    // ========== 고급 기능 ==========
    /**
     * @brief 배치 전송 (여러 메시지 한번에)
     * @param messages 메시지 배열
     * @param count 메시지 개수
     * @return 성공한 전송 개수
     */
    int sendBatch(const Message messages[], int count);
    
    /**
     * @brief 우선순위 큐 관리
     */
    void managePriorityQueue();
    
    /**
     * @brief 적응형 전송 간격 조정
     * @param networkCondition 네트워크 상태 (0.0-1.0)
     */
    void adaptTransmissionInterval(float networkCondition);
    
    /**
     * @brief 메시지 중복 제거
     * @param message 확인할 메시지
     * @return true: 중복, false: 신규
     */
    bool isDuplicateMessage(const Message& message);
    
private:
    // ========== 내부 헬퍼 함수들 ==========
    /**
     * @brief 메시지 큐에 추가
     * @param message 추가할 메시지
     * @param queue 대상 큐 (true: 송신, false: 수신)
     * @return true: 성공, false: 큐 가득참
     */
    bool addToQueue(const Message& message, bool isSendQueue);
    
    /**
     * @brief 메시지 큐에서 제거
     * @param queue 대상 큐 (true: 송신, false: 수신)
     * @return 제거된 메시지
     */
    Message removeFromQueue(bool isSendQueue);
    
    /**
     * @brief JSON 파싱
     * @param jsonData JSON 문자열
     * @param command 파싱된 명령어 (출력)
     * @param data 파싱된 데이터 (출력)
     * @return true: 성공, false: 실패
     */
    bool parseJSON(const String& jsonData, String& command, String& data);
    
    /**
     * @brief 체크섬 계산 (데이터 무결성 확인용)
     * @param data 데이터
     * @return 체크섬 값
     */
    uint16_t calculateChecksum(const String& data);
    
    /**
     * @brief 메시지 유효성 검사
     * @param message 검사할 메시지
     * @return true: 유효, false: 무효
     */
    bool validateMessage(const Message& message);
    
    /**
     * @brief 버퍼 정리
     */
    void clearBuffers();
    
    /**
     * @brief 연결 상태 업데이트
     * @param protocol 프로토콜
     * @param status 새로운 상태
     */
    void updateConnectionStatus(CommunicationProtocol protocol, CommunicationStatus status);
    
    /**
     * @brief 타임스탬프 생성
     * @return 현재 타임스탬프
     */
    unsigned long generateTimestamp();
    
    /**
     * @brief 메시지 ID 생성
     * @return 고유 메시지 ID
     */
    String generateMessageID();
    
    // ========== 내부 상태 변수들 ==========
    bool remoteControlEnabled;          ///< 원격 제어 활성화
    String authToken;                   ///< 인증 토큰
    String encryptionKey;              ///< 암호화 키
    
    // 성능 모니터링
    unsigned long totalResponseTime;    ///< 총 응답 시간
    int responseCount;                 ///< 응답 횟수
    unsigned long lastBandwidthCheck;  ///< 마지막 대역폭 체크 시간
    
    // 네트워크 상태
    bool wifiConnected;                ///< WiFi 연결 상태
    int signalStrength;               ///< 신호 강도
    String ipAddress;                 ///< IP 주소
    unsigned long lastSyncTime;       ///< 마지막 시간 동기화 시간
    
    // 오류 추적
    static const int MAX_ERROR_LOG = 10;
    struct ErrorEntry {
        unsigned long timestamp;
        int errorCode;
        String errorMessage;
    };
    ErrorEntry errorLog[MAX_ERROR_LOG];
    int errorLogIndex;
    
    // 메시지 히스토리 (중복 검사용)
    static const int MESSAGE_HISTORY_SIZE = 20;
    String messageHistory[MESSAGE_HISTORY_SIZE];
    int historyIndex;
    
    // 동적 설정
    float adaptiveMultiplier;          ///< 적응형 전송 간격 배율
    bool autoRecoveryEnabled;          ///< 자동 복구 활성화
    int maxQueueSize;                 ///< 최대 큐 크기
    
    // 파일 전송 상태
    bool fileTransferActive;          ///< 파일 전송 진행 중
    String currentFilename;           ///< 현재 전송 중인 파일명
    unsigned long currentFileSize;    ///< 현재 파일 크기
    int currentChunkIndex;           ///< 현재 청크 인덱스
};

#endif // COMMUNICATION_H