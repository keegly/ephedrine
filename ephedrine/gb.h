#ifndef GB_H
#define GB_H

#include "cpu.h"
#include "mmu.h"
#include "gpu.h"

class Gameboy
{
	public:
		Gameboy(std::vector<uint8_t> cart);
		void reset();
		void handle_interrupts();
		void load(std::vector<uint8_t> cart);
		CPU cpu;
		MMU mmu;
		GPU ppu;
	private:
	
};

#endif // !GB_H