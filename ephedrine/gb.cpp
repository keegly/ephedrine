#include <memory>

#include "spdlog/spdlog.h"

#include "gb.h"
#include "cpu.h"
#include "mmu.h"
#include "ppu.h"

//#include "instructions.h"


Gameboy::Gameboy(std::vector<uint8_t> cart) : mmu(cart), cpu(mmu), ppu(mmu)
{
}

void Gameboy::load(std::vector<uint8_t> cartridge)
{
	mmu.load(cartridge);
	mmu.boot_rom_enabled = false;
}

void Gameboy::tick(int ticks)
{
	// step x ticks
	if (cpu.halted)
		return;


	cpu.step();
	//handle interrupts
	cpu.handle_interrupts();
	ppu.update(cpu.cycles);
}