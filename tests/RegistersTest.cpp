#include "CppUnitLite/TestHarness.h"
#include "CpuRegisters.h"
#include "Util.h"

// LDA Instructions

TEST(CpuRegisters,LDA_Imm)
{
  CpuRegisters* cpu = new CpuRegisters();
  cpu->getMemory()->writeByteTo(0,LDA_Imm);
  cpu->getMemory()->writeByteTo(1,-15);
  cpu->RunInstruction();
  CHECK(cpu->getA() == -15);
  CHECK(cpu->getN());
  CHECK(!cpu->getZ());
}

TEST(CpuRegisters,LDA_Zp)
{
  CpuRegisters* cpu = new CpuRegisters();
  cpu->getMemory()->writeByteTo(0,LDA_Zp);
  cpu->getMemory()->writeByteTo(1,42);
  cpu->getMemory()->writeByteTo(42,17);
  cpu->RunInstruction();
  CHECK(cpu->getA() == 17);
}

TEST(CpuRegisters,LDA_Zpx)
{
  CpuRegisters* cpu = new CpuRegisters();
  cpu->getMemory()->writeByteTo(0,LDA_Zpx);
  cpu->getMemory()->writeByteTo(1,42);
  cpu->getMemory()->writeByteTo(43,17);
  cpu->setX(1);
  cpu->RunInstruction();
  CHECK(cpu->getA() == 17);
}

TEST(CpuRegisters,LDA_Abs)
{
  CpuRegisters* cpu = new CpuRegisters();
  cpu->getMemory()->writeByteTo(0,LDA_Abs);
  cpu->getMemory()->writeByteTo(1,0x39);
  cpu->getMemory()->writeByteTo(2,0x01);
  cpu->getMemory()->writeByteTo(0x0139,99);
  cpu->RunInstruction();
  CHECK(cpu->getA() == 99);
}

TEST(CpuRegisters,LDA_Absx)
{
  CpuRegisters* cpu = new CpuRegisters();
  MemoryState* mem = cpu->getMemory();
  mem->writeByteTo(0,LDA_Absx);
  mem->writeByteTo(1,0x39);
  mem->writeByteTo(2,0x01);
  mem->writeByteTo(0x013A,99);
  cpu->setX(1);
  cpu->RunInstruction();
  CHECK(cpu->getA() == 99);
}

TEST(CpuRegisters,LDA_Absy)
{
  CpuRegisters* cpu = new CpuRegisters();
  MemoryState* mem = cpu->getMemory();
  mem->writeByteTo(0,LDA_Absy);
  mem->writeByteTo(1,0x29);
  mem->writeByteTo(2,0x01);
  mem->writeByteTo(0x012B,99);
  cpu->setY(2);
  cpu->RunInstruction();
  CHECK(cpu->getA() == 99);
}

TEST(CpuRegisters,LDA_Indx)
{
  CpuRegisters* cpu = new CpuRegisters();
  MemoryState* mem = cpu->getMemory();
  mem->writeByteTo(0,LDA_Indx);
  mem->writeByteTo(1,0x3E);
  mem->writeByteTo(0x43,0x15);
  mem->writeByteTo(0x44,0x24);
  mem->writeByteTo(0x2415,74);
  cpu->setX(5);
  cpu->RunInstruction();
  CHECK(cpu->getA() == 74);
}

TEST (CpuRegisters,LDA_Indy)
{
  CpuRegisters* cpu = new CpuRegisters();
  MemoryState* mem = cpu->getMemory();
  mem->writeByteTo(0,LDA_Indy);
  mem->writeByteTo(1,0xF2);
  mem->writeByteTo(0xF2,0x11);
  mem->writeByteTo(0XF3,0x22);
  mem->writeByteTo(0x2222,23);
  cpu->setY(0x2222-0x2211);
  cpu->RunInstruction();
  CHECK(cpu->getA() == 23);
}

// LDX Instructions

