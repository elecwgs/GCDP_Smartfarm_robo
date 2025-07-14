#include <Arduino.h>
#include "smartfarm.h"

// 전역 스마트팜 시스템 객체
SmartFarm farm;

// 인터럽트 핀 (비상 정지용, 선택사항)
#define EMERGENCY_STOP_PIN A0
volatile bool emergencyStopPressed = false;

// 비상 정지 인터럽트 핸들러
void emergencyStopISR() {
  emergencyStopPressed = true;
}

void setup() {
  // 시리얼 통신 초기화
  Serial.begin(9600);
  while (!Serial && millis() < 3000) {
    // 시리얼 연결 대기 (최대 3초)
    delay(10);
  }
  
  Serial.println("\n" + String('=', 50));
  Serial.println("🌱 스마트팜 상추재배 시스템 v2.0 시작");
  Serial.println(String('=', 50));
  
  // 비상 정지 핀 설정 (선택사항)
  pinMode(EMERGENCY_STOP_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(EMERGENCY_STOP_PIN), emergencyStopISR, FALLING);
  
  // 시스템 초기화
  Serial.println("🔧 시스템 초기화 중...");
  if (!farm.init()) {
    Serial.println("❌ 시스템 초기화 실패!");
    Serial.println("   하드웨어 연결을 확인하고 리셋하세요.");
    
    // 오류 LED 깜빡임 (핀 13)
    pinMode(LED_BUILTIN, OUTPUT);
    while (true) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(200);
      digitalWrite(LED_BUILTIN, LOW);
      delay(200);
    }
  }
  
  Serial.println("✅ 스마트팜 시스템 시작 완료!");
  Serial.println("📊 실시간 모니터링을 시작합니다...\n");
  
  // 시작 상태 LED (3번 깜빡임)
  pinMode(LED_BUILTIN, OUTPUT);
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(300);
    digitalWrite(LED_BUILTIN, LOW);
    delay(300);
  }
  
  // 메모리 사용량 출력 (디버그용)
  #ifdef DEBUG
  Serial.print("사용 가능한 메모리: ");
  Serial.print(freeMemory());
  Serial.println(" bytes");
  #endif
}

void loop() {
  // 비상 정지 확인
  if (emergencyStopPressed) {
    Serial.println("🚨 비상 정지 버튼 감지!");
    farm.emergencyStop();
    
    // 비상 정지 상태 표시
    while (true) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
    }
  }
  
  // 메인 시스템 실행
  farm.run();
  
  // 시스템 상태 LED (정상 동작 시 천천히 깜빡임)
  static unsigned long lastLedBlink = 0;
  static bool ledState = false;
  
  if (millis() - lastLedBlink > 2000) {
    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState);
    lastLedBlink = millis();
  }
  
  // 메모리 누수 감지 (디버그 모드)
  #ifdef DEBUG
  static unsigned long lastMemoryCheck = 0;
  static int lastFreeMemory = 0;
  
  if (millis() - lastMemoryCheck > 60000) { // 1분마다
    int currentMemory = freeMemory();
    if (lastFreeMemory > 0 && (lastFreeMemory - currentMemory) > 50) {
      Serial.print("⚠️ 메모리 감소 감지: ");
      Serial.print(currentMemory);
      Serial.println(" bytes");
    }
    lastFreeMemory = currentMemory;
    lastMemoryCheck = millis();
  }
  #endif
}

// 메모리 체크 함수 (AVR 전용)
#ifdef __AVR__
int freeMemory() {
  char top;
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &top - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
#else
int freeMemory() {
  return -1; // 다른 플랫폼에서는 지원하지 않음
}
#endif

// 시스템 리셋 함수 (소프트웨어 리셋)
void resetSystem() {
  Serial.println("🔄 시스템 리셋 중...");
  delay(1000);
  
  #ifdef __AVR__
  // AVR 소프트웨어 리셋
  asm volatile ("  jmp 0");
  #else
  // 다른 플랫폼에서는 ESP.restart() 등 사용
  Serial.println("⚠️ 수동으로 리셋 버튼을 누르세요.");
  #endif
}

// 오류 발생 시 호출되는 함수
void handleSystemError(const String& errorMessage) {
  Serial.println("🚨 시스템 오류 발생!");
  Serial.println("오류 내용: " + errorMessage);
  
  // 오류 로그 저장
  // 추후 EEPROM이나 SD카드에 저장 가능
  
  // 비상 모드로 전환
  farm.emergencyStop();
  
  // 사용자에게 알림
  Serial.println("시스템이 안전 모드로 전환되었습니다.");
  Serial.println("하드웨어를 점검한 후 리셋하세요.");
  
  // 무한 루프로 대기
  while (true) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }
}