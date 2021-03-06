#include "MemoryState.h"
#include "Util.h"
#include "CpuV2.h"
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>

#include "Mapper0.h"
#include "Mapper1.h"
#include "Mapper2.h"
#include "Mapper4.h"
#include "Mapper7.h"
using namespace std;

MemoryState::MemoryState(void)
{
  ppuLatch = false;
  for (int i = 0; i < RAM_SIZE; i++)
    RAM[i] = 0;
  for (int i = 0; i < 0x20; i++)
    palette[i] = 0;
  for (int i = 0; i < 0x400; i++)
    nametable1[i] = nametable2[i] = 0;
  apu = NULL;
  PPUSTATUS = 0x0;
  PPU_LAST_WRITE = 0x0;
}

MemoryState::~MemoryState(void)
{
}

void MemoryState::setGamepad(GamepadState* gpad)
{
  gamepad = gpad;
}

void MemoryState::setApu(ApuState* _apu)
{
  apu = _apu;
}

void MemoryState::setCpu(CpuV2* _cpu)
{
  cpu = _cpu;
}

////////////////////////////////////////////////////////////////////////
// Read/Write Operations
////////////////////////////////////////////////////////////////////////

int MemoryState::readByteFrom(int address)
{
  if (address < 0x2000)
    {
      return (RAM[address & 0x7FF]) & 0xFF;
    }
  else if (address < 0x5000)
    {
      int retVal = 0;
      switch (address)
	{
	case 0x2002:
	  {
	    ppuLatch = false;
	    retVal = PPUSTATUS & 0xFF;
	    PPUSTATUS &= 0x7F;         // Reading 0x2002 resets the NMI flag
	    retVal |= (PPU_LAST_WRITE & 0x1F);
	    return retVal; 
	  }
	case 0x2004:
	  return OAM[OAMADDR] & 0xFF;
	case 0x2007:
	  if (PPUADDR >= 0x3F00)
	    retVal = ppuReadByteFrom(PPUADDR) & 0xFF;
	  else
	    {
	      retVal = ppuDataBuffer;
	      ppuDataBuffer = ppuReadByteFrom(PPUADDR) & 0xFF;
	    }
	  PPUADDR++;
	  return retVal;
	case 0x4016:
	  return (gamepad->readPlayer1() & 0xFF) | 0x40; // the 0x40 is just because, I guess?
	case 0x4017:
	  return (gamepad->readPlayer2() & 0xFF) | 0x40; // Apparently, that is how it works, and some games expect it
	default:
	  return -1;
	}
    }
  else
    {
      return (mapper->readByteFrom(address)) & 0xFF;
    }
}

