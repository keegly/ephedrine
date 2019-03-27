#include <cstdint>
#include <vector>
#include <fstream>
#include <iostream>

#include "spdlog/spdlog.h"
#include "bit_utility.h"
#include "mmu.h"
#include "ppu.h"
#include "gb.h"

MMU::MMU()
{
	boot_rom_enabled = true;
	ram_enabled = false;
	cartridge_.clear();
	rom_banks = 0;
	num_ram_banks = 0;
	active_ram_bank = 0;
}
MMU::MMU(std::vector<uint8_t> &cart, bool boot_rom) : cartridge_(cart), boot_rom_enabled(boot_rom)
{
	load(cartridge_);
}

void MMU::load(std::vector<uint8_t> c)
{
	if (c.empty()) return;

	rom_banks = (32 << cartridge_[0x0148]) / 16;
	spdlog::get("stdout")->debug("Rom Banks: {0}", rom_banks);
	num_ram_banks = cartridge_[0x0149];
	spdlog::get("stdout")->debug("Ram Banks: {0:04X}", num_ram_banks);
	memory_bank_controller_ = static_cast<CartridgeType>(cartridge_[0x0147]);
	if (num_ram_banks > 1) {
		ram_banks_.resize(num_ram_banks);
	}
	else {
		ram_banks_.resize(1);
	}
	if (rom_banks <= 2) {
		int i = 0;
		for (uint8_t byte : cartridge_) {
			memory_[i] = byte;
			++i;
		}
	}
	else {
		// load the ROM and the first switchable bank, break the cart into it's
		// individual 8kB banks, and store them for easy usage later
		cart_rom_banks_.resize(rom_banks);
		for (int i = 0; i < 0x8000; ++i) {
			memory_[i] = cartridge_[i];
		}

		int count = 0;
		int index = 0;
		for (int i = 0; i < rom_banks; ++i) {
			while (count < 0x4000) {
				cart_rom_banks_[i][count] = c[index];
				++count;
				++index;
			}
			count = 0;
		}
	}
}

void MMU::ShowDebugWindow()
{
}

void MMU::SetPPUMode(uint8_t mode)
{
	// invalid mode
	if (mode > 0x03)
		return;

	//spdlog::get("stdout")->debug("setting ppu mode {0}", mode);

	bitmask_clear(memory_[STAT], 0x03);
	bitmask_set(memory_[STAT], mode);
}

void MMU::SaveBufferedRAM(std::ofstream &ofs)
{
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
	case CartridgeType::kMBC7wSensorwRumblewRAMwBattery:
		for (int i = 0xA000; i < 0xC000; ++i) {
			ofs << memory_[i];
		}
		break;
	default:
		spdlog::get("stdout")->info("Cartridge type {0} not battery buffered, not saving", static_cast<uint8_t>(memory_bank_controller_));
		break;
	}
}

void MMU::LoadBufferedRAM(std::ifstream &ifs)
{
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
	case CartridgeType::kMBC7wSensorwRumblewRAMwBattery:
		for (int i = 0xA000; i < 0xC000; ++i) {
			ifs >> memory_[i];
		}
		break;
	default:
		spdlog::get("stdout")->info("{0} not battery buffered", static_cast<uint8_t>(memory_bank_controller_));
		break;
	}
}

