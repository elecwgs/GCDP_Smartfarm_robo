#example로! 

#include <WiFi.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <TimeLib.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

// WiFi 설정
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* blynk_token = "YOUR_BLYNK_TOKEN";

// NTP 설정
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 32400); // 한국 시간 (UTC+9)

// 핀 설정
#define DHT_PIN 4
#define DHT_TYPE DHT22
#define WATER_TEMP_PIN 2
#define LIGHT_SENSOR_PIN A1
#define WATER_PUMP_RELAY_PIN 5     // 물 펌프
#define NUTRIENT_PUMP_RELAY_PIN 6  // 영양제 펌프
#define FAN_RELAY_PIN 7            // 환기팬
#define LED_STRIP_PIN 8            // LED 조명
#define PH_SENSOR_PIN A2
#define TDS_SENSOR_PIN A3
#define WATER_LEVEL_PIN A4         // 물 탱크 수위
#define NUTRIENT_LEVEL_PIN A5      // 영양제 탱크 수위
#define HUMIDITY_SENSOR_PIN A6     // 스펀지 습도 센서 (저항 방식)

// 센서 객체 생성
DHT dht(DHT_PIN, DHT_TYPE);
OneWire oneWire(WATER_TEMP_PIN);
DallasTemperature waterTempSensor(&oneWire);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// 상추 NFT 수경재배 최적 환경 설정값
const float OPTIMAL_AIR_TEMP_MIN = 18.0;    // 공기 온도 18-22°C
const float OPTIMAL_AIR_TEMP_MAX = 22.0;
const float OPTIMAL_HUMIDITY_MIN = 60.0;    // 습도 60-70%
const float OPTIMAL_HUMIDITY_MAX = 70.0;
const float OPTIMAL_WATER_TEMP_MIN = 16.0;  // 양액 온도 16-20°C
const float OPTIMAL_WATER_TEMP_MAX = 20.0;
const float OPTIMAL_PH_MIN = 5.5;           // pH 5.5-6.5
const float OPTIMAL_PH_MAX = 6.5;
const int OPTIMAL_TDS_MIN = 800;            // TDS 800-1200 ppm
const int OPTIMAL_TDS_MAX = 1200;

// 자동화 스케줄 설정
const int LED_ON_HOUR = 6;                  // LED 켜는 시간 (오전 6시)
const int LED_OFF_HOUR = 20;                // LED 끄는 시간 (오후 8시)
const int NIGHT_FAN_HOUR = 22;              // 밤 환기 시간 (오후 10시)
const int MORNING_FAN_HOUR = 6;             // 아침 환기 시간 (오전 6시)
const float HOT_TEMP_THRESHOLD = 25.0;      // 더우면 팬 돌리기 기준 온도

// 급수 설정
const int WATERING_INTERVAL = 60;           // 60분마다 급수 체크
const int WATER_PUMP_DURATION = 30;         // 물 공급 30초
const int NUTRIENT_PUMP_DURATION = 10;      // 영양제 공급 10초
const int SPONGE_DRY_THRESHOLD = 30;        // 스펀지 습도 30% 이하면 급수
const int SPONGE_WET_THRESHOLD = 70;        // 스펀지 습도 70% 이상이면 급수 중지

// 변수 선언
float airTemp, airHumidity, waterTemp, pH, tds;
int lightLevel, waterLevel, nutrientLevel, spongeHumidity;
unsigned long previousMillis = 0;
unsigned long wateringMillis = 0;
unsigned long fanMillis = 0;
const long sensorInterval = 5000;           // 5초마다 센서 체크
const long wateringIntervalMs = WATERING_INTERVAL * 60 * 1000;

// 시스템 상태
bool waterPumpStatus = false;
bool nutrientPumpStatus = false;
bool fanStatus = false;
bool ledStatus = false;
bool isDayTime = false;
bool isWatering = false;
bool autoFanMode = true;

