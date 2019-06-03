#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

#include "bit_utility.h"
#include "gb.h"
#include "mmu.h"
#include "ppu.h"
#include "spdlog/spdlog.h"

MMU::MMU(std::vector<uint8_t> &cart, const bool boot_rom)
    : boot_rom_enabled(boot_rom), cartridge_(cart) {
  Load(cartridge_);
}

void MMU::Load(std::vector<uint8_t> &c) {
  if (c.empty()) return;

  rom_banks = (32 << cartridge_[0x0148]) / 16;
  spdlog::get("stdout")->debug("Rom Banks: {0}", rom_banks);
  // TODO: fix ram bank
  num_ram_banks = external_ram_size_[cartridge_[0x0149]];
  spdlog::get("stdout")->debug("Ram Banks: {0}", num_ram_banks);
  memory_bank_controller_ = static_cast<CartridgeType>(cartridge_[0x0147]);
  if (num_ram_banks > 1) {
    ram_banks_.resize(num_ram_banks);
  } else {
    ram_banks_.resize(1);
  }
  if (rom_banks <= 2) {
    std::copy(cartridge_.begin(), cartridge_.end(), memory_.begin());
  } else {
    // load the ROM and the first switchable bank, break the cart into it's
    // individual 8kB banks, and store them for easy usage later
    cart_rom_banks_.resize(rom_banks);
    std::copy_n(cartridge_.begin(), 0x8000, memory_.begin());
    for (int i = 0; i < rom_banks; ++i) {
      std::copy_n(c.begin() + (0x4000 * i), 0x4000, cart_rom_banks_[i].begin());
    }
  }
}

void MMU::ShowDebugWindow() {}

void MMU::SetPPUMode(const uint8_t mode) {
  // invalid mode
  if (mode > 0x03) return;

  bitmask_clear(memory_[STAT], 0x03);
  bitmask_set(memory_[STAT], mode);
}

void MMU::SaveBufferedRAM(std::ofstream &ofs) {
  // only save if we would have a battery onboard
  switch (memory_bank_controller_) {
    case CartridgeType::kMBC1wRAMwBattery:
    case CartridgeType::kMBC2wBattery:
    case CartridgeType::kROMRAMwBattery:
    case CartridgeType::kMMM01wRAMwBattery:
    case CartridgeType::kMBC3wTimerwBattery:
    case CartridgeType::kMBC3wTimerwRAMwBattery:
    case CartridgeType::kMBC3wRAMwBattery:
    case CartridgeType::kMBC5wRAMwBattery:
    case CartridgeType::kMBC5wRumblewRAMwBattery:
    case CartridgeType::kMBC7wSensorwRumblewRAMwBattery: {
      // TODO: save/load all RAM banks
      std::copy(memory_.begin() + 0xA000, memory_.begin() + 0xC000,
                ram_banks_[active_ram_bank_].begin());
      /*    auto out_it = std::ostream_iterator<uint8_t>(ofs);
          for (const auto &bank : ram_banks_) {
            std::copy(std::begin(bank), std::end(bank), out_it);
          }*/
      for (int i = 0; i < num_ram_banks; ++i) {
        for (int j = 0; j < 0x2000; ++j) {
          ofs << ram_banks_[i][j];
        }
      }
      break;
    }
    default:
      spdlog::get("stdout")->info(
          "Cartridge type {0} not battery buffered",
          static_cast<uint8_t>(memory_bank_controller_));
      break;
  }
}

