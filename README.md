# [Ardudows Systems GitHub Repository](https://github.com/windows10101713/Ardudows_Systems?utm_source=chatgpt.com)

# 🖥️ Ardudows Systems

### ESP32-S3 기반 임베디드 운영체제 프로젝트

### Embedded Operating System Project for ESP32-S3

Ardudows Systems는 ESP32-S3를 기반으로 제작된 경량 임베디드 운영체제 프로젝트입니다.
Ardudows Systems is a lightweight embedded operating system project built for the ESP32-S3 platform.

TFT 디스플레이, PS/2 키보드, SD 카드, Wi-Fi 및 USB 기능을 활용하여 마이크로컨트롤러 환경에서 데스크톱 스타일 인터페이스와 시스템 기능을 구현하는 것을 목표로 합니다.
The project aims to implement desktop-style interfaces and system-level functionality in a microcontroller environment using TFT displays, PS/2 keyboards, SD cards, Wi-Fi, and USB features.

Ardudows Systems는 단순한 Arduino 프로젝트가 아니라, ESP32-S3의 하드웨어 성능을 최대한 활용하여 소형 컴퓨터 수준의 환경을 구축하는 실험적인 시스템입니다.
Ardudows Systems is not just a simple Arduino project, but an experimental system designed to push the ESP32-S3 toward a mini computer-like environment.

---

# ✨ 주요 기능 / Core Features

## 🪟 그래픽 인터페이스 / Graphical Interface

* TFT_eSPI 기반 그래픽 렌더링 시스템
  TFT_eSPI-based graphical rendering system

* 창(Window) 스타일 사용자 인터페이스
  Window-style user interface

* 키보드 기반 입력 시스템
  Keyboard-driven input system

* JPEG 이미지 디코딩 지원
  JPEG image decoding support

---

## 📂 파일 시스템 / File System

* SD 카드 기반 파일 저장 시스템
  SD card-based file storage system

* FS(File System) API 지원
  FS (File System) API support

* 설정 저장 및 Preferences 관리
  Preferences-based configuration storage

* 운영체제 스타일 디렉터리 구조
  Operating system-style directory structure

---

## 🌐 네트워크 기능 / Networking Features

* Wi-Fi 연결 및 관리
  Wi-Fi connectivity and management

* HTTP / HTTPS 클라이언트 기능
  HTTP / HTTPS client functionality

* 내장 Web Server 지원
  Built-in Web Server support

* DNS Server 기능 지원
  DNS Server functionality

* Async TCP 및 비동기 네트워크 처리
  Async TCP and asynchronous networking

* Socket.IO 기반 실시간 통신 지원
  Socket.IO-based real-time communication

---

## ⚙️ 시스템 기능 / System Features

* OTA(Over-The-Air) 업데이트 지원
  OTA (Over-The-Air) update support

* ESP-IDF 시스템 API 활용
  ESP-IDF system API integration

* FreeRTOS 세마포어 및 멀티태스킹 지원
  FreeRTOS semaphore and multitasking support

* 로그 및 디버깅 시스템
  Logging and debugging system

* 메모리 및 시스템 상태 관리
  Memory and system state management

---

## 🔌 USB 및 BLE 기능 / USB and BLE Features

* USB Host 기능 지원
  USB Host functionality support

* TinyUSB(tusb) 기반 USB 처리
  TinyUSB (tusb)-based USB processing

* BLE 스캔 기능
  BLE scanning functionality

* BLE 클라이언트 연결 지원
  BLE client connection support

* BLE Advertised Device 분석 기능
  BLE advertised device analysis support

---

# 🧩 하드웨어 요구사항 / Hardware Requirements

다음 하드웨어가 필요합니다.
The following hardware is required.

1. ESP32-S3 N16R8
2. SPI TFT Display
3. PS/2 Keyboard
4. SD Card Module (Optional)
5. USB-compatible peripherals (Optional)

---

# 📚 사용 라이브러리 / Used Libraries

```cpp id="o4p1aw"
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

# 🎯 프로젝트 목표 / Project Goals

Ardudows Systems의 목표는 ESP32-S3를 단순한 IoT 칩이 아닌, 실제 운영체제 개념을 실험할 수 있는 플랫폼으로 확장하는 것입니다.
The goal of Ardudows Systems is to transform the ESP32-S3 from a simple IoT chip into a platform capable of experimenting with real operating system concepts.

이 프로젝트는 다음과 같은 영역을 탐구합니다.
This project explores areas such as:

* 🖥️ 임베디드 GUI 시스템
  Embedded GUI systems

* ⚡ 경량 운영체제 구조
  Lightweight operating system architecture

* 🌍 네트워크 기반 시스템 기능
  Network-based system functionality

* 🧠 저사양 하드웨어 최적화
  Low-resource hardware optimization

* 🔬 ESP32 기반 컴퓨터화 실험
  ESP32-based computerization experiments

---

# 🚧 개발 상태 / Development Status

현재 Ardudows Systems는 활발히 개발 중인 프로젝트입니다.
Ardudows Systems is currently under active development.

새로운 시스템 기능, 응용 프로그램, 드라이버 및 최적화 작업이 지속적으로 추가되고 있습니다.
New system features, applications, drivers, and optimizations are continuously being added.

---

# 👨‍💻 제작자 / Author

windows10101713

---

# 📜 라이선스 / License

이 프로젝트는 학습, 개발 및 하드웨어 실험 목적으로 제작된 실험적 프로젝트입니다.
This project is an experimental project intended for learning, development, and hardware experimentation purposes.
