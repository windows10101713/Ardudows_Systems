# 🖥️ Ardudows Systems

### ESP32-S3 기반 실험적 임베디드 운영체제

### Experimental Embedded Operating System for ESP32-S3

> “ESP32-S3를 어디까지 컴퓨터처럼 만들 수 있을까?”
> “How far can an ESP32-S3 become a computer-like system?”

---

# 📖 소개 / Introduction

Ardudows Systems는 ESP32-S3 기반으로 제작된 실험적 임베디드 운영체제 프로젝트입니다.
Ardudows Systems is an experimental embedded operating system project built for the ESP32-S3 platform.

이 프로젝트는 단순한 Arduino 예제가 아니라, 마이크로컨트롤러 환경에서 운영체제 구조를 구현하는 것을 목표로 합니다.
This project is not just a simple Arduino sketch. It aims to implement operating-system-like structures inside a microcontroller environment.

GUI 시스템, 파일 시스템, 레지스트리, 드라이버 구조, 네트워크 시스템, USB Host, OTA 업데이트 등을 직접 구현하고 있습니다.
GUI systems, file systems, registry systems, driver structures, networking systems, USB Host support, and OTA updates are implemented directly inside the project.

---

# ⚠️ Important Notice / 중요 안내

Ardudows Systems는 매우 실험적인 프로젝트입니다.
Ardudows Systems is a highly experimental project.

일부 기능은 아직 완전히 구현되지 않았거나 정상적으로 동작하지 않을 수 있습니다.
Some features may not be fully implemented or may not work correctly.

특히 다음 기능들은 개발 중이거나 불안정할 수 있습니다:
The following systems may still be unstable or incomplete:

* USB Host
* BLE Features
* Async Networking
* OTA Update
* GUI Components
* File System Operations
* Multi-tasking

ESP32-S3 환경, SPI 속도, 라이브러리 버전 등에 따라 동작이 달라질 수 있습니다.
Behavior may vary depending on ESP32-S3 environment, SPI speed, and library versions.

---

# ✨ 주요 특징 / Features

## 🪟 GUI 시스템 / GUI System

* TFT_eSPI 기반 GUI
* Keyboard-driven interface
* Window-style desktop environment
* JPEG image rendering support

---

## 📂 파일 시스템 / File System

* SD 카드 기반 저장 구조
* Custom directory system
* Registry-style configuration files
* Custom Ardudows file extensions

---

## 🌐 네트워크 기능 / Networking

* Wi-Fi support
* HTTP / HTTPS client
* Async Web Server
* DNS Server
* Socket.IO support

---

## 🔌 하드웨어 기능 / Hardware Features

* USB Host support
* TinyUSB integration
* BLE Scan / BLE Client
* OTA firmware update
* SPI TFT display support
* PS/2 keyboard support

---

# 🧠 프로젝트 목표 / Project Goals

Ardudows Systems의 목표는 ESP32-S3를 단순 IoT 칩이 아닌 “소형 컴퓨터 플랫폼”처럼 활용하는 것입니다.
The goal of Ardudows Systems is to push the ESP32-S3 beyond a simple IoT chip and use it as a mini computer platform.

이 프로젝트는 다음 개념들을 실험합니다:
This project experiments with:

* 🖥️ GUI systems
* ⚡ Lightweight operating system structures
* 📂 File system architecture
* 🌐 Network integration
* 🔌 Hardware abstraction
* 🧠 Embedded desktop environments

---

# 🧩 하드웨어 요구사항 / Hardware Requirements

## 필수 하드웨어 / Required Hardware

* ESP32-S3 N16R8
* SPI TFT Display (ILI9488 Recommended)
* PS/2 Keyboard

## 선택 하드웨어 / Optional Hardware

* SD Card Module
* RTC Module
* USB Devices
* Wi-Fi Network

---

# 📦 사용 라이브러리 / Libraries Used

Ardudows Systems는 많은 라이브러리와 저수준 ESP32 기능을 사용합니다.
Ardudows Systems uses many libraries and low-level ESP32 features.

