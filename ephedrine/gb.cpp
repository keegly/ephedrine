#include <fstream>

#include <cereal/archives/binary.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include "spdlog/spdlog.h"

#include "apu.h"
#include "cpu.h"
#include "gb.h"
#include "mmu.h"
#include "ppu.h"

uint16_t Gameboy::divider_ = 0;
std::array<uint8_t, 2> Gameboy::joypad{{0xf, 0xf}};

Gameboy::Gameboy() : cpu(mmu), ppu(mmu), apu(mmu) {
  // no game
}

Gameboy::Gameboy(std::vector<uint8_t> &cart, std::string game)
    : mmu(cart), cpu(mmu), ppu(mmu), apu(mmu), game_(std::move(game)) {
  std::ifstream ifs{game_ + ".sav", std::ios::binary};
  spdlog::get("stdout")->info("Loading {0}", game_);
  if (ifs) {
    mmu.LoadBufferedRAM(ifs);
  }
}

Gameboy::~Gameboy() {
  // save the "battery buffered" external ram to disk
  if (mmu.cart_ram_modified) {
    std::ofstream ofs{game_ + ".sav", std::ios::binary};
    if (ofs) {
      mmu.SaveBufferedRAM(ofs);
    }
  }
}

// Simulate a toggling of the power switch
void Gameboy::Reset() {}

void Gameboy::Load(std::vector<uint8_t> cartridge) {
  mmu.Load(cartridge);
  mmu.boot_rom_enabled = false;
}

void Gameboy::SaveState() {
  std::ofstream ofs{game_ + ".st8", std::ios::binary};
  if (!ofs) {
    spdlog::get("stdout")->error("Error saving state");
    return;
  }
  cereal::BinaryOutputArchive oarchive(ofs);
  oarchive(mmu, cpu, ppu, joypad, current_screen_cycles_, game_, divider_,
           timer_ticks_);
}

void Gameboy::LoadState() {
  std::ifstream ifs{game_ + ".st8", std::ios::binary};
  if (!ifs) {
    spdlog::get("stdout")->error("Error loading state");
    return;
  }
  cereal::BinaryInputArchive iarchive(ifs);
  iarchive(mmu, cpu, ppu, joypad, current_screen_cycles_, game_, divider_,
           timer_ticks_);
}

void Gameboy::TimerTick(int cycles) {
  // divider is always counting regardless
  // increments every 256 cpu cycles (4.1 mhz) so 64 machine cycles?
  cycles *= 4;  // convert our machine cycle to cpu cycle
  divider_tick_cycles_ += cycles;
  if (divider_tick_cycles_ >= 256) {
    ++divider_;
    divider_tick_cycles_ = 0;
  }
  mmu.SetRegister(DIV, divider_ >> 8);
  const uint8_t timer_ctrl = mmu.ReadByte(TAC);

  if (!bit_check(timer_ctrl, 2)) return;

  // timer enabled
  const uint8_t timer_modulo = mmu.ReadByte(TMA);
  uint8_t timer_counter = mmu.ReadByte(TIMA);
  if (timer_ticks_ <= clocks_[timer_ctrl & 0x03]) {
    timer_ticks_ += cycles;
  } else {
    if (timer_counter == 0xFF) {
      timer_counter = timer_modulo;
      // request timer interrupt (bit 2)
      uint8_t int_req = mmu.ReadByte(IF);
      bit_set(int_req, 2);
      mmu.WriteByte(IF, int_req);
    } else {
      ++timer_counter;
    }
    // timer_counter == 0xFF ? timer_counter = timer_modulo : ++timer_counter;
    // spdlog::get("stdout")->debug("timer counter: {0}, modulo: {1},
    // clocks_:{2}", timer_counter, timer_modulo, clocks_[timer_ctrl & 0x03]);
    mmu.WriteByte(TIMA, timer_counter);
    timer_ticks_ = 0;
  }
}

void Gameboy::HandleInput(const std::array<uint8_t, 2> jp) {
  // std::lock_guard<std::mutex> lg(mutex);
  // update our internal joypad
  joypad = jp;
  // only request joypad int if there's a button pressed
  if (joypad[0] < 0x0F || joypad[1] < 0x0F) {
    uint8_t int_flag = mmu.ReadByte(IF);
    // bit 4 is joypad interrupt request - remove magic number usage here
    // TODO
    bit_set(int_flag, 4);
    mmu.WriteByte(IF, int_flag);
  }
}

int Gameboy::Tick(int ticks) {
  // A vertical refresh happens every 70224 clocks(140448 in GBC double speed
  // mode) : 59, 7275 Hz
  int current_screen_cycles = 0;
  while (!ppu.finished_current_screen) {
    auto prev_inst = cpu.Execute();
    // handle interrupts
    // if we're on the HALT opcode, need to handle interrupts
    // a little differently
    if (!cpu.IsHalted()) {
      cpu.HandleInterrupts();
    }
    TimerTick(cpu.cycles);
    ppu.Update(cpu.cycles);
    current_screen_cycles += cpu.cycles;
    if (--ticks <= 0) {
      break;
    }
  }
  ppu.finished_current_screen = false;
  return current_screen_cycles;
}
