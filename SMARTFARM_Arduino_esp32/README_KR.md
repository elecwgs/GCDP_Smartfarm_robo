# 🌱 스마트팜 상추재배 시스템 v2.0

Arduino 기반의 완전 자동화된 스마트팜 시스템으로 상추를 최적 환경에서 재배합니다.

## 📋 목차

- [시스템 개요](#시스템-개요)
- [하드웨어 구성](#하드웨어-구성)
- [소프트웨어 아키텍처](#소프트웨어-아키텍처)
- [설치 및 설정](#설치-및-설정)
- [사용법](#사용법)
- [API 문서](#api-문서)
- [테스트](#테스트)
- [문제해결](#문제해결)
- [기여하기](#기여하기)

## 🎯 시스템 개요

### 주요 기능
- **자동 환경 제어**: 온도, 습도, 조도 실시간 모니터링 및 제어
- **정시 스케줄링**: RTC 모듈 기반 정확한 시간 관리
- **원격 모니터링**: ESP32를 통한 실시간 데이터 전송
- **적응형 제어**: 환경 변화에 따른 자동 조정
- **안전 시스템**: 비상 정지 및 오류 복구 기능

### 최적 재배 환경
- **온도**: 15°C ~ 20°C
- **습도**: 60% ~ 80%
- **조도**: 300 ~ 600 (아날로그 값)
- **조명 시간**: 16시간/일 (06:00-22:00)

## 🔧 하드웨어 구성

### 메인 컨트롤러
- Arduino Uno R3
- DS1307 RTC 모듈 (실시간 시계)

### 센서
- DHT22 온습도 센서
- LDR 조도 센서

### 액추에이터
- DC380 워터펌프 × 2
- 12V 냉각팬 × 4
- LED 성장등
- NEMA17 스텝모터 (선형 액추에이터)

### 통신
- ESP32 (WiFi/Bluetooth)
- SoftwareSerial 통신

### 연결도

```
Arduino Uno    →    부품
═══════════════════════════════
Digital Pin 2  →    DHT22 데이터
Digital Pin 3  →    워터펌프 1
Digital Pin 4  →    워터펌프 2
Digital Pin 5  →    냉각팬 1
Digital Pin 6  →    냉각팬 2
Digital Pin 7  →    냉각팬 3
Digital Pin 8  →    냉각팬 4
Digital Pin 9  →    LED 성장등
Digital Pin 10 →    스텝모터 STEP
Digital Pin 11 →    스텝모터 DIR
Digital Pin 12 →    스텝모터 ENABLE
Digital Pin 13 →    상태 LED
Analog Pin A1  →    LDR 조도센서
Analog Pin A2  →    ESP32 RX
Analog Pin A3  →    ESP32 TX
SDA (A4)       →    RTC SDA
SCL (A5)       →    RTC SCL
```

## 🏗️ 소프트웨어 아키텍처

### 프로젝트 구조

```
SmartFarm/
├── platformio.ini              # PlatformIO 설정
├── Makefile                   # 빌드 자동화
├── README.md                  # 완전한 프로젝트 문서
├── include/                   # 헤더 파일들
│   ├── config.h              # 시스템 설정
│   ├── sensors.h             # 센서 헤더
│   ├── actuators.h           # 액추에이터 헤더
│   ├── scheduler.h           # 스케줄러 헤더
│   ├── communication.h       # 통신 헤더
│   └── smartfarm.h           # 메인 시스템 헤더
├── src/                      # 구현 파일들
│   ├── main.cpp              # 메인 함수
│   ├── sensors.cpp           # 센서 구현
│   ├── actuators.cpp         # 액추에이터 구현
│   ├── scheduler.cpp         # 스케줄러 구현
│   ├── communication.cpp     # 통신 구현
│   └── smartfarm.cpp         # 메인 시스템 구현
├── lib/                      # 커스텀 라이브러리
│   └── utilities/            # 유틸리티 라이브러리
│       ├── utilities.h       # 유틸리티 헤더
│       ├── utilities.cpp     # 유틸리티 구현
│       └── library.json      # 라이브러리 메타데이터
├── test/                     # 테스트 파일들
│   ├── test_sensors.cpp      # 센서 테스트
│   ├── test_actuators.cpp    # 액추에이터 테스트
│   └── test_scheduler.cpp    # 스케줄러 테스트
└── data/                     # 설정 파일
    └── config.json           # 시스템 설정
```


## 사용법

### 1. 프로젝트 생성

```
mkdir SmartFarm
cd SmartFarm
pio project init --board uno
```

### 2. 파일 생성
위 모든 파일들을 해당 디렉토리에 생성

### 3. 빌드 및 실행
```
# Makefile 사용
make setup          # 개발 환경 설정
make build          # 빌드
make upload         # 업로드
make monitor        # 시리얼 모니터링
make test           # 테스트 실행

# 또는 PlatformIO 직접 사용
pio run -t upload   # 빌드 + 업로드
pio device monitor  # 모니터링
pio test           # 테스트
```


## 개발 워크플로우
```
개발: make dev (클린 + 빌드 + 테스트)
테스트: make test 또는 개별 테스트
배포: make release (전체 파이프라인)
모니터링: make upload-monitor
```

## 완성 기능
- RTC 기반 정확한 시간 관리
- 환경 센서 모니터링 (온도/습도/조도)
- 자동 제어 (펌프/팬/LED/모터)
- 스케줄링 (물주기/조명/식물이동)
- ESP32 통신 (JSON 데이터 전송)
- 오류 처리 (센서 오류, 비상 정지)
- 유틸리티 (로깅, 데이터 저장, 알림)
- 테스트 (단위 테스트, 통합 테스트)
