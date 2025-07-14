# 🌱 Smart Farm Lettuce Cultivation System v2.0

A fully automated Arduino-based smart farming system designed for optimal lettuce cultivation with real-time environmental control and monitoring.

## 📋 Table of Contents

- [System Overview](#system-overview)
- [Hardware Components](#hardware-components)
- [Software Architecture](#software-architecture)
- [Installation & Setup](#installation--setup)
- [Usage Guide](#usage-guide)
- [API Documentation](#api-documentation)
- [Testing](#testing)
- [Communication](#communication)

## 🎯 System Overview

### Key Features
- **Automated Environmental Control**: Real-time monitoring and control of temperature, humidity, and light
- **Precision Scheduling**: RTC module-based accurate time management
- **Remote Monitoring**: Real-time data transmission via ESP32
- **Adaptive Control**: Automatic adjustment based on environmental changes
- **Safety Systems**: Emergency stop and error recovery capabilities
- **Modular Architecture**: Easily extensible and maintainable codebase

### Optimal Growing Environment
- **Temperature**: 15°C ~ 20°C (59°F ~ 68°F)
- **Humidity**: 60% ~ 80%
- **Light Level**: 300 ~ 600 (analog reading)
- **Lighting Duration**: 16 hours/day (06:00-22:00)

### Automated Schedule
| Time  | Action | Description |
|-------|--------|-------------|
| 06:00 | LED ON | Start 16-hour lighting cycle |
| 08:00 | Watering 1 | First daily watering (30s) |
| 10:00 | Plant Movement 1 | First position adjustment |
| 15:00 | Plant Movement 2 | Second position adjustment |
| 18:00 | Watering 2 | Second daily watering (30s) |
| 22:00 | LED OFF | Night mode begins |

## 🔧 Hardware Components

### Main Controller
- **Arduino Uno R3** - Main microcontroller
- **DS1307 RTC Module** - Real-time clock with battery backup

### Sensors
- **DHT22** - Temperature and humidity sensor
- **LDR (Light Dependent Resistor)** - Light intensity sensor

### Actuators
- **DC380 Water Pumps × 2** - Automated irrigation system
- **12V Cooling Fans × 4** - Temperature and humidity control
- **LED Grow Lights** - Photosynthesis support
- **NEMA17 Stepper Motor** - Linear actuator for plant positioning

### Communication
- **ESP32 Module** - WiFi/Bluetooth connectivity
- **SoftwareSerial** - UART communication

### Connection Diagram

```
Arduino Uno    →    Component
═══════════════════════════════════
Digital Pin 2  →    DHT22 Data
Digital Pin 3  →    Water Pump 1
Digital Pin 4  →    Water Pump 2
Digital Pin 5  →    Cooling Fan 1
Digital Pin 6  →    Cooling Fan 2
Digital Pin 7  →    Cooling Fan 3
Digital Pin 8  →    Cooling Fan 4
Digital Pin 9  →    LED Grow Light
Digital Pin 10 →    Stepper STEP
Digital Pin 11 →    Stepper DIR
Digital Pin 12 →    Stepper ENABLE
Digital Pin 13 →    Status LED
Analog Pin A1  →    LDR Light Sensor
Analog Pin A2  →    ESP32 RX
Analog Pin A3  →    ESP32 TX
SDA (A4)       →    RTC SDA
SCL (A5)       →    RTC SCL
```

## 🏗️ Software Architecture

### Project Structure

```
SmartFarm/
├── platformio.ini              # PlatformIO configuration
├── Makefile                   # Build automation
├── README.md                  # Project documentation
├── include/                   # Header files
│   ├── config.h              # System configuration
│   ├── sensors.h             # Sensor manager class
│   ├── actuators.h           # Actuator control class
│   ├── scheduler.h           # Scheduling class
│   ├── communication.h       # Communication manager
│   └── smartfarm.h           # Main system class
├── src/                      # Source files
│   ├── main.cpp              # Main application
│   ├── sensors.cpp           # Sensor implementations
│   ├── actuators.cpp         # Actuator implementations
│   ├── scheduler.cpp         # Scheduler implementations
│   ├── communication.cpp     # Communication implementations
│   └── smartfarm.cpp         # Main system implementation
├── lib/                      # Custom libraries
│   └── utilities/            # Utility library
├── test/                     # Test files
└── data/                     # Configuration files
```

## 🚀 Installation & Setup

### 1. Development Environment

#### Install PlatformIO
```bash
# Install VS Code extension
1. Open VS Code
2. Go to Extensions tab
3. Search for "PlatformIO IDE" and install
4. Reload VS Code and verify PlatformIO icon appears
```

#### Clone Repository
```bash
git clone https://github.com/your-repo/smart-farm.git
cd smart-farm
```

### 2. Hardware Setup

#### Power Connections
- Arduino: USB or DC 7-12V
- Actuators: Separate 12V adapter recommended
- Sensors: Arduino 5V/3.3V pins

⚠️ **Safety**: Use separate power supplies for motors and sensors. Ensure common ground connections.

### 3. Software Configuration

#### Library Dependencies
```ini
lib_deps = 
    adafruit/RTClib@^2.1.1
    adafruit/DHT sensor library@^1.4.4
    paulstoffregen/SoftwareSerial@^1.0
    arduino-libraries/Stepper@^1.1.3
```

## 📱 Usage Guide

### 1. System Startup

```bash
# Build and upload
pio run -t upload

# Monitor serial output
pio device monitor

# Using Makefile
make upload-monitor
```

### 2. System Status Monitoring

```
========== System Status ==========
Current Time: 14:25:30
Current Date: 2025-07-14
Temperature: 18.5°C (Optimal: 15.0-20.0°C)
Humidity: 65.2% (Optimal: 60.0-80.0%)
Light Level: 450 (Optimal: 300-600)
Control Status - Pump: OFF, Fan: OFF, LED: ON
Today's Tasks - Watering1: Complete, Watering2: Pending
Overall Environment: 🌱 Optimal
================================
```

### 3. Build Commands

```bash
make help           # Show available commands
make setup          # Setup development environment
make build          # Build project
make upload         # Upload firmware
make monitor        # Start serial monitor
make test           # Run all tests
```

## 🔌 API Documentation

### ESP32 JSON Data Format

```json
{
  "temperature": 18.50,
  "humidity": 65.20,
  "lightLevel": 450,
  "pumpStatus": false,
  "fanStatus": false,
  "ledStatus": true,
  "wateringActive": false,
  "currentTime": "14:25:30",
  "currentDate": "2025-07-14",
  "watering1Done": true,
  "watering2Done": false,
  "tempStatus": "OK",
  "humidityStatus": "OK",
  "lightStatus": "OK",
  "overallStatus": "OPTIMAL",
  "timestamp": 1721289930
}
```

### Control Commands

| Command | Parameters | Description |
|---------|------------|-------------|
| `led` | `on/off`, `brightness:0-255` | LED control |
| `pump` | `on/off`, `duration:seconds` | Pump control |
| `fan` | `on/off`, `speed:0-255` | Fan control |
| `emergency` | `stop` | Emergency shutdown |

## 🧪 Testing

### Running Tests

```bash
# Run all tests
make test
# or
pio test

# Run specific test modules
make test-sensors
make test-actuators
make test-scheduler
```

### Test Coverage
- ✅ Sensor initialization
- ✅ Data reading validation
- ✅ RTC functionality
- ✅ Actuator control
- ✅ Emergency stop functionality

## 🐛 Troubleshooting

### Common Issues

#### 1. RTC Time Incorrect
```cpp
rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
```

#### 2. DHT Sensor Reading Errors
- Verify 5V and GND connections
- Check data pin connection
- Add pull-up resistor (4.7kΩ)

#### 3. ESP32 Communication Failure
- Verify 9600 bps setting
- Check RX/TX pin connections

#### 4. Memory Overflow
- Minimize String usage
- Use F() macro for constants
- Optimize buffer sizes

## 🤝 Contributing

### Development Guidelines

1. **Fork** the repository
2. **Create** a feature branch
3. **Follow** coding standards
4. **Add** comprehensive tests
5. **Submit** a pull request

### Coding Standards

```cpp
// Class names: PascalCase
class SensorManager { }

// Function names: camelCase  
void readSensors() { }

// Variables: camelCase
float currentTemperature;

// Constants: UPPER_SNAKE_CASE
const int MAX_RETRY_COUNT = 3;
```
---------------------
---------------
-----------------
----------