uint8_t MMU::ReadByte(uint16_t loc)
{
	if (boot_rom_enabled && loc <= 0xFF)
		return boot_rom_[loc];
	if (cartridge_.empty())
		return 0xFF;
	// PPU mode
	uint8_t ppu_mode = memory_[STAT] & 0x03;
	// Read from a ROM bank
	/*if (rom_banks > 2 && loc >= 0x4000 && loc <= 0x7FFF) {
		return cart_rom_banks_[active_rom_bank][loc - 0x4000];
	}*/
	// joypad bits 6 and 7 always return 1
	if (loc == P1) {
		bitmask_set(memory_[loc], 0xC0); // should be C0
	}
	//bit 7 unused and always returns 1, bits 0-2 return 0 when LCD is off
	if (loc == STAT) {
		bitmask_set(memory_[loc], 0x80);
		if (!bit_check(LCDC, 7)) {
			bitmask_clear(memory_[loc], 0x03);
		}
	}
	// Vram inaccessible during mode 3
	//if (loc >= 0x8000 && loc <= 0x9FFF && ppu_mode == kPPUModeLCDTransfer) {
	//	return 0xFF;
	//}
	// RAM has to be enabled to read from it
	if (loc >= 0xA000 && loc <= 0xBFFF && !ram_enabled) {
		return 0xFF;
	}
	if (loc >= 0xA000 && loc <= 0xBFFF) {
		//return ram_banks_[active_ram_bank].at(loc - 0xA000);
	}
	// Only able to read from OAM in H-Blank and V-Blank
	//if (loc >= 0xFE00 && loc <= 0xFE9F && ppu_mode >= kPPUModeOAMSearch) {
	//	return 0xFF;
	//}

	return memory_[loc]; // needs more logic regarding certain addresses returning FF at certain times etc

	// if reading from OAM in mode 2, return 0xFF
	// if no cartridge_ inserted, reads to ROM/RAM return 0xFF
	// bank N, mem[0x4000 - 0x7FFF] = cartridge_[0x4000 * N - 0x7FFF *  N]
}

