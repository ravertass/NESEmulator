#pragma once
#include "CpuRegisters.h"
#include "Util.h"

class CpuBoolean : public CpuRegisters
{
 private:
  typedef CpuRegisters super;
 public:
  CpuBoolean(void);
  ~CpuBoolean(void);
  bool RunInstruction();
};