void MemoryState::writeByteTo(int address, int value)
{
#ifdef PPU_WRITE_LOG
  if (address >= 0x2000 && address < 0x2007 && address != 0x2004)
    {
      int oldValue;
      switch (address)
	{
	case 0x2000:
	  oldValue = PPUCTRL;
	  break;
	case 0x2001:
	  oldValue = PPUMASK;
	  break;
	case 0x2002:
	  oldValue = PPUSTATUS;
	  break;
	case 0x2003:
	  oldValue = OAMADDR;
	  break;
	case 0x2005:
	  oldValue = (isPpuScrollOnX) ? PPUSCROLLY : PPUSCROLLX;
	  break;
	case 0x2006:
	  oldValue = PPUADDR;
	  break;
	case 0x2007:
	  oldValue = 0x2323232323232323;
	  break;
	}
      cout << "0x" << hex << address << " = 0x" << setw(2) << value << "   from 0x" << setw(2) << oldValue << "\n";
    }
#endif
  if (address < 0x2000)
    RAM[address] = (value & 0xFF); // Each byte only holds 8 bits of data
  if (address >= 0x4000 && address <= 0x4017 && address != 0x4014 && address != 0x4016)
    {
      if (apu != NULL)
	apu->write_register(address, value);
    }
  else if (address < 0x5000)
    {
      switch (address)
	{
	case 0x2000:
	  PPUCTRL = (value & 0xFF);
	  loopyT &= ~0xC00;
	  loopyT |= (value & 0x03) << 10;
	  break;
	case 0x2001:
	  PPUMASK = (value & 0xFF);
	  break;
	case 0x2002:
	  PPUSTATUS = (value & 0xFF);
	  break;
	case 0x2003:
	  OAMADDR = (value & 0xFF);
	  break;
	case 0x2004:
	  oamWriteByteTo(OAMADDR, value & 0xFF);
	  OAMADDR++;
	  break;
	case 0x2005:
	  if (!ppuLatch)
	    {
	      loopyT &= ~(0x001F);
	      loopyT |= (value & 0xF8) >> 3;
	      loopyX = value & 0x07;
	    }
	  else
	    {
	      loopyT &= ~(0x73E0);
	      loopyT |= (value & 0xF8) << 2;
	      loopyT |= (value & 0x07) << 12;
	    }
	  ppuLatch = !ppuLatch;
	  break;
	case 0x2006:
	  if (!ppuLatch)
	    {
	      loopyT &= ~(0xFF00);
	      loopyT |= (0x3F & value) << 8;
	    }
	  else
	    {
	      loopyT &= ~(0x00FF);
	      loopyT |= (0xFF & value);
	      PPUADDR = loopyT;
	    }
	  mapper->updatePpuAddr(PPUADDR);
	  ppuLatch = !ppuLatch;
	  break;
	case 0x2007:
	  ppuWriteByteTo(PPUADDR,value);
	  if (!(PPUCTRL & 0x04))
	    PPUADDR++;
	  else
	    PPUADDR += 32;
	  mapper->updatePpuAddr(PPUADDR);
	  break;
	case 0x4014:                   // DMA
	  DMA(value);
	case 0x4016:
	  if (JOYSTROBE == 1 && value == 0)
	    gamepad->strobe();
	  JOYSTROBE=value;
	  break;
	case 0x6004:
	  cout << value;
	default:
	  return; // Unimplemented behavior
	}
    }
  else
    {
      mapper->writeByteTo(address, value);
    }
  if (address >= 0x2000 && address <= 0x2007)
    PPU_LAST_WRITE = value;
}

int MemoryState::ppuReadByteFrom(int address)
{
  if (address < 0x2000)
    return (mapper->ppuReadByteFrom(address)) & 0xFF;
  else if (address < 0x2400)
    return readFromNametable(0,address) & 0xFF;
  else if (address < 0x2800)
    return readFromNametable(1,address) & 0xFF;
  else if (address < 0x2C00)
    return readFromNametable(2,address) & 0xFF;
  else if (address < 0x3000)
    return readFromNametable(3,address) & 0xFF;
  else if (address < 0x3F00)
    return ppuReadByteFrom(address - 0x1000) & 0xFF;
  else
    return palette[(address-0x3F20) % 0x20] & 0xFF; // Rest of RAM is just palette mirrored
}

void MemoryState::ppuWriteByteTo(int address, int value)
{
  if (address < 0x2000)
    mapper->ppuWriteByteTo(address, value & 0xFF);
  else if (address < 0x2400)
    writeToNametable(0,address,value & 0xFF);
  else if (address < 0x2800)
    writeToNametable(1,address,value & 0xFF);
  else if (address < 0x2C00)
    writeToNametable(2,address,value & 0xFF);
  else if (address < 0x3000)
    writeToNametable(3,address,value & 0xFF);
  else if (address < 0x3F00)
    ppuWriteByteTo(address - 0x1000, value & 0xFF);
  else if ((address-0x3F00) % 0x10 == 0)
    palette[0] = value&0xFF;
  else
    palette[(address-0x3F00) % 0x20] = value & 0xFF; // Rest of RAM is just palette mirrored
}

int MemoryState::apuReadByteFrom(void*, unsigned address)
{
  return readByteFrom(address);
}