void setup() {
  Serial.begin(115200);
  
  // 센서 초기화
  dht.begin();
  waterTempSensor.begin();
  lcd.init();
  lcd.backlight();
  
  // 릴레이 핀 설정
  pinMode(WATER_PUMP_RELAY_PIN, OUTPUT);
  pinMode(NUTRIENT_PUMP_RELAY_PIN, OUTPUT);
  pinMode(FAN_RELAY_PIN, OUTPUT);
  pinMode(LED_STRIP_PIN, OUTPUT);
  
  // 초기 상태 OFF
  digitalWrite(WATER_PUMP_RELAY_PIN, LOW);
  digitalWrite(NUTRIENT_PUMP_RELAY_PIN, LOW);
  digitalWrite(FAN_RELAY_PIN, LOW);
  digitalWrite(LED_STRIP_PIN, LOW);
  
  // WiFi 연결
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("WiFi 연결 중...");
  }
  Serial.println("WiFi 연결됨!");
  
  // NTP 시간 동기화
  timeClient.begin();
  timeClient.update();
  
  // Blynk 초기화
  Blynk.begin(blynk_token, ssid, password);
  
  // LCD 초기 메시지
  lcd.setCursor(0, 0);
  lcd.print("NFT Lettuce Farm");
  lcd.setCursor(0, 1);
  lcd.print("System Ready");
  delay(2000);
}

void loop() {
  Blynk.run();
  timeClient.update();
  
  unsigned long currentMillis = millis();
  
  // 센서 데이터 읽기 (5초마다)
  if (currentMillis - previousMillis >= sensorInterval) {
    previousMillis = currentMillis;
    
    readSensors();
    controlLighting();
    controlVentilation();
    displayLCD();
    printSensorData();
    sendToBlynk();
    checkAlerts();
  }
  
  // 급수 제어
  controlWatering(currentMillis);
}

void readSensors() {
  // 공기 온습도 센서
  airTemp = dht.readTemperature();
  airHumidity = dht.readHumidity();
  
  // 양액 온도
  waterTempSensor.requestTemperatures();
  waterTemp = waterTempSensor.getTempCByIndex(0);
  
  // 조도 센서
  lightLevel = analogRead(LIGHT_SENSOR_PIN);
  
  // pH 센서
  int phRaw = analogRead(PH_SENSOR_PIN);
  pH = map(phRaw, 0, 4095, 0, 14);
  
  // TDS 센서 (영양분 농도)
  int tdsRaw = analogRead(TDS_SENSOR_PIN);
  tds = map(tdsRaw, 0, 4095, 0, 2000);
  
  // 물 탱크 수위
  int waterLevelRaw = analogRead(WATER_LEVEL_PIN);
  waterLevel = map(waterLevelRaw, 0, 4095, 0, 100);
  
  // 영양제 탱크 수위
  int nutrientLevelRaw = analogRead(NUTRIENT_LEVEL_PIN);
  nutrientLevel = map(nutrientLevelRaw, 0, 4095, 0, 100);
  
  // 스펀지 습도 센서 (저항 방식)
  int spongeRaw = analogRead(HUMIDITY_SENSOR_PIN);
  spongeHumidity = map(spongeRaw, 0, 4095, 100, 0); // 저항이 높을수록 건조함
}

void controlLighting() {
  int currentHour = timeClient.getHours();
  
  // 낮/밤 판단
  isDayTime = (currentHour >= LED_ON_HOUR && currentHour < LED_OFF_HOUR);
  
  if (isDayTime) {
    // 낮 시간: 자연광 부족시에만 LED 켜기
    if (lightLevel < 3000) {
      digitalWrite(LED_STRIP_PIN, HIGH);
      ledStatus = true;
    } else {
      digitalWrite(LED_STRIP_PIN, LOW);
      ledStatus = false;
    }
  } else {
    // 밤 시간: LED 끄기
    digitalWrite(LED_STRIP_PIN, LOW);
    ledStatus = false;
  }
}

