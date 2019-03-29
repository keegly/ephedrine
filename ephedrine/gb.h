#ifndef GB_H
#define GB_H

#include "cpu.h"
#include "mmu.h"
#include "ppu.h"

// Interrupt Flag
constexpr uint16_t IF = 0xFF0F;
// Interrupt Enable
constexpr uint16_t IE = 0xFFFF;
// Joypad Registers
constexpr uint16_t P1 = 0xFF00;
// Timer Registers
constexpr uint16_t DIV = 0xFF04;
constexpr uint16_t TIMA = 0xFF05;
constexpr uint16_t TMA = 0xFF06;
constexpr uint16_t TAC = 0xFF07;
// PPU Registers
constexpr uint16_t LCDC = 0xFF40;
constexpr uint16_t STAT = 0xFF41;
constexpr uint16_t SCY = 0xFF42;
constexpr uint16_t SCX = 0xFF43;
constexpr uint16_t LY = 0xFF44;
constexpr uint16_t LYC = 0xFF45;
constexpr uint16_t DMA = 0xFF46;
constexpr uint16_t BGP = 0xFF47;
constexpr uint16_t OBP0 = 0xFF48;
constexpr uint16_t OBP1 = 0xFF49;
constexpr uint16_t WY = 0xFF4A;
constexpr uint16_t WX = 0xFF4B;

constexpr uint8_t INPUT_START = 0x08;
constexpr uint8_t INPUT_SELECT = 0x04;
constexpr uint8_t INPUT_B = 0x02;
constexpr uint8_t INPUT_A = 0x01;
constexpr uint8_t INPUT_DOWN = 0x08;
constexpr uint8_t INPUT_UP = 0x04;
constexpr uint8_t INPUT_LEFT = 0x02;
constexpr uint8_t INPUT_RIGHT = 0x01;

class Gameboy {
 public:
  Gameboy();
  Gameboy(std::vector<uint8_t> &cart, const std::string &game);
  void Reset();
  void HandleInput(std::array<uint8_t, 2> jp);
  int Tick(int ticks);
  void TickUntil(uint16_t pc);
  void Load(std::vector<uint8_t> cart);
  static void SetTimer(const uint16_t t) { divider_ = t; }
  static uint16_t GetTimer() { return divider_; }
  void TimerTick(int cycles);
  MMU mmu;
  CPU cpu;
  PPU ppu;
  const int max_cycles_per_vertical_refresh = 70224;
  static std::array<uint8_t, 2> joypad;
  void SaveState();
  void LoadState();
  ~Gameboy();

 private:
  // a vert refresh after this many cycles
  int current_screen_cycles_{};
  std::string game_{};
  static uint16_t divider_;
  int timer_ticks_{};
  const int clocks_[4] = {1024, 16, 64, 256};
};

#endif  // !GB_H
