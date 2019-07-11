#include "apu.h"
#include "gb.h"

APU::APU(MMU& mmu) : mmu_(mmu) {
  // Initialize all the sound registers to their boot up values
  mmu_.SetRegister(NR10, 0x80);
  mmu_.SetRegister(NR11, 0xBF);
  mmu_.SetRegister(NR12, 0xF3);
  mmu_.SetRegister(NR14, 0xBF);
  mmu_.SetRegister(NR21, 0x3F);
  mmu_.SetRegister(NR22, 0x00);
  mmu_.SetRegister(NR24, 0xBF);
  mmu_.SetRegister(NR30, 0x7F);
  mmu_.SetRegister(NR31, 0xFF);
  mmu_.SetRegister(NR32, 0x9F);
  mmu_.SetRegister(NR33, 0xBF);
  mmu_.SetRegister(NR41, 0xFF);
  mmu_.SetRegister(NR42, 0x00);
  mmu_.SetRegister(NR43, 0x00);
  mmu_.SetRegister(NR44, 0xBF);
  mmu_.SetRegister(NR50, 0x77);
  mmu_.SetRegister(NR51, 0xF3);
  // DMG / GBP / CGB
  mmu_.SetRegister(NR52, 0xF1);
}
