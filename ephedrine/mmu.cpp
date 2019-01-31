#include <cstdint>
#include <vector>
#include <iostream>

#include "logger.h"
#include "bit_utility.h"
#include "mmu.h"
#include "ppu.h"

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
	//boot_rom_enabled = true;
	if (c.empty()) return;

	rom_banks = (32 << cartridge[0x0148]) / 16;
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
		Logger::logger->info("bank switching?: {0:04x}", val);
		if (val == 0 || val == 20 || val == 40 || val == 60) ++val;

		for (int i = 0; i < 0x4000; ++i) {
			memory[0x4000 + i] = cart_rom_banks[val % rom_banks][i];
		}
	}
	// no writing to ROM
	if (loc <= 0x8000)
		return;
	if (loc >= 0xC000 && loc < 0xDE00) {
		// write to "mirror" ram too
		memory[loc] = val;
		memory[loc + 0x2000] = val;
	}

	/*if (loc == 0xFF02 && val == 0x81) {
		Logger::logger->debug((char)this->memory[0xFF01]);
	}*/

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

	memory[loc] = val;
}