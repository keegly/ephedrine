#include <cstdint>
#include <vector>

#include "mmu.h"
#include "gpu.h"

MMU::MMU(uint8_t *cart, long size) : cartridge(cart, cart+size)
{
	load(cartridge);
}

MMU::MMU(std::vector<uint8_t> cart) : cartridge(cart)
{
	load(cartridge);
}

void MMU::load(std::vector<uint8_t> c)
{
	rom_banks = (32 << cartridge[0x0148]) / 16;
	boot_rom_enabled = true;
	if (rom_banks <= 2) {
		int i = 0;
		for (uint8_t byte : cartridge) {
			memory[i] = byte;
			++i;
		}
	}
}

uint8_t MMU::read_byte(uint16_t loc)
{
	if (boot_rom_enabled && loc <= 0xFF)
		return boot_rom[loc];
	else
		return memory[loc]; // needs more logic regarding certain addresses returning FF at certain times etc
}

void MMU::write_byte(uint16_t loc, uint8_t val)
{
	// check for bank switching here?
	if (loc == 0xFF50 && val == 0x01)
		boot_rom_enabled = false;
	// no writing to ROM
	if (loc <= 0x8000)
		return;

	// any writing to 0xFF44 resets it
	/*if (loc == 0xFF44)
		val = 0;*/
	// TODO: disallow writing to vram unless in modes 0 - 2
	// TODO: disallow writing to oam unless in modes 0 - 1
	// ^^ unless display is disabled

	memory[loc] = val;
}