```cpp
#include <string.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <FS.h>
#include <SD.h>
#include <esp_system.h>
#include <WiFi.h>
#include <Preferences.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Wire.h>
#include <HTTPClient.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEClient.h>
#include <BLEAdvertisedDevice.h>
#include "esp_wifi.h"
#include <esp_netif.h>
#include "type.h"
#include <WiFiClientSecure.h>
#include "router.h"
#include <Update.h>
#include <esp_ota_ops.h>
#include <SocketIoClient.h>
#include <TJpg_Decoder.h>
#include "tusb.h"
#include <math.h>
#include <ESPAsyncWebServer.h>
#include <freertos/semphr.h>
#include <AsyncTCP.h>
#include "usb/usb_host.h"
#include "esp_intr_alloc.h"
#include "esp_log.h"
```

---

# 🧪 개발 환경 / Development Environment

| Component           | Version         |
| ------------------- | --------------- |
| Arduino IDE         | 2.x             |
| ESP32 Board Package | 3.3.8           |
| Board               | ESP32-S3 N16R8  |
| Display             | ILI9488 SPI TFT |
| PSRAM               | Enabled         |

---

# 🔧 Required TFT_eSPI Configuration / 필수 TFT_eSPI 설정

다음 파일을 수정해야 합니다:
You must edit this file:

```text
Arduino/libraries/TFT_eSPI/User_Setup.h
```

권장 설정:
Recommended configuration:

```cpp
// ==========================================================
//  Ardudows TFT Setup
//  ESP32-S3 + ILI9488 (SPI)
// ==========================================================

#define USER_SETUP_INFO "ESP32-S3 + ILI9488 SPI"

// -------------------------------
// Display driver
// -------------------------------
#define ILI9488_DRIVER

// -------------------------------
// SPI pins (ESP32-S3 SAFE PINS)
// -------------------------------

#define TFT_MOSI 11
#define TFT_SCLK 14
#define TFT_CS   10
#define TFT_DC   9
#define TFT_RST  8

//#define TFT_MISO 35

#define SD_CS    7
#define SD_MISO  13
#define SD_MOSI  6
#define SD_SCLK  16

// -------------------------------
// Backlight
// -------------------------------

#define TFT_BL -1
#define TFT_BACKLIGHT_ON HIGH

// -------------------------------
// SPI settings
// -------------------------------

#define SPI_FREQUENCY       40000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY 2500000

// -------------------------------
// ESP32-S3 SPI
// -------------------------------

#define USE_HSPI_PORT

// -------------------------------
// Display size
// -------------------------------

#define DISPLAY_W 480
#define DISPLAY_H 320

// -------------------------------
// Fonts
// -------------------------------

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT

// -------------------------------
// Optional settings
// -------------------------------

//#define TFT_RGB_ORDER TFT_BGR
//#define TFT_INVERSION_ON
```

---

# ⚡ SPI Frequency Warning / SPI 주파수 경고

너무 높은 SPI 속도는 ESP32-S3에서 다음 문제를 유발할 수 있습니다:
Too high SPI frequency may cause:

* 화면 깨짐 / Screen corruption
* SD 카드 오류 / SD card errors
* 부팅 실패 / Boot failure
* 랜덤 크래시 / Random crashes
* 시스템 멈춤 / System freeze

추천 설정:
Recommended setting:

```cpp
#define SPI_FREQUENCY 40000000
```

---

# 🚀 시작 방법 / Getting Started

## 1️⃣ Arduino IDE 설치

Install Arduino IDE

---

## 2️⃣ ESP32 Board Package 설치

Install ESP32 board package

추천 버전:
Recommended version:

```text
ESP32 Board Package 3.3.8
```

---

## 3️⃣ PSRAM 활성화

Enable PSRAM

Arduino IDE 설정:

```text
PSRAM → Enabled
```

---

## 4️⃣ Partition Scheme 설정

Configure Partition Scheme

추천 설정:

```text
16MB Flash
```

---

## 5️⃣ 업로드

Upload firmware to ESP32-S3

---

# 📁 디렉터리 구조 / Directory Structure

