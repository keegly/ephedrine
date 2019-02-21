#include <cstdint>
#include <vector>
#include <iostream>

#include "spdlog/spdlog.h"
#include "bit_utility.h"
#include "mmu.h"
#include "ppu.h"
#include "gb.h"

MMU::MMU()
{
	boot_rom_enabled = true;
	cartridge.clear();
	rom_banks = 0;
	ram_banks = 0;
}
MMU::MMU(std::vector<uint8_t> cart) : cartridge(cart)
{
	//boot_rom_enabled = false;
	load(cartridge);
}

void MMU::load(std::vector<uint8_t> c)
{
	boot_rom_enabled = false;
	if (c.empty()) return;

	rom_banks = (32 << cartridge[0x0148]) / 16;
	spdlog::get("stdout")->debug("Rom Banks: {0}", rom_banks);
	ram_banks = cartridge[0x0149];
	spdlog::get("stdout")->debug("Ram Banks: {0:04X}", ram_banks);
	if (rom_banks <= 2) {
		int i = 0;
		for (uint8_t byte : cartridge) {
			memory[i] = byte;
			++i;
		}
	}
	else {
		// load the ROM and the first switchable bank, break the cart into it's
		// individual 8kB banks, and store them for easy usage later
		cart_rom_banks.resize(rom_banks);
		for (int i = 0; i < 0x8000; ++i) {
			memory[i] = cartridge[i];
		}

		int count = 0;
		int index = 0;
		for (int i = 0; i < rom_banks; ++i) {
			while (count < 0x4000) {
				cart_rom_banks[i][count] = c[index];
				++count;
				++index;
			}
			count = 0;
		}
	}
}

void MMU::set_ppu_mode(uint8_t mode)
{
	// invalid mode
	if (mode > 0x03)
		return;

	bitmask_clear(memory[STAT], 0x03);
	bitmask_set(memory[STAT], mode);
}

uint8_t MMU::read_byte(uint16_t loc)
{
	if (boot_rom_enabled && loc <= 0xFF)
		return boot_rom[loc];
	if (cartridge.empty())
		return 0xFF;
	// Read from a ROM bank
	/*if (rom_banks > 2 && loc >= 0x4000 && loc <= 0x7FFF) {
		return cart_rom_banks[active_rom_bank][loc - 0x4000];
	}*/
	// joypad bits 6 and 7 always return 1
	if (loc == 0xFF00) {
		bitmask_set(memory[loc], 0xC0); // should be C0
	}
	//if (loc == 0xFF80)
//		spdlog::get("stdout")->debug("ff80 read: {0:02X}", memory[loc]);

	return memory[loc]; // needs more logic regarding certain addresses returning FF at certain times etc

	// if reading from OAM in mode 2, return 0xFF
	// if no cartridge inserted, reads to ROM/RAM return 0xFF
	// bank N, mem[0x4000 - 0x7FFF] = cartridge[0x4000 * N - 0x7FFF *  N]
}

void MMU::write_byte(uint16_t loc, uint8_t val)
{
	// check for bank switching here?
	if (loc == 0xFF50 && val == 0x01)
		boot_rom_enabled = false;

	// writes here set the lower 5 bits of the ROM bank
	if (loc >= 0x2000 && loc <= 0x3FFF && rom_banks > 2) {
		spdlog::get("stdout")->debug("bank switching @ {0:04x}: {1:02x}", loc, val);
		if (val == 0 || val == 20 || val == 40 || val == 60) ++val;

		active_rom_bank = val % rom_banks;
		spdlog::get("stdout")->debug("selecting rom bank {0}", active_rom_bank);
		for (int i = 0; i < 0x4000; ++i) {
			memory[0x4000 + i] = cart_rom_banks[val % rom_banks][i];
		}
		return;
	}
	// select the RAM/Upper bits of ROM bank
	if (loc >= 0x4000 && loc <= 0x5FFF) {
		spdlog::get("stdout")->debug("ram/upper bits of rom bank @ {0:04X} - {1:02X} val", loc, val);
		return;
	}
	// Rom/Ram mode
	if (loc >= 0x6000 && loc <= 0x7FFF) {
		spdlog::get("stdout")->debug("rom/ram mode set @ {0:04X} - {1:02X} val", loc, val);
		return;
	}
	if (loc >= 0xA000 && loc <= 0xBFFF) {
		//spdlog::get("stdout")->debug("External ram access @ {0:04X} - {1:02X}", loc, val);
	}
	// no writing to ROM
	if (loc <= 0x8000)
		return;
	// write to "mirror" ram too
	if (loc >= 0xC000 && loc < 0xDE00) {
		memory[loc] = val;
		memory[loc + 0x2000] = val;
		return;
	}
	// Writes to DIV reset it
	if (loc == DIV)
		Gameboy::set_timer(0);

	// deal with read only bits on the LCD STAT register
	//if (loc == STAT) {
	//	uint8_t temp = val;
	//	bitmask_clear(val, 0x03);
	//	bitmask_set(memory[loc], val);
	//	return;
	//}

	// any writing to 0xFF44 resets it
	/*if (loc == 0xFF44)
		val = 0;*/
		// TODO: disallow writing to vram (0x8000 - 0x9FFF) unless in modes 0 - 2
		//if ((loc >= 0x8000 && loc < 0xA000))
		// TODO: disallow writing to oam  (0xFE00 - 0xFE9F) unless in modes 0 - 1
		// ^^ unless display is disabled
		// Joypad writes (for button/direction selection only change bits 4/5)
	if (loc == 0xFF00) {
		bitmask_clear(memory[loc], 0x30);
		bitmask_set(memory[loc], val & 0x30);
		//val = memory[loc] | val;
		return;
	}

	// OAM Data Transfer
	if (loc == 0xFF46) {
		// val is the MSB of our source xfer address
		uint16_t src = val << 8;
		// bottom of OEM Ram
		uint16_t dest = 0xFE00;
		for (int i = 0; i < 160; i += 4)
		{
			memory[dest + i] = memory[src + i];
			memory[dest + (i + 1)] = memory[src + (i + 1)];
			memory[dest + (i + 2)] = memory[src + (i + 2)];
			memory[dest + (i + 3)] = memory[src + (i + 3)];
		}
		return;
	}


	//spdlog::get("stdout")->debug("FF00 write: {0:02X}", val);
//if (loc == 0xFF80)
	//spdlog::get("stdout")->debug("FF80 write: {0:02X}", val);

	memory[loc] = val;
}

void MMU::set_register(uint16_t reg, uint8_t val)
{
	// Limit our access to only hardware registers
	// Use the proper write_byte access for the rest of memory
	// This is so we can set registers when needed while allowing actual writes
	// to them to reset as appropriate or whatever.
	if (reg < 0xFF00)
		return;

	memory[reg] = val;
}

uint8_t MMU::get_register(uint16_t reg)
{
	if (reg < 0xFF00) return 0;
	return memory[reg];
}