void MMU::LoadBufferedRAM(std::ifstream &ifs) {
  // only save if we would have a battery onboard
  switch (memory_bank_controller_) {
    case CartridgeType::kMBC1wRAMwBattery:
    case CartridgeType::kMBC2wBattery:
    case CartridgeType::kROMRAMwBattery:
    case CartridgeType::kMMM01wRAMwBattery:
    case CartridgeType::kMBC3wTimerwBattery:
    case CartridgeType::kMBC3wTimerwRAMwBattery:
    case CartridgeType::kMBC3wRAMwBattery:
    case CartridgeType::kMBC5wRAMwBattery:
    case CartridgeType::kMBC5wRumblewRAMwBattery:
    case CartridgeType::kMBC7wSensorwRumblewRAMwBattery: {
      /* auto in_it = std::istream_iterator<uint8_t>(ifs);
       for (int i = 0; i < num_ram_banks; ++i) {
         std::copy(std::begin(ram_banks_[i]), ram_banks_[i].end(), in_it);
       }*/
      for (int i = 0; i < num_ram_banks; ++i) {
        for (int j = 0; j < 0x2000; ++j) {
          ifs >> ram_banks_[i][j];
        }
      }
      // load bank 0 into memory, so the emu can see it
      // since apparently the game won't necessarily bank switch on start up
      std::copy(ram_banks_[active_ram_bank_].begin(),
                ram_banks_[active_ram_bank_].end(), memory_.begin() + 0xA000);
      break;
    }
    default:
      spdlog::get("stdout")->info(
          "Cartridge type {0} not battery buffered",
          static_cast<uint8_t>(memory_bank_controller_));
      break;
  }
}

uint8_t MMU::ReadByte(const uint16_t address) {
  if (boot_rom_enabled && address <= 0xFF) return boot_rom_[address];
  if (cartridge_.empty()) return 0xFF;
  // PPU mode
  const uint8_t ppu_mode = memory_[STAT] & 0x03;
  // Read from a ROM bank
  // joypad bits 6 and 7 always return 1
  if (address == P1) {
    bitmask_set(memory_[address], 0xC0);  // should be C0
  }
  // bit 7 unused and always returns 1, bits 0-2 return 0 when LCD is off
  if (address == STAT) {
    bitmask_set(memory_[address], 0x80);
    if (!bit_check(memory_[LCDC], 7)) {
      bitmask_clear(memory_[address], 0x03);
    }
  }
  // upper 3 bits always return 1
  if (address == IF) {
    bitmask_set(memory_[address], 0xE0);
  }
  // Vram inaccessible during mode 3
  if ((address >= 0x8000 && address <= 0x9FFF) &&
      ppu_mode == kPPUModeLCDTransfer) {
    return 0xFF;
  }
  // RAM has to be enabled to read from it
  if (address >= 0xA000 && address <= 0xBFFF && !ram_enabled_) {
    return 0xFF;
  }
  // Only able to read from OAM in H-Blank and V-Blank
  if (address >= 0xFE00 && address <= 0xFE9F && ppu_mode >= kPPUModeOAMSearch) {
    return 0xFF;
  }

  return memory_[address];  // needs more logic regarding certain addresses
}