unsigned char* MemoryState::mirroredNametableAtXY(int x, int y)
{
  if (mapper->mirroring == MIRRORING_HORIZONTAL)
    {
      if ((y%2) == 0)
	return nametable1;
      else
	return nametable2;
    }
  else if (mapper->mirroring == MIRRORING_VERTICAL)
    {
      if ((x%2) == 0)
	return nametable1;
      else
	return nametable2;
    }
  else if (mapper->mirroring == MIRRORING_LOWER_BANK)
    return nametable1;
  else if (mapper->mirroring == MIRRORING_UPPER_BANK)
    return nametable2;
  else
    {
      cout << "Unknown mirroring configuration\n";
      return nametable1;
    }
}

int MemoryState::readFromNametable(int nametable, int address)
{
  /*int currentNametable = 1;
  if (mapper->mirroring == 0) // Horizontal Mirroring
    {
      if (nametable > 1)
	currentNametable = 2;
    }
  else if (mapper->mirroring == 1) // Vertical Mirroring
    {
      if (nametable == 1 || nametable == 3)
	currentNametable = 2;
    }
  else
    {
    } // TODO: Single-Screen Mirroring - needs mapper support
  int arrayAddress = (address - 0x2000) % 0x400;
  if (currentNametable == 1)
    return nametable1[arrayAddress] & 0xFF;
  else
  return nametable2[arrayAddress] & 0xFF;*/
  unsigned char* nt = mirroredNametableAtXY(nametable&0x1, (nametable&0x2)>>1);
  return nt[address % 0x400];
}

void MemoryState::DMA(int address)
{
  for (int i = 0; i <= 0xFF; i++)
    {
      int currentAddress = (address << 8) + i;
      writeByteTo(0x2004,readByteFrom(currentAddress));
    }
  cpu->cycles += 512; //incrementCycles(512);
  return;
}

void MemoryState::writeToNametable(int nametable, int address, int value)
{
  int currentNametable = 1;
  nametable %= 4;
  if (mapper->mirroring == 0) // Horizontal Mirroring
    {
      if (nametable > 1)
	currentNametable = 2;
    }
  else if (mapper->mirroring == 1) // Vertical Mirroring
    {
      if (nametable == 1 || nametable == 3)
	currentNametable = 2;
    }
  else
    {
    } // TODO: Single-Screen Mirroring - needs mapper support
  int arrayAddress = address & 0x3FF;
  if (currentNametable == 1)
    nametable1[arrayAddress] = value;
  else
    nametable2[arrayAddress] = value;
}

int MemoryState::getNametableEntryForTile(int x, int y, int xScroll, int yScroll)
{
  if (PPUCTRL & 0x01)
    x += 32;
  if (PPUCTRL & 0x02)
    y += 30;
  x += (xScroll/8);
  y += (yScroll/8);
  unsigned char* currentNametable = mirroredNametableAtXY((x/32),(y/30));
  int address = (y%30)*32;         // Account for both y wrapping around and each line being 32 tiles wide
  address += (x%32);               // Account for x coordinate
  return currentNametable[address];
}

int MemoryState::attributeEntryForXY(int x, int y, int xScroll, int yScroll)
{
  if (PPUCTRL & 0x01)
    x += 32;
  if (PPUCTRL & 0x02)
    y += 30;
  x += (xScroll/8);
  y += (yScroll/8);
  unsigned char* currentNametable = mirroredNametableAtXY((x/32),(y/30));
  int address = 0x3C0;

  // I don't remember why this works. Voodoo for now.
  int first = ((y%30)>>2)*8;
  address += first + ((x%32) >> 2);
  return currentNametable[address];
}

int MemoryState::oamReadByteFrom(int address)
{
  return OAM[address] & 0xFF;
}

void MemoryState::oamWriteByteTo(int address, int value)
{
  OAM[address] = (value & 0xFF);
}

