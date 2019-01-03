#include "gpu.h"


GPU::GPU(MMU& m) : mmu(m)
{
	curr_scanline_cycles = 0;
}

void GPU::update(int cycles)
{
	// refresh LCD
	// do each line for 456 cycles
	// one LY = 144, V-blank until 153 then reset LY to 0 and repeat
	// if disabled, LY should stay at 0
	uint8_t currLY = mmu.read_byte(LY);
	if (curr_scanline_cycles >= 456) {
		// render our line (background + sprites)
		//gfx[160 * currLY];
		// inc Ly (next line)
		mmu.write_byte(LY, currLY + 1);
		curr_scanline_cycles = 0;
	}
	if (currLY == 144) {
		// v blank (set bit 0 of 0xFF0F)
		// set mode 1 in 0xFF41
		uint8_t stat = mmu.read_byte(STAT);
		mmu.write_byte(STAT, stat |= 1U << 0);
		//gb.memory[0xFF0F] |= 1U << 0;
		vblank = true;
	}
	if (currLY > 153) {
		mmu.write_byte(LY, 0);
		// reset STAT mode
		//gb.memory[0xFF0F] &= ~(1U << 0);
		vblank = false;
	}
	curr_scanline_cycles += cycles;
}

uint8_t * GPU::render()
{
	for (int i = 0; i < 23040; ++i) {

	}
	return gfx;
}