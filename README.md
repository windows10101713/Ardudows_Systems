# Ardudows_Systems
Ardudows Systems For ESP32 S3

You needs
1. ESP32 S3 N16R8
2. SPI TFT
3. PS/2 keyboard

This need libraries
#include <string.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <FS.h>
#include <SD.h>
#include <esp_system.h>
#include <WiFi.h>
#include "Fonts/ASCFont.h"
#include "Fonts/KSFont.h"
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
//#include <RTClib.h>
#include <ESPAsyncWebServer.h>
#include <freertos/semphr.h>
#include <AsyncTCP.h>
#include "usb/usb_host.h" 
#include "esp_intr_alloc.h"
#include "esp_log.h"