```text
/Ardudows
├── System
│   ├── Registry
│   │   ├── User.asf
│   │   ├── Network.asf
│   │   ├── Setup.asf
│   │   ├── Kernel.asf
│   │   └── Boot.asf
│   │
│   ├── Driver
│   │   ├── TFT.adf
│   │   ├── SD.adf
│   │   ├── RTC.adf
│   │   └── Touch.adf
│   │
│   ├── Kernel
│   │   ├── AFK
│   │   │   └── Ardudows.akf
│   │   │
│   │   └── ATK
│   │       └── Ardudows.akf
│   │
│   ├── Log
│   │   ├── Boot.arf
│   │   ├── Dump.arf
│   │   ├── Output.arf
│   │   ├── panic.arf
│   │   ├── Hardware
│   │   └── Software
│   │
│   ├── NetWork
│   │   ├── config.anf
│   │   └── NetCheck.anf
│   │
│   ├── Boot
│   │   └── Boot.abf
│   │
│   ├── API
│   │   └── API.asf
│   │
│   ├── Debug
│   │   └── Debug.asf
│   │
│   ├── Setup
│   │   └── Setup.asf
│   │
│   └── Firmware
│       └── Firmware.asf
│
├── Users
│   └── Administrator
│       ├── Administrator.auf
│       └── UserDATA
│           └── UserDATA.auf
│
├── Programs
│   ├── Programs_X16
│   │   ├── Explorer
│   │   ├── CMD
│   │   ├── NotePad
│   │   ├── Clock
│   │   ├── Calculators
│   │   └── Registry_Editer
│   │
│   └── Programs_X32
│       └── Minecraft_Server
│
├── Assets
│   ├── Image
│   ├── Font
│   ├── Sound
│   └── Other
│
└── Licens
    ├── ReadME.aif
    └── Product.alf
```

# 📖 파일 확장자 / File Extension Types

| Extension | Description                     |
| --------- | ------------------------------- |
| `.asf`    | Ardudows System File            |
| `.adf`    | Ardudows Driver File            |
| `.akf`    | Ardudows Kernel File            |
| `.apf`    | Ardudows Program File           |
| `.arf`    | Ardudows Report / Log File      |
| `.anf`    | Ardudows Network File           |
| `.abf`    | Ardudows Boot File              |
| `.auf`    | Ardudows User File              |
| `.acf`    | Ardudows CD File                |
| `.awf`    | Ardudows Weird / Easteregg File |

---

# 📊 시스템 리소스 사용량 / System Resource Usage

현재 ESP32-S3 N16R8 기준 사용량:
Current usage on ESP32-S3 N16R8:

| Resource      | Usage           |
| ------------- | --------------- |
| Flash Usage   | ~3MB / 16MB     |
| RAM Usage     | ~223KB / 327KB  |
| Display       | 480x320 SPI TFT |
| SPI Frequency | 40MHz           |
| PSRAM         | Enabled         |

---

# ✅ 구현된 기능 / Implemented Features

* [x] GUI System
* [x] SD Card File System
* [x] Registry System
* [x] Wi-Fi Support
* [x] BLE Support
* [x] OTA Update
* [x] USB Host
* [x] Driver System
* [x] Logging System
* [x] Installer System
* [x] Network System

---

# 🚧 예정 기능 / Planned Features

* [ ] Audio System
* [ ] Mouse Support
* [ ] Multi-window GUI
* [ ] Package Manager
* [ ] Advanced Shell
* [ ] App Store-style System
* [ ] ArduCraft Experiment

---

# 🛠️ 문제 해결 / Troubleshooting

## ⚠️ White Screen

TFT 핀 설정을 확인하세요.
Check TFT pin configuration.

---

## ⚠️ SD Card Error

SD 카드 SPI 설정을 확인하세요.
Check SD card SPI configuration.

---

## ⚠️ Boot Loop

SPI 주파수를 낮춰보세요.
Try lowering SPI frequency.

---

## ⚠️ USB Problems

일부 USB 장치는 추가 전력이 필요할 수 있습니다.
Some USB devices may require additional power.

---

# 👨‍💻 제작자 / Author

windows10101713

---

# 📜 라이선스 / License

이 프로젝트는 학습, 실험 및 개발 목적으로 제작되었습니다.
This project is intended for learning, experimentation, and development purposes.

---

# ⭐ 프로젝트 상태 / Project Status

> Experimental / Alpha Build
> 실험적 알파 빌드

> “ESP32-S3를 운영체제처럼 만들기 위한 실험.”
> “An experiment to turn the ESP32-S3 into a computer-like operating system.”
