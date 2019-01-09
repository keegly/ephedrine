#ifndef GB_H
#define GB_H

#include "cpu.h"
#include "mmu.h"
#include "ppu.h"

class Gameboy
{
	public:
		Gameboy(std::vector<uint8_t> cart);
		void reset();
		void handle_interrupts();
		void tick(int ticks);
		void load(std::vector<uint8_t> cart);
		CPU cpu;
		MMU mmu;
		PPU ppu;
	private:
	
};

#endif // !GB_H