TEST(CpuRegisters,LDX_Imm)
{
  CpuRegisters* cpu = new CpuRegisters();
  cpu->getMemory()->writeByteTo(0,LDX_Imm);
  cpu->getMemory()->writeByteTo(1,-15);
  cpu->RunInstruction();
  CHECK(cpu->getX() == -15);
  CHECK(cpu->getN());
  CHECK(!cpu->getZ());
}

TEST(CpuRegisters,LDX_Zp)
{
  CpuRegisters* cpu = new CpuRegisters();
  cpu->getMemory()->writeByteTo(0,LDX_Zp);
  cpu->getMemory()->writeByteTo(1,42);
  cpu->getMemory()->writeByteTo(42,17);
  cpu->RunInstruction();
  CHECK(cpu->getX() == 17);
}

TEST(CpuRegisters,LDX_Zpy)
{
  CpuRegisters* cpu = new CpuRegisters();
  cpu->getMemory()->writeByteTo(0,LDX_Zpy);
  cpu->getMemory()->writeByteTo(1,42);
  cpu->getMemory()->writeByteTo(43,17);
  cpu->setY(1);
  cpu->RunInstruction();
  CHECK(cpu->getX() == 17);
}

TEST(CpuRegisters,LDX_Abs)
{
  CpuRegisters* cpu = new CpuRegisters();
  cpu->getMemory()->writeByteTo(0,LDX_Abs);
  cpu->getMemory()->writeByteTo(1,0x39);
  cpu->getMemory()->writeByteTo(2,0x01);
  cpu->getMemory()->writeByteTo(0x0139,99);
  cpu->RunInstruction();
  CHECK(cpu->getX() == 99);
}

TEST(CpuRegisters,LDX_Absy)
{
  CpuRegisters* cpu = new CpuRegisters();
  MemoryState* mem = cpu->getMemory();
  mem->writeByteTo(0,LDX_Absy);
  mem->writeByteTo(1,0x29);
  mem->writeByteTo(2,0x01);
  mem->writeByteTo(0x012B,99);
  cpu->setY(2);
  cpu->RunInstruction();
  CHECK(cpu->getX() == 99);
}

// Transfer Instructions

TEST (CpuRegisters,TAX)
{
  CpuRegisters* cpu = new CpuRegisters();
  MemoryState* mem = cpu->getMemory();
  mem->writeByteTo(0,TAX);
  cpu->setA(45);
  cpu->RunInstruction();
  CHECK(cpu->getX() == 45);
}

TEST (CpuRegisters,TAY)
{
  CpuRegisters* cpu = new CpuRegisters();
  MemoryState* mem = cpu->getMemory();
  mem->writeByteTo(0,TAY);
  cpu->setA(45);
  cpu->RunInstruction();
  CHECK(cpu->getY() == 45);
}

TEST (CpuRegisters,TSX)
{
  CpuRegisters* cpu = new CpuRegisters();
  MemoryState* mem = cpu->getMemory();
  mem->writeByteTo(0,TSX);
  cpu->setS(45);
  cpu->RunInstruction();
  CHECK(cpu->getX() == 45);
}

TEST (CpuRegisters,TXA)
{
  CpuRegisters* cpu = new CpuRegisters();
  cpu->getMemory()->writeByteTo(0,TXA);
  cpu->setX(245);
  cpu->RunInstruction();
  CHECK(cpu->getA() == 245);
}

TEST (CpuRegisters,TXS)
{
  CpuRegisters* cpu = new CpuRegisters();
  cpu->getMemory()->writeByteTo(0,TXS);
  cpu->setX(245);
  cpu->RunInstruction();
  CHECK(cpu->getS() == 245);
}

TEST (CpuRegisters,TYA)
{
  CpuRegisters* cpu = new CpuRegisters();
  cpu->getMemory()->writeByteTo(0,TYA);
  cpu->setY(245);
  cpu->RunInstruction();
  CHECK(cpu->getA() == 245);
}



int main()
{
  TestResult tr;
  TestRegistry::runAllTests(tr);
  
  return 0;
}
