# 스마트팜 시스템 Makefile
# PlatformIO 기반 빌드 자동화

# 기본 설정
PROJECT_NAME = SmartFarm
VERSION = 2.0.0
BOARD = uno
ENV = uno

# 색상 정의
RED = \033[0;31m
GREEN = \033[0;32m
YELLOW = \033[1;33m
BLUE = \033[0;34m
NC = \033[0m # No Color

# 기본 타겟
.DEFAULT_GOAL := help

# 도움말
help: ## 사용 가능한 명령어 표시
	@echo "$(BLUE)🌱 스마트팜 시스템 빌드 도구 v$(VERSION)$(NC)"
	@echo ""
	@echo "$(YELLOW)사용법:$(NC)"
	@echo "  make <target>"
	@echo ""
	@echo "$(YELLOW)주요 명령어:$(NC)"
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?## / {printf "  $(GREEN)%-15s$(NC) %s\n", $$1, $$2}' $(MAKEFILE_LIST)
	@echo ""
	@echo "$(YELLOW)환경:$(NC)"
	@echo "  BOARD: $(BOARD)"
	@echo "  ENV: $(ENV)"

# 빌드 관련
build: ## 프로젝트 빌드
	@echo "$(YELLOW)🔨 프로젝트 빌드 중...$(NC)"
	pio run -e $(ENV)
	@echo "$(GREEN)✅ 빌드 완료$(NC)"

clean: ## 빌드 파일 정리
	@echo "$(YELLOW)🧹 빌드 파일 정리 중...$(NC)"
	pio run -t clean
	@echo "$(GREEN)✅ 정리 완료$(NC)"

rebuild: clean build ## 클린 빌드

# 업로드 관련
upload: ## 펌웨어 업로드
	@echo "$(YELLOW)📤 펌웨어 업로드 중...$(NC)"
	pio run -e $(ENV) -t upload
	@echo "$(GREEN)✅ 업로드 완료$(NC)"

monitor: ## 시리얼 모니터 시작
	@echo "$(YELLOW)📺 시리얼 모니터 시작...$(NC)"
	pio device monitor

upload-monitor: upload monitor ## 업로드 후 모니터링

# 테스트 관련
test: ## 모든 테스트 실행
	@echo "$(YELLOW)🧪 테스트 실행 중...$(NC)"
	pio test -e test
	@echo "$(GREEN)✅ 테스트 완료$(NC)"

test-sensors: ## 센서 테스트만 실행
	@echo "$(YELLOW)📊 센서 테스트 실행 중...$(NC)"
	pio test -f test_sensors
	@echo "$(GREEN)✅ 센서 테스트 완료$(NC)"

test-actuators: ## 액추에이터 테스트만 실행
	@echo "$(YELLOW)⚙️ 액추에이터 테스트 실행 중...$(NC)"
	pio test -f test_actuators
	@echo "$(GREEN)✅ 액추에이터 테스트 완료$(NC)"

test-scheduler: ## 스케줄러 테스트만 실행
	@echo "$(YELLOW)⏰ 스케줄러 테스트 실행 중...$(NC)"
	pio test -f test_scheduler
	@echo "$(GREEN)✅ 스케줄러 테스트 완료$(NC)"

test-verbose: ## 상세 테스트 실행
	@echo "$(YELLOW)🔍 상세 테스트 실행 중...$(NC)"
	pio test -v
	@echo "$(GREEN)✅ 상세 테스트 완료$(NC)"

# 개발 환경
setup: ## 개발 환경 설정
	@echo "$(YELLOW)🛠️ 개발 환경 설정 중...$(NC)"
	@echo "PlatformIO 설치 확인..."
	@which pio > /dev/null || (echo "$(RED)❌ PlatformIO가 설치되지 않았습니다$(NC)" && exit 1)
	@echo "라이브러리 의존성 설치..."
	pio lib install
	@echo "$(GREEN)✅ 개발 환경 설정 완료$(NC)"

check: ## 코드 품질 검사
	@echo "$(YELLOW)🔍 코드 품질 검사 중...$(NC)"
	pio check
	@echo "$(GREEN)✅ 검사 완료$(NC)"

format: ## 코드 포맷팅
	@echo "$(YELLOW)✨ 코드 포맷팅 중...$(NC)"
	find src include -name "*.cpp" -o -name "*.h" | xargs clang-format -i
	@echo "$(GREEN)✅ 포맷팅 완료$(NC)"

# 문서화
docs: ## 문서 생성
	@echo "$(YELLOW)📚 문서 생성 중...$(NC)"
	doxygen Doxyfile 2>/dev/null || echo "$(YELLOW)⚠️ Doxygen이 설치되지 않음$(NC)"
	@echo "$(GREEN)✅ 문서 생성 완료$(NC)"

# 배포 관련
package: build ## 배포 패키지 생성
	@echo "$(YELLOW)📦 배포 패키지 생성 중...$(NC)"
	@mkdir -p dist
	@cp .pio/build/$(ENV)/firmware.hex dist/$(PROJECT_NAME)-v$(VERSION).hex
	@cp .pio/build/$(ENV)/firmware.elf dist/$(PROJECT_NAME)-v$(VERSION).elf
	@echo "패키지 내용:" > dist/package-info.txt
	@echo "프로젝트: $(PROJECT_NAME)" >> dist/package-info.txt
	@echo "버전: $(VERSION)" >> dist/package-info.txt
	@echo "빌드 날짜: $$(date)" >> dist/package-info.txt
	@echo "보드: $(BOARD)" >> dist/package-info.txt
	@echo "$(GREEN)✅ 패키지 생성 완료: dist/$(NC)"

release: clean test package ## 릴리즈 빌드 (테스트 + 패키징)
	@echo "$(GREEN)🚀 릴리즈 v$(VERSION) 준비 완료$(NC)"

# 환경별 빌드
build-uno: ## Arduino Uno용 빌드
	$(MAKE) build ENV=uno BOARD=uno

build-esp32: ## ESP32용 빌드
	$(MAKE) build ENV=esp32 BOARD=esp32dev

build-debug: ## 디버그 모드 빌드
	$(MAKE) build ENV=debug

build-release: ## 릴리즈 모드 빌드
	$(MAKE) build ENV=release

# 시뮬레이션
simulate: ## 시뮬레이션 실행
	@echo "$(YELLOW)🖥️ 시뮬레이션 실행 중...$(NC)"
	pio run -e simulation
	@echo "$(GREEN)✅ 시뮬레이션 완료$(NC)"

# 유지보수
backup: ## 프로젝트 백업
	@echo "$(YELLOW)💾 프로젝트 백업 중...$(NC)"
	@mkdir -p backup
	@tar -czf backup/$(PROJECT_NAME)-backup-$$(date +%Y%m%d-%H%M%S).tar.gz \
		--exclude='.pio' --exclude='backup' --exclude='.git' .
	@echo "$(GREEN)✅ 백업 완료: backup/$(NC)"

restore: ## 백업에서 복원 (사용 시 주의)
	@echo "$(RED)⚠️ 이 작업은 현재 파일을 덮어쓸 수 있습니다$(NC)"
	@read -p "계속하시겠습니까? [y/N] " confirm && [ "$$confirm" = "y" ]
	@echo "$(YELLOW)📥 복원할 백업 파일을 선택하세요:$(NC)"
	@ls -la backup/

clean-all: clean ## 모든 생성 파일 삭제
	@echo "$(YELLOW)🗑️ 모든 생성 파일 삭제 중...$(NC)"
	rm -rf .pio
	rm -rf dist
	rm -rf docs/html
	rm -rf backup
	@echo "$(GREEN)✅ 삭제 완료$(NC)"

# 정보 출력
info: ## 프로젝트 정보 출력
	@echo "$(BLUE)📊 프로젝트 정보$(NC)"
	@echo "이름: $(PROJECT_NAME)"
	@echo "버전: $(VERSION)"
	@echo "보드: $(BOARD)"
	@echo "환경: $(ENV)"
	@echo ""
	@echo "$(BLUE)📁 디렉토리 구조$(NC)"
	tree -I '.pio|.git|backup|dist' -L 2 || ls -la

status: ## Git 상태 및 빌드 정보
	@echo "$(BLUE)📋 상태 정보$(NC)"
	@echo ""
	@echo "$(YELLOW)Git 상태:$(NC)"
	git status --short 2>/dev/null || echo "Git 저장소가 아닙니다"
	@echo ""
	@echo "$(YELLOW)빌드 상태:$(NC)"
	@[ -f .pio/build/$(ENV)/firmware.hex ] && echo "✅ 펌웨어 빌드됨" || echo "❌ 펌웨어 미빌드"
	@[ -d .pio/libdeps ] && echo "✅ 라이브러리 설치됨" || echo "❌ 라이브러리 미설치"

# 고급 기능
memory: ## 메모리 사용량 분석
	@echo "$(YELLOW)🧠 메모리 사용량 분석 중...$(NC)"
	pio run -e memory_profile -t size

benchmark: ## 성능 벤치마크
	@echo "$(YELLOW)⚡ 성능 벤치마크 실행 중...$(NC)"
	@echo "빌드 시간 측정..."
	@time $(MAKE) clean > /dev/null 2>&1
	@time $(MAKE) build > /dev/null 2>&1

lint: ## 코드 린팅
	@echo "$(YELLOW)🔍 코드 린팅 중...$(NC)"
	pio check --flags "--enable=all"

# OTA 업데이트 (ESP32용)
ota: ## OTA 무선 업데이트
	@echo "$(YELLOW)📡 OTA 업데이트 중...$(NC)"
	pio run -e ota -t upload
	@echo "$(GREEN)✅ OTA 업데이트 완료$(NC)"

# 개발자 도구
install-tools: ## 개발 도구 설치
	@echo "$(YELLOW)🛠️ 개발 도구 설치 중...$(NC)"
	@echo "clang-format 설치 확인..."
	@which clang-format > /dev/null || echo "$(YELLOW)⚠️ clang-format을 설치하세요$(NC)"
	@echo "doxygen 설치 확인..."
	@which doxygen > /dev/null || echo "$(YELLOW)⚠️ doxygen을 설치하세요$(NC)"
	@echo "tree 설치 확인..."
	@which tree > /dev/null || echo "$(YELLOW)⚠️ tree를 설치하세요$(NC)"

# 편의 기능
quick: build upload monitor ## 빌드 + 업로드 + 모니터링
dev: clean build test ## 개발용 (클린 + 빌드 + 테스트)
ci: clean build test package ## CI용 (전체 파이프라인)

# Phony 타겟 정의
.PHONY: help build clean rebuild upload monitor upload-monitor test test-sensors test-actuators test-scheduler test-verbose setup check format docs package release build-uno build-esp32 build-debug build-release simulate backup restore clean-all info status memory benchmark lint ota install-tools quick dev ci