void controlVentilation() {
  int currentHour = timeClient.getHours();
  bool shouldRunFan = false;
  
  if (autoFanMode) {
    // 온도가 높으면 팬 돌리기
    if (airTemp > HOT_TEMP_THRESHOLD) {
      shouldRunFan = true;
    }
    
    // 낮 시간 환기 (더울 때)
    if (isDayTime && airTemp > OPTIMAL_AIR_TEMP_MAX) {
      shouldRunFan = true;
    }
    
    // 밤 시간 환기 (습도 조절)
    if (!isDayTime && airHumidity > OPTIMAL_HUMIDITY_MAX) {
      shouldRunFan = true;
    }
    
    // 정해진 시간에 환기 (아침, 저녁)
    if (currentHour == MORNING_FAN_HOUR || currentHour == NIGHT_FAN_HOUR) {
      shouldRunFan = true;
    }
  }
  
  digitalWrite(FAN_RELAY_PIN, shouldRunFan);
  fanStatus = shouldRunFan;
}

void controlWatering(unsigned long currentMillis) {
  // 스펀지 습도 기반 급수 제어
  if (!isWatering && spongeHumidity < SPONGE_DRY_THRESHOLD) {
    startWatering();
  } else if (isWatering && spongeHumidity > SPONGE_WET_THRESHOLD) {
    stopWatering();
  }
  
  // 정기 급수 (1시간마다)
  if (currentMillis - wateringMillis >= wateringIntervalMs) {
    wateringMillis = currentMillis;
    if (spongeHumidity < SPONGE_WET_THRESHOLD) {
      startWatering();
    }
  }
}

void startWatering() {
  if (waterLevel < 20 || nutrientLevel < 20) {
    Serial.println("⚠️ 탱크 수위 부족! 급수 중단");
    return;
  }
  
  isWatering = true;
  
  // 물 공급 (30초)
  digitalWrite(WATER_PUMP_RELAY_PIN, HIGH);
  waterPumpStatus = true;
  Serial.println("💧 물 공급 시작");
  
  delay(WATER_PUMP_DURATION * 1000);
  
  // 영양제 공급 (10초)
  digitalWrite(NUTRIENT_PUMP_RELAY_PIN, HIGH);
  nutrientPumpStatus = true;
  Serial.println("🧪 영양제 공급 시작");
  
  delay(NUTRIENT_PUMP_DURATION * 1000);
  
  // 급수 완료
  digitalWrite(WATER_PUMP_RELAY_PIN, LOW);
  digitalWrite(NUTRIENT_PUMP_RELAY_PIN, LOW);
  waterPumpStatus = false;
  nutrientPumpStatus = false;
  
  Serial.println("✅ 급수 완료");
  
  // 5분 후 습도 재체크
  delay(5000);
  readSensors();
  
  if (spongeHumidity >= SPONGE_WET_THRESHOLD) {
    isWatering = false;
  }
}

void stopWatering() {
  isWatering = false;
  digitalWrite(WATER_PUMP_RELAY_PIN, LOW);
  digitalWrite(NUTRIENT_PUMP_RELAY_PIN, LOW);
  waterPumpStatus = false;
  nutrientPumpStatus = false;
  Serial.println("🛑 급수 중지 - 충분한 수분");
}

void displayLCD() {
  lcd.clear();
  
  // 낮/밤 표시
  lcd.setCursor(0, 0);
  if (isDayTime) {
    lcd.print("DAY ");
  } else {
    lcd.print("NIGHT ");
  }
  
  lcd.print("T:");
  lcd.print(airTemp, 1);
  lcd.print("C");
  
  lcd.setCursor(0, 1);
  lcd.print("Sponge:");
  lcd.print(spongeHumidity);
  lcd.print("%");
  
  if (isWatering) {
    lcd.setCursor(13, 1);
    lcd.print("W");
  }
  
  delay(2000);
  
  // 두 번째 화면
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("pH:");
  lcd.print(pH, 1);
  lcd.print(" TDS:");
  lcd.print(tds, 0);
  
  lcd.setCursor(0, 1);
  lcd.print("H2O:");
  lcd.print(waterLevel);
  lcd.print("% NUT:");
  lcd.print(nutrientLevel);
  lcd.print("%");
}

