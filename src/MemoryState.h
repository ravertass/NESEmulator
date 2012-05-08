#pragma once

#include "Util.h"
#include "Mapper.h"
#define RAM_SIZE 0x2000

class MemoryState
{
private:
  Mapper* mapper;
  char RAM[RAM_SIZE];
  char palette[0x20];
  char nametable1[0x400];
  char nametable2[0x400];
  int mirroring;

  // PPU Registers
  char PPUCTRL;
  char PPUMASK;
  char PPUSTATUS;
  char OAMADDR;
  char OAMDATA;
  char PPUSCROLL;
  char PPUADDR;
  char PPUDATA;
public:
  MemoryState(void);
  ~MemoryState(void);
  
  int readByteFrom(int address);
  void writeByteTo(int address, int value);
  
  void loadFileToRAM(char* filename);

  int ppuReadByteFrom(int address);
  void ppuWriteByteTo(int address, int value);

  int readFromNametable(int nametable, int address);
  void writeToNametable(int nametable, int address, int value);
};