void MMU::WriteByte(const uint16_t address, uint8_t value) {
  if (address == 0xFF50 && value == 0x01) {
    boot_rom_enabled = false;
  }

  // RAM Enable
  if (address >= 0 && address <= 0x1FFF) {
    (value & 0x0F) == 0x0A ? ram_enabled_ = true : ram_enabled_ = false;
    // Cover all the MBC3 variants without explicitly listing them all
    if (memory_bank_controller_ >= CartridgeType::kMBC3 &&
        memory_bank_controller_ <= CartridgeType::kMBC3wRAMwBattery) {
      (value & 0x0F) == 0x0A ? rtc_enabled_ = true : rtc_enabled_ = false;
    }
  }

  // writes here set the lower 5 bits of the ROM bank
  // or the lower 7 bits (MBC3)
  if (address >= 0x2000 && address <= 0x3FFF && rom_banks > 2) {
    if (memory_bank_controller_ >= CartridgeType::kMBC1 &&
        memory_bank_controller_ <= CartridgeType::kMBC1wRAMwBattery) {
      // clear the top 2 bits
      bitmask_clear(value, 0xE0);
      // clear the bottom 5 from teh current selected bank val
      bitmask_clear(active_rom_bank_, 0x1F);
      // bitmask_set(active_rom_bank_, value);
      active_rom_bank_ |= value;
      SelectRomBank(active_rom_bank_);
    } else if (memory_bank_controller_ >= CartridgeType::kMBC3 &&
               memory_bank_controller_ <= CartridgeType::kMBC3wRAMwBattery) {
      // clear bit 7
      bit_clear(value, 7);
      // clear bits 6 through 0 of the active selected bank
      bitmask_clear(active_rom_bank_, 0x7F);
      active_rom_bank_ |= value;
      SelectRomBank(active_rom_bank_);
    }
    return;
  }
  // select the RAM/Upper bits of ROM bank (MBC1)
  // TODO: Ram Bank/RTC Register Write (MBC3)
  if (address >= 0x4000 && address <= 0x5FFF) {
    switch (memory_bank_controller_) {
      case CartridgeType::kMBC1:
      case CartridgeType::kMBC1wRAM:
      case CartridgeType::kMBC1wRAMwBattery:
        // this is only a 2 bit register, so clear the rest
        bitmask_clear(value, 0xFC);
        // switch RAM banks
        if (ram_banking_mode_) {
          spdlog::get("stdout")->debug("RAM bank change @ {0:04X} - {1:02X}",
                                       address, value);
          SelectRamBank(value);
        }
        // otherwise set the top two bits of the ROM bank
        else {
          spdlog::get("stdout")->debug(
              "upper two bits of rom bank @ {0:04X} - {1:02X}", address, value);
          bitmask_clear(active_rom_bank_, 0xE0);
          bitmask_set(active_rom_bank_, value << 5U);
          // change rom bank
          SelectRomBank(active_rom_bank_);
          // TODO: select ram bank 0
          SelectRamBank(0);
        }
        break;
      case CartridgeType::kMBC3wTimerwBattery:
      case CartridgeType::kMBC3wTimerwRAMwBattery:
      case CartridgeType::kMBC3:
      case CartridgeType::kMBC3wRAM:
      case CartridgeType::kMBC3wRAMwBattery:
        if (value <= 0x07) {
          // Select the appropriate RAM bank
          spdlog::get("stdout")->debug("MBC3 Ram bank switch");
          SelectRamBank(value);
        } else if (value <= 0x0C) {
          // Select the appropriate RTC Register to map
          spdlog::get("stdout")->debug("MBC3 RTC reg write: {0:02x}", value);
        }
        break;
      default:
        break;
    }
    return;
  }
  // Rom/Ram mode (MBC1)
  // TODO: Latch Clock Data (MBC3)
  if (address >= 0x6000 && address <= 0x7FFF) {
    if (memory_bank_controller_ == CartridgeType::kMBC1wRAM ||
        memory_bank_controller_ == CartridgeType::kMBC1wRAMwBattery) {
      // clear all but only the lowest 2 bits?
      bitmask_clear(value, 0xFE);
      spdlog::get("stdout")->debug("rom/ram mode set @ {0:04X} - {1:02X}",
                                   address, value);
      ram_banking_mode_ = value;
      if (ram_banking_mode_) {
        // clear the upper two bits of the selected rom bank
        // since we're using those bits to select the ram bank now
        bitmask_clear(active_rom_bank_, 0xC0);
        SelectRomBank(active_rom_bank_);
      } else {
        // select ram bank 0
        SelectRamBank(0);
      }
    } else if (memory_bank_controller_ >= CartridgeType::kMBC3 &&
               memory_bank_controller_ <= CartridgeType::kMBC3wRAMwBattery) {
      spdlog::get("stdout")->debug("MBC3 RTC Latch access");
    }
    return;
  }
  const uint8_t ppu_mode = memory_[STAT] & 0x03;
  // Vram inaccessible during mode 3
  if (address >= 0x8000 && address <= 0x9FFF &&
      ppu_mode == kPPUModeLCDTransfer) {
    return;
  }
  if (address >= 0xA000 && address <= 0xBFFF) {
    // spdlog::get("stdout")->debug("External ram access @ {0:04X} - {1:02X}",
    // loc, val);
    cart_ram_modified = true;
  }
  // no writing to ROM
  if (address <= 0x8000) return;
  // write to "mirror" ram too
  if (address >= 0xC000 && address < 0xDE00) {
    memory_[address] = value;
    memory_[address + 0x2000] = value;
    return;
  }
  // Writes to DIV reset it
  if (address == DIV) {
    Gameboy::SetTimer(0);
    // spdlog::get("stdout")->debug("Timer divider set to 0");
  }

  // any writing to 0xFF44 resets it
  if (address == 0xFF44) {
    value = 0;
  }

  // TODO: disallow writing to vram (0x8000 - 0x9FFF) unless in modes 0 - 2
  if ((address >= 0x8000 && address < 0xA000) &&
      ppu_mode == kPPUModeLCDTransfer) {
    spdlog::get("stdout")->debug("Invalid VRam Access @ {0:04X}");
    return;
  }
  // Joypad writes (for button/direction selection only change bits 4/5)
  if (address == P1) {
    // spdlog::get("stdout")->debug("write to P1: {0:02x}", val);
    // clear the 2 selection bits
    // bitmask_clear(memory_[loc], 0x30);
    bitmask_set(memory_[address], value);
    switch ((value >> 4) & 0x03) {
      case 0x01:
        // start, sel, a, b selected
        bitmask_clear(memory_[address], 0x0f);
        bitmask_set(memory_[address], Gameboy::joypad[0] & 0xf);
        break;
      case 0x02:
        // direction pad
        bitmask_clear(memory_[address], 0x0f);
        bitmask_set(memory_[address], Gameboy::joypad[1] & 0xf);
        break;
      case 0x03:
        // any button?
        bitmask_clear(memory_[address], 0x0f);
        bitmask_set(memory_[address],
                    (Gameboy::joypad[0] | Gameboy::joypad[1]) & 0xf);
        break;
      default:
        spdlog::get("stdout")->error("Incorrect value in P1: {0:02x}", value);
        break;
    }
    return;
  }

  // OAM Data Transfer, only possible during modes 0 and 1
  // unless lcd disabled
  if (address == DMA &&
      (ppu_mode <= kPPUModeVBlank || bit_check(memory_[LCDC], 7))) {
    // val is the MSB of our source xfer address
    const uint16_t src = value << 8;
    // bottom of OEM Ram
    const uint16_t dest = 0xFE00;
    for (auto i = 0; i < 160; i += 4) {
      memory_[dest + i] = memory_[src + i];
      memory_[dest + (i + 1)] = memory_[src + (i + 1)];
      memory_[dest + (i + 2)] = memory_[src + (i + 2)];
      memory_[dest + (i + 3)] = memory_[src + (i + 3)];
    }
    return;
  }

  memory_.at(address) = value;
  // memory_[loc] = val;
}

