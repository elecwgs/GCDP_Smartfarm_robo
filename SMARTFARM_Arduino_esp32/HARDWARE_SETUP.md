# 스마트팜 하드웨어 설치 가이드

## 필요한 부품 리스트

### 메인 보드
- Arduino Uno/Nano/Mega (1개)
- 브레드보드 또는 PCB (1개)
- 점퍼 와이어 세트

### 센서 모듈
- DHT22 온습도 센서 (1개)
- 토양수분센서 (1개)
- 조도센서 (LDR) (1개)
- 10kΩ 저항 (LDR용) (1개)

### 제어 장치
- DC380 워터펌프 (2개)
- 12V 팬 (4개)
- LED 스트립 또는 고출력 LED (1개)
- 스텝모터 (NEMA17 권장) (1개)
- 스텝모터 드라이버 (A4988 또는 DRV8825) (1개)

### 표시 장치
- 16x2 LCD (I2C 백팩 포함) (1개)

### 통신 모듈
- ESP32 개발보드 (1개)

### 전원 및 제어
- 8채널 릴레이 모듈 (1개)
- 12V/10A 스위칭 파워서플라이 (1개)
- 5V 레귤레이터 (1개)
- 전원 커넥터 및 케이블

### 기구 부품
- 방수 케이스 (1개)
- CNC 리니어 가이드 (1세트)
- 물 호스 및 연결부품
- 지지대 및 고정 부품

## 회로도 연결

### 아두이노 핀 연결 상세

#### 디지털 핀
```
D2  → DHT22 Data Pin
D3  → Relay Module IN1 (Pump1)
D4  → Relay Module IN2 (Pump2)
D5  → Relay Module IN3 (Fan1)
D6  → Relay Module IN4 (Fan2)
D7  → Relay Module IN5 (Fan3)
D8  → Relay Module IN6 (Fan4)
D9  → Relay Module IN7 (LED)
D10 → Stepper Driver STEP
D11 → Stepper Driver DIR
D12 → Stepper Driver ENABLE
```

#### 아날로그 핀
```
A0 → Soil Moisture Sensor Signal
A1 → LDR (Light Sensor)
A2 → ESP32 TX (Arduino RX)
A3 → ESP32 RX (Arduino TX)
```

#### I2C 핀 (LCD)
```
SDA → LCD SDA
SCL → LCD SCL
```

#### 전원 연결
```
5V  → DHT22 VCC, LCD VCC, Relay Module VCC
GND → 모든 GND 핀 공통 연결
12V → Pump, Fan, LED, Stepper Motor
```

## 단계별 조립 가이드

### 1단계: 센서 연결

#### DHT22 온습도 센서
```
DHT22 Pin 1 (VCC) → Arduino 5V
DHT22 Pin 2 (Data) → Arduino D2
DHT22 Pin 3 (NC) → 연결 안함
DHT22 Pin 4 (GND) → Arduino GND
```

#### 토양수분센서
```
Sensor VCC → Arduino 5V
Sensor GND → Arduino GND
Sensor A0 → Arduino A0
```

#### 조도센서 (LDR)
```
LDR 한쪽 → Arduino A1
LDR 다른쪽 → 10kΩ 저항 → GND
Arduino A1 → 10kΩ 저항 → 5V
```

### 2단계: 릴레이 모듈 연결

#### 릴레이 모듈 전원
```
Relay VCC → Arduino 5V
Relay GND → Arduino GND
```

#### 릴레이 제어 신호
```
Relay IN1 → Arduino D3 (Pump1)
Relay IN2 → Arduino D4 (Pump2)
Relay IN3 → Arduino D5 (Fan1)
Relay IN4 → Arduino D6 (Fan2)
Relay IN5 → Arduino D7 (Fan3)
Relay IN6 → Arduino D8 (Fan4)
Relay IN7 → Arduino D9 (LED)
```

#### 고전력 장치 연결
```
12V+ → Relay COM
Relay NO → 펌프/팬/LED 양극
장치 음극 → 12V-
```

### 3단계: LCD 연결

#### I2C LCD 연결
```
LCD VCC → Arduino 5V
LCD GND → Arduino GND
LCD SDA → Arduino SDA (A4 on Uno)
LCD SCL → Arduino SCL (A5 on Uno)
```

#### I2C 주소 확인
```cpp
// I2C 스캐너 코드
#include <Wire.h>

void setup() {
  Wire.begin();
  Serial.begin(9600);
  Serial.println("I2C Scanner");
}

void loop() {
  byte error, address;
  int nDevices;
  
  Serial.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address<16) Serial.print("0");
      Serial.println(address,HEX);
      nDevices++;
    }
  }
  if (nDevices == 0) Serial.println("No I2C devices found");
  else Serial.println("done");
  delay(5000);
}
```

### 4단계: 스텝모터 연결