void MMU::WriteByte(uint16_t loc, uint8_t val)
{
	if (loc == 0xFF50 && val == 0x01)
		boot_rom_enabled = false;

	// RAM Enable
	if (loc >= 0 && loc <= 0x1FFF) {
		//spdlog::get("stdout")->debug("RAM enable @ {0:04x}: {1:02x}", loc, val);
		BITMASK_CHECK_ALL(val, 0x0A) ? ram_enabled = true : ram_enabled = false;
		//spdlog::get("stdout")->debug("ram_enabled = {0}", ram_enabled);
	}

	// writes here set the lower 5 bits of the ROM bank
	if (loc >= 0x2000 && loc <= 0x3FFF && rom_banks > 2) {
		//spdlog::get("stdout")->debug("bank switching @ {0:04x}: {1:02x}", loc, val);
		if (val == 0 || val == 20 || val == 40 || val == 60) ++val;

		active_rom_bank = val % rom_banks;
		//spdlog::get("stdout")->debug("selecting rom bank {0}", active_rom_bank);
		for (int i = 0; i < 0x4000; ++i) {
			memory_[0x4000 + i] = cart_rom_banks_[val % rom_banks][i];
		}
		return;
	}
	// select the RAM/Upper bits of ROM bank
	if (loc >= 0x4000 && loc <= 0x5FFF) {
		// switch RAM banks
		if (ram_banking_mode) {
			std::copy(memory_.begin() + 0xA000, memory_.begin() + 0xC000, ram_banks_[active_ram_bank].begin());
			active_ram_bank = val % num_ram_banks;
			std::copy(ram_banks_[active_ram_bank].begin(), ram_banks_[active_ram_bank].end(), memory_.begin() + 0xA000);
			// TODO
			// copy bytes from ram to old ram bank in vector and
			// then copy the bytes from the new active bank into memory
		}
		// otherwise set the top two bits of the ROM bank
		else {
			bitmask_clear(active_rom_bank, 0xC0);
			bitmask_set(active_rom_bank, val << 6);
		}
		//spdlog::get("stdout")->debug("ram/upper bits of rom bank @ {0:04X} - {1:02X} val", loc, val);
		return;
	}
	// Rom/Ram mode
	if (loc >= 0x6000 && loc <= 0x7FFF) {
		//spdlog::get("stdout")->debug("rom/ram mode set @ {0:04X} - {1:02X} val", loc, val);
		// TODO: if mode 0, switch to ram bank 0 (and stay)
		ram_banking_mode = val;
		return;
	}
	uint8_t ppu_mode = memory_[STAT] & 0x03;
	// Vram inaccessible during mode 3
	if (loc >= 0x8000 && loc <= 0x9FFF && ppu_mode == kPPUModeLCDTransfer) {
		return;
	}
	if (loc >= 0xA000 && loc <= 0xBFFF) {
		//spdlog::get("stdout")->debug("External ram access @ {0:04X} - {1:02X}", loc, val);
		//if (num_ram_banks > 0) {
		//	ram_banks_[active_ram_bank].at(loc - 0xA000) = val;
		//}
		cart_ram_modified = true;
	}
	// no writing to ROM
	if (loc <= 0x8000)
		return;
	// write to "mirror" ram too
	if (loc >= 0xC000 && loc < 0xDE00) {
		memory_[loc] = val;
		memory_[loc + 0x2000] = val;
		return;
	}
	// Writes to DIV reset it
	if (loc == DIV) {
		Gameboy::SetTimer(0);
		//spdlog::get("stdout")->debug("Timer divider set to 0");
	}

	// any writing to 0xFF44 resets it
	/*if (loc == 0xFF44)
		val = 0;*/
		// TODO: disallow writing to vram (0x8000 - 0x9FFF) unless in modes 0 - 2
		//if ((loc >= 0x8000 && loc < 0xA000))
		// TODO: disallow writing to oam  (0xFE00 - 0xFE9F) unless in modes 0 - 1
		// ^^ unless display is disabled
		// Joypad writes (for button/direction selection only change bits 4/5)
	if (loc == P1) {
		//spdlog::get("stdout")->debug("write to P1: {0:02x}", val);
		// clear the 2 selection bits
		//bitmask_clear(memory_[loc], 0x30);
		bitmask_set(memory_[loc], val);
		switch ((val >> 4) & 0x03) {
		case 0x01:
			// start, sel, a, b selected
			bitmask_clear(memory_[loc], 0x0f);
			bitmask_set(memory_[loc], Gameboy::joypad[0] & 0xf);
			break;
		case 0x02:
			// direction pad
			bitmask_clear(memory_[loc], 0x0f);
			bitmask_set(memory_[loc], Gameboy::joypad[1] & 0xf);
			break;
		case 0x03:
			// any button?
			bitmask_clear(memory_[loc], 0x0f);
			bitmask_set(memory_[loc], (Gameboy::joypad[0] | Gameboy::joypad[1]) & 0xf);
			break;
		default:
			spdlog::get("stdout")->error("Incorrect value in P1: {0:02x}", val);
			break;
		}
		return;
	}

	// OAM Data Transfer, only possible during modes 0 and 1
	if (loc == DMA && ppu_mode <= kPPUModeVBlank) {
		// val is the MSB of our source xfer address
		uint16_t src = val << 8;
		// bottom of OEM Ram
		uint16_t dest = 0xFE00;
		for (int i = 0; i < 160; i += 4)
		{
			memory_[dest + i] = memory_[src + i];
			memory_[dest + (i + 1)] = memory_[src + (i + 1)];
			memory_[dest + (i + 2)] = memory_[src + (i + 2)];
			memory_[dest + (i + 3)] = memory_[src + (i + 3)];
		}
		return;
	}


	memory_.at(loc) = val;
	//memory_[loc] = val;
}

void MMU::SetRegister(uint16_t reg, uint8_t val)
{
	// Limit our access to only hardware registers
	// Use the proper WriteByte access for the rest of memory_
	// This is so we can set registers when needed while allowing actual writes
	// to them to reset as appropriate or whatever.
	if (reg < 0xFF00)
		return;

	memory_[reg] = val;
}

uint8_t MMU::GetRegister(uint16_t reg)
{
	if (reg < 0xFF00) return 0;
	return memory_[reg];
}
