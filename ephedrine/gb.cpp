#include <memory>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"


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
}

void Gameboy::tick(int ticks)
{
	// step x ticks
	if (!cpu.halted)
		cpu.step();
	//handle interrupts
	ppu.update(cpu.cycles);
}

//void Gameboy::handle_interrupts()
//{
//	if (!ime)
//		return; // interrupts globally disabled
//	// check register 0xFF0F to see which interrupt was generated
//	const uint16_t offset[]{ 0x0040, 0x0048, 0x0050, 0x0058, 0x0060 };
//	uint16_t address = 0x0000;
//	uint8_t mask = 0x01;
//	for (uint8_t i = 0; i < 5; ++i) {
//		if (mask && memory[0xFF0F]) {
//			address = offset[i];
//			// clear teh bit
//			memory[0xFF0F] &= ~(1U << i);
//			break;
//		}
//		mask <<= 1;
//	}
//	if (address == 0x0000) return;
//	// put current pc on stack
//	memory[sp] = pc >> 8;
//	memory[sp - 1] = pc;
//	sp -= 2;
//
//	// set pc to the correct interrupt handler address
//	pc = address;
//}