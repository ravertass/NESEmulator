#pragma once
#include "CpuBoolean.h"
#include "Util.h"
class CpuArithmetic : public CpuBoolean
{
 private:
  typedef CpuBoolean super;
  void addToA(int value);
  void subFromA(int value);
  void cmpMReg(int memory, int reg);
  void decMemory(int address);
  void incMemory(int address);
  void DCP(int address);
  void ISB(int address);
  void RRA(int address);
 public:
  CpuArithmetic(void);
  ~CpuArithmetic(void);
  bool RunInstruction();
};