void MMU::SelectRomBank(uint8_t bank) {
  // only mbc1 is unable to access those banks
  if ((memory_bank_controller_ == CartridgeType::kMBC1 ||
       memory_bank_controller_ == CartridgeType::kMBC1wRAM ||
       memory_bank_controller_ == CartridgeType::kMBC1wRAMwBattery) &&
      (bank == 0 || bank == 0x20 || bank == 0x40 || bank == 0x60)) {
    ++bank;
  } else if ((memory_bank_controller_ >= CartridgeType::kMBC3 &&
              memory_bank_controller_ <= CartridgeType::kMBC3wRAMwBattery) &&
             bank == 0) {
    ++bank;  // writing a 0 selects bank 1
  }

  bank %= rom_banks;
  // spdlog::get("stdout")->debug("selecting rom bank {0}", bank);
  std::copy(cart_rom_banks_[bank].begin(), cart_rom_banks_[bank].end(),
            memory_.begin() + 0x4000);
}

void MMU::SelectRamBank(uint8_t bank) {
  // save the current ram banks state first
  std::copy(memory_.begin() + 0xA000, memory_.begin() + 0xC000,
            ram_banks_[active_ram_bank_].begin());
  // should we need this?
  num_ram_banks > 0 ? active_ram_bank_ = bank % num_ram_banks
                    : active_ram_bank_ = 0;
  spdlog::get("stdout")->debug("Selected RAM bank {0}", active_ram_bank_);
  std::copy(ram_banks_[active_ram_bank_].begin(),
            ram_banks_[active_ram_bank_].end(), memory_.begin() + 0xA000);
}

void MMU::SetRegister(uint16_t reg, uint8_t val) {
  // Limit our access to only hardware registers
  // Use the proper WriteByte access for the rest of memory
  // This is so we can set registers when needed while allowing actual writes
  // to them to reset as appropriate or whatever.
  if (reg < 0xFF00) return;

  memory_[reg] = val;
}

uint8_t MMU::GetRegister(uint16_t reg) {
  if (reg < 0xFF00) return 0xFF;
  return memory_[reg];
}