unsigned char MemoryState::colorForPaletteIndex(bool isSprite, int paletteIndex, int index)
{
  unsigned char retVal = 0;
  if (index == 0)
    retVal = palette[0];
  else
    {
      int baseAddress = 0;
      if (isSprite)
	baseAddress += 0x10;
      baseAddress += paletteIndex*4;
      retVal = palette[baseAddress+index];
    }
  if (PPUMASK & 0x01)
    retVal &= 0x30;
  return retVal;
}

void MemoryState::scanlineCounter()
{
  mapper->scanlineCounter();
}

////////////////////////////////////////////////////////////////////////
// File IO
////////////////////////////////////////////////////////////////////////

void MemoryState::loadFileToRAM(char* filename)
{
  FILE* fileStream;
  size_t size;
  size_t result;
  char* file;

  cout << "Loading ROM... ";

  fileStream = fopen(filename, "rb");
  if (fileStream == NULL)
    {
      cout << "Could not read file. ROM not loaded.\n";
      return;
    }

  // obtain file size
  fseek(fileStream, 0, SEEK_END);
  size = ftell(fileStream);
  rewind(fileStream);

  file = (char*)malloc(sizeof(char)*size);
  if (file == NULL)
    {
      cout << "Memory error. ROM not loaded.\n";
      fclose(fileStream);
      return;
    }

  result = fread (file,1,size,fileStream);
  if (result != size)
    {
      cout << "Reading error. ROM not loaded.\n";
      fclose(fileStream);
      return;
    }
  fclose(fileStream);

  bool isROM = (file[0] == 'N' && file[1] == 'E' && file[2] == 'S' && file[3] == 0x1A);
  if (!isROM)
    {
      cout << "This is not a NES rom.";
      return;
    }

  int mapperNumber = ((file[6] & 0xF0) >> 4) + (file[7] & 0xF0);
  cout << "With mapper #" << mapperNumber << " ";

  switch (mapperNumber)
    {
    case 0:
      mapper = new Mapper0(file);
      break;
    case 1:
      mapper = new Mapper1(file);
      break;
    case 2:
      mapper = new Mapper2(file);
      break;
    case 4:
      mapper = new Mapper4(file);
      ((Mapper4*)mapper)->cpu = cpu;
      break;
    case 7:
      mapper = new Mapper7(file);
      break;
    default:
      cout << "Error! Unknown mapper number " << mapperNumber << ".\n";
      break;
    }

  cout << "Done.\n";
  
  return;
}

void MemoryState::saveState(ofstream& file)
{
  WRITEC(RAM,RAM_SIZE);
  WRITEC(palette,0x20);
  WRITEC(nametable1,0x400);
  WRITEC(nametable2,0x400);
  WRITE(&ppuDataBuffer,int,1);
  WRITEC(OAM,256);
  WRITEC(&JOYSTROBE,1);
  WRITE(&ppuLatch,bool,1);
  WRITEC(&PPUCTRL,1);
  WRITEC(&PPUMASK,1);
  WRITEC(&PPUSTATUS,1);
  WRITEC(&OAMADDR,1);
  WRITEC(&PPU_LAST_WRITE,1);
  WRITE(&PPUADDR,int,1);
  mapper->saveState(file);
}

void MemoryState::loadState(ifstream& file)
{
  READC(RAM,RAM_SIZE);
  READC(palette,0x20);
  READC(nametable1,0x400);
  READC(nametable2,0x400);
  READ(&ppuDataBuffer,int,1);
  READC(OAM,256);
  READC(&JOYSTROBE,1);
  READ(&ppuLatch,bool,1);
  READC(&PPUCTRL,1);
  READC(&PPUMASK,1);
  READC(&PPUSTATUS,1);
  READC(&OAMADDR,1);
  READC(&PPU_LAST_WRITE,1);
  READ(&PPUADDR,int,1);
  mapper->loadState(file);
}

void MemoryState::saveBattery(char* filename)
{
  mapper->saveBattery(filename);
}

void MemoryState::loadBattery(char* filename)
{
  mapper->loadBattery(filename);
}