#### 스텝모터 드라이버 연결
```
Driver VDD → Arduino 5V
Driver GND → Arduino GND
Driver VMOT → 12V+
Driver GND → 12V-
Driver STEP → Arduino D10
Driver DIR → Arduino D11
Driver ENABLE → Arduino D12
```

#### 스텝모터 와이어 연결
```
A4988 기준:
1A → 스텝모터 코일1+
1B → 스텝모터 코일1-
2A → 스텝모터 코일2+
2B → 스텝모터 코일2-
```

### 5단계: ESP32 통신 연결

#### ESP32 연결
```
ESP32 TX → Arduino A2 (SoftwareSerial RX)
ESP32 RX → Arduino A3 (SoftwareSerial TX)
ESP32 GND → Arduino GND
ESP32 VCC → Arduino 5V (또는 별도 3.3V 공급)
```

## 전원 공급 시스템

### 전원 분배
```
12V 10A 파워서플라이
├── 12V → 펌프, 팬, LED, 스텝모터
├── 5V 레귤레이터 → Arduino, 센서, LCD
└── GND → 공통 그라운드
```

### 전원 소비량 계산
```
Arduino + 센서: 500mA @ 5V
펌프 2개: 2A @ 12V
팬 4개: 2A @ 12V
LED: 2A @ 12V
스텝모터: 1A @ 12V
릴레이 모듈: 200mA @ 5V
LCD: 100mA @ 5V

총 소비전력: 약 84W
권장 파워서플라이: 12V 10A (120W)
```

## 캘리브레이션 방법

### 토양수분센서 캘리브레이션
```cpp
// 건조 상태에서 읽기
int dryValue = analogRead(A0);
Serial.print("Dry: ");
Serial.println(dryValue);

// 물에 담근 상태에서 읽기
int wetValue = analogRead(A0);
Serial.print("Wet: ");
Serial.println(wetValue);

// 임계값 설정
const int OPTIMAL_SOIL_MOISTURE_MIN = wetValue + (dryValue - wetValue) * 0.3;
const int OPTIMAL_SOIL_MOISTURE_MAX = wetValue + (dryValue - wetValue) * 0.7;
```

### 조도센서 캘리브레이션
```cpp
// 어두운 환경에서 읽기
int darkValue = analogRead(A1);
Serial.print("Dark: ");
Serial.println(darkValue);

// 밝은 환경에서 읽기
int brightValue = analogRead(A1);
Serial.print("Bright: ");
Serial.println(brightValue);

// 임계값 설정
const int OPTIMAL_LIGHT_MIN = darkValue + (brightValue - darkValue) * 0.3;
const int OPTIMAL_LIGHT_MAX = darkValue + (brightValue - darkValue) * 0.7;
```

### 스텝모터 캘리브레이션
```cpp
// 스텝모터 정밀도 테스트
void testStepperPrecision() {
  for(int i = 0; i < 200; i++) {  // 한 바퀴 회전
    stepper.step(1);
    delay(10);
  }
  delay(1000);
  
  for(int i = 0; i < 200; i++) {  // 역방향 한 바퀴
    stepper.step(-1);
    delay(10);
  }
}
```

## 안전 고려사항

### 전기적 안전
- 모든 고전력 장치는 릴레이를 통해 제어
- 적절한 퓨즈 설치
- 접지 연결 확인
- 단락 보호 회로 구성

### 방수 처리
- 센서 방수 처리
- 전선 연결부 실리콘 처리
- 제어 박스 방수 등급 IP65 이상

### 기계적 안전
- 리니어 가이드 리미트 스위치 설치
- 펌프 드라이 런 보호
- 팬 블레이드 보호 커버

## 문제 해결 체크리스트

### 센서 문제
□ 전원 공급 확인 (5V, GND)
□ 핀 연결 확인
□ 센서 동작 전압 확인
□ 풀업 저항 확인 (필요시)

### LCD 문제
□ I2C 주소 확인
□ SDA, SCL 핀 연결 확인
□ 백라이트 점등 확인
□ 라이브러리 호환성 확인

### 릴레이 문제
□ 릴레이 모듈 전원 확인
□ 신호 레벨 확인 (5V)
□ 릴레이 동작 LED 확인
□ 부하 전원 확인 (12V)

### 통신 문제
□ ESP32 전원 확인
□ TX/RX 핀 교차 연결 확인
□ 보드레이트 일치 확인
□ 공통 GND 연결 확인

## 성능 최적화 팁

### 노이즈 감소
- 센서 라인과 전원 라인 분리
- 페라이트 코어 사용
- 적절한 캐패시터 사용

### 안정성 향상
- 와치독 타이머 구현
- 전원 감시 회로
- 백업 전원 시스템

### 확장성 고려
- 예비 핀 확보
- 모듈화 설계
- 표준 커넥터 사용