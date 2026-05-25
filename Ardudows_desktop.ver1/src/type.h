#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>

#define WORLD_HEIGHT 64
#define CHUNK_SIZE 16

// 설계도를 여기에 완전히 격리시킵니다.
struct Chunk {
  int x;
  int z;
  uint8_t blocks[CHUNK_SIZE][CHUNK_SIZE][WORLD_HEIGHT]; 
};

#endif
