# [Ardudows Systems GitHub Repository](https://github.com/windows10101713/Ardudows_Systems?utm_source=chatgpt.com)

## Ardudows Systems

Ardudows Systems is a lightweight operating system project for the ESP32-S3.
It is designed to provide a desktop-like environment on embedded hardware using a TFT display and keyboard input.

The project includes:

* File manager
* Wi-Fi tools
* Web features
* OTA update system
* USB support
* BLE functions
* Image decoder
* Router utilities
* Async web server
* Custom applications and tools

Made for experimentation, learning, and fun on ESP32 hardware.

---

## Hardware Requirements

You need:

1. ESP32-S3 N16R8
2. SPI TFT display
3. PS/2 keyboard

---

## Required Libraries

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

## Features

* Desktop-style UI
* SD card file system
* Wi-Fi connectivity
* Web server support
* OTA firmware updates
* BLE scanning and client support
* USB host support
* JPEG image decoding
* Async networking
* Multi-tool environment
* Custom Ardudows utilities

---

## Status

Ardudows Systems is currently in active development.
New features, applications, and system improvements are continuously being added.

---

## Goal

The goal of Ardudows Systems is to push the ESP32-S3 beyond a normal microcontroller project and create a mini computer-like experience on embedded hardware.

---

## Author

Created by windows10101713.