void printSensorData() {
  Serial.println("=== NFT 상추 수경재배 센서 데이터 ===");
  Serial.print("현재 시간: ");
  Serial.print(timeClient.getHours());
  Serial.print(":");
  Serial.print(timeClient.getMinutes());
  Serial.print(" (");
  Serial.print(isDayTime ? "낮" : "밤");
  Serial.println(")");
  
  Serial.print("공기 온도: ");
  Serial.print(airTemp);
  Serial.println("°C");
  
  Serial.print("공기 습도: ");
  Serial.print(airHumidity);
  Serial.println("%");
  
  Serial.print("양액 온도: ");
  Serial.print(waterTemp);
  Serial.println("°C");
  
  Serial.print("스펀지 습도: ");
  Serial.print(spongeHumidity);
  Serial.println("%");
  
  Serial.print("물 탱크: ");
  Serial.print(waterLevel);
  Serial.println("%");
  
  Serial.print("영양제 탱크: ");
  Serial.print(nutrientLevel);
  Serial.println("%");
  
  Serial.print("조도: ");
  Serial.println(lightLevel);
  
  Serial.print("pH: ");
  Serial.println(pH);
  
  Serial.print("TDS: ");
  Serial.print(tds);
  Serial.println(" ppm");
  
  Serial.println("=== 시스템 상태 ===");
  Serial.print("LED 조명: ");
  Serial.println(ledStatus ? "ON" : "OFF");
  
  Serial.print("환기팬: ");
  Serial.println(fanStatus ? "ON" : "OFF");
  
  Serial.print("물 펌프: ");
  Serial.println(waterPumpStatus ? "ON" : "OFF");
  
  Serial.print("영양제 펌프: ");
  Serial.println(nutrientPumpStatus ? "ON" : "OFF");
  
  Serial.print("급수 모드: ");
  Serial.println(isWatering ? "진행중" : "대기");
  
  Serial.println("=============================");
}

void sendToBlynk() {
  Blynk.virtualWrite(V1, airTemp);
  Blynk.virtualWrite(V2, airHumidity);
  Blynk.virtualWrite(V3, waterTemp);
  Blynk.virtualWrite(V4, spongeHumidity);
  Blynk.virtualWrite(V5, waterLevel);
  Blynk.virtualWrite(V6, nutrientLevel);
  Blynk.virtualWrite(V7, pH);
  Blynk.virtualWrite(V8, tds);
  Blynk.virtualWrite(V9, lightLevel);
  Blynk.virtualWrite(V10, ledStatus);
  Blynk.virtualWrite(V11, fanStatus);
  Blynk.virtualWrite(V12, waterPumpStatus);
  Blynk.virtualWrite(V13, nutrientPumpStatus);
  Blynk.virtualWrite(V14, isDayTime);
}

void checkAlerts() {
  String alertMessage = "";
  
  if (airTemp > HOT_TEMP_THRESHOLD) {
    alertMessage += "고온 경고! ";
  }
  
  if (waterTemp > OPTIMAL_WATER_TEMP_MAX) {
    alertMessage += "양액 온도 높음! ";
  }
  
  if (pH < OPTIMAL_PH_MIN || pH > OPTIMAL_PH_MAX) {
    alertMessage += "pH 이상! ";
  }
  
  if (tds < OPTIMAL_TDS_MIN || tds > OPTIMAL_TDS_MAX) {
    alertMessage += "양액 농도 이상! ";
  }
  
  if (waterLevel < 20) {
    alertMessage += "물 부족! ";
  }
  
  if (nutrientLevel < 20) {
    alertMessage += "영양제 부족! ";
  }
  
  if (spongeHumidity < 20) {
    alertMessage += "스펀지 과건조! ";
  }
  
  if (alertMessage.length() > 0) {
    Serial.println("🚨 " + alertMessage);
    Blynk.logEvent("nft_alert", alertMessage);
  }
}

// Blynk 수동 제어
BLYNK_WRITE(V15) { // 수동 급수
  if (param.asInt()) {
    startWatering();
  }
}

BLYNK_WRITE(V16) { // 팬 자동/수동 모드
  autoFanMode = param.asInt();
}

BLYNK_WRITE(V17) { // 수동 팬 제어
  if (!autoFanMode) {
    bool fanControl = param.asInt();
    digitalWrite(FAN_RELAY_PIN, fanControl);
    fanStatus = fanControl;
  }
}

BLYNK_WRITE(V18) { // 수동 LED 제어
  bool ledControl = param.asInt();
  digitalWrite(LED_STRIP_PIN, ledControl);
  ledStatus = ledControl;