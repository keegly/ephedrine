#ifndef GB_H
#define GB_H

#include <cereal/archives/binary.hpp>
#include "apu.h"
#include "cpu.h"
#include "mmu.h"
#include "ppu.h"

/* Interrupt Flag */
inline constexpr uint16_t IF = 0xFF0F;
/* Interrupt Enable */
inline constexpr uint16_t IE = 0xFFFF;
/* Joypad Registers */
inline constexpr uint16_t P1 = 0xFF00;
/* Timer Registers */
inline constexpr uint16_t DIV = 0xFF04;
inline constexpr uint16_t TIMA = 0xFF05;
inline constexpr uint16_t TMA = 0xFF06;
inline constexpr uint16_t TAC = 0xFF07;
/* PPU Registers */
inline constexpr uint16_t LCDC = 0xFF40;
inline constexpr uint16_t STAT = 0xFF41;
inline constexpr uint16_t SCY = 0xFF42;
inline constexpr uint16_t SCX = 0xFF43;
inline constexpr uint16_t LY = 0xFF44;
inline constexpr uint16_t LYC = 0xFF45;
inline constexpr uint16_t DMA = 0xFF46;
inline constexpr uint16_t BGP = 0xFF47;
inline constexpr uint16_t OBP0 = 0xFF48;
inline constexpr uint16_t OBP1 = 0xFF49;
inline constexpr uint16_t WY = 0xFF4A;
inline constexpr uint16_t WX = 0xFF4B;
/* Joy Pad */
inline constexpr uint8_t INPUT_START = 0x08;
inline constexpr uint8_t INPUT_SELECT = 0x04;
inline constexpr uint8_t INPUT_B = 0x02;
inline constexpr uint8_t INPUT_A = 0x01;
inline constexpr uint8_t INPUT_DOWN = 0x08;
inline constexpr uint8_t INPUT_UP = 0x04;
inline constexpr uint8_t INPUT_LEFT = 0x02;
inline constexpr uint8_t INPUT_RIGHT = 0x01;
/* Audio Registers */
inline constexpr uint16_t NR10 = 0xFF10;  // Channel 1 Sweep Register
inline constexpr uint16_t NR11 = 0xFF11;  // Ch 1 Sound Length/Wave Pattern Duty
inline constexpr uint16_t NR12 = 0xFF12;  // Channel 1 Volume Envelope
inline constexpr uint16_t NR13 = 0xFF13;  // Channel 1 Frequency Low Bits
inline constexpr uint16_t NR14 = 0xFF14;  // Channel 1 Frequency High Bits

inline constexpr uint16_t NR21 = 0xFF16;  // Ch 2 Sound Length/Wave Pattern Duty
inline constexpr uint16_t NR22 = 0xFF17;  // Channel 2 Volume Envelope
inline constexpr uint16_t NR23 = 0xFF18;  // Channel 2 Frequency Low Bits
inline constexpr uint16_t NR24 = 0xFF19;  // Channel 2 Frequency High Bits

inline constexpr uint16_t NR30 = 0xFF1A;  // Channel 3 Sound On/Off
inline constexpr uint16_t NR31 = 0xFF1B;  // Channel 3 Sound Length
inline constexpr uint16_t NR32 = 0xFF1C;  // Channel 3 Select Output Level
inline constexpr uint16_t NR33 = 0xFF1D;  // Channel 3 Frequency Low Bits
inline constexpr uint16_t NR34 = 0xFF1E;  // Channel 3 Frequency High Bits

inline constexpr uint16_t NR41 = 0xFF20;  // Channel 4 Sound Length
inline constexpr uint16_t NR42 = 0xFF21;  // Channel 4 Volume Envelope
inline constexpr uint16_t NR43 = 0xFF22;  // Channel 4 Polynomial Counter
inline constexpr uint16_t NR44 = 0xFF23;  // Channel 4 Counter/Consecutive
inline constexpr uint16_t NR50 = 0xFF24;  // Channel Control / On-Off / Volume
inline constexpr uint16_t NR51 = 0xFF25;  // Sound Output Terminal Select
inline constexpr uint16_t NR52 = 0xFF26;  // Sound on/off

class Gameboy {
 public:
  Gameboy();
  Gameboy(std::vector<uint8_t> &cart, std::string game);
  Gameboy(const Gameboy &) = delete;             // copy ctor
  Gameboy(Gameboy &&) = delete;                  // move ctor
  Gameboy &operator=(Gameboy const &) = delete;  // copy assignment
  Gameboy &operator=(Gameboy &&) = delete;       // move assignment
  ~Gameboy() noexcept;
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
  APU apu;
  const int max_cycles_per_vertical_refresh = 70224;
  static std::array<uint8_t, 2> joypad;
  void SaveState();
  void LoadState();
  template <class Archive>
  void serialize(Archive &archive) {
    archive(mmu, cpu, ppu, joypad, current_screen_cycles_, game_, divider_,
            timer_ticks_);
  }

 private:
  // a vert refresh after this many cycles
  int current_screen_cycles_ = 0;
  std::string game_{};
  // Timer/Divider
  static uint16_t divider_;
  int timer_ticks_ = 0;
  int divider_tick_cycles_ = 0;
  const int clocks_[4] = {1024, 16, 64, 256};
};

#endif  // !GB_H
