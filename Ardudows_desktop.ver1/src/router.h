#ifndef ROUTER_H
#define ROUTER_H

#include <Arduino.h>

// 데이터 크기를 미리 알려주면 나중에 플래싱할 때 편해
extern const unsigned int router_len;
// 실제 데이터 배열 (메모리 절약을 위해 Flash 영역에 저장)
extern const unsigned char router[] PROGMEM;

#endif
