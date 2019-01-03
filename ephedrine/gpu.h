#ifndef GPU_H
#define GPU_H

#include <cstdint>
#include "mmu.h"

// Special Registers
#define LCDC	0xFF40
#define STAT	0xFF41
#define SCY		0xFF42
#define SCX		0xFF43
#define LY		0xFF44

class GPU {
public:
	GPU(MMU& m);
	bool vblank;
	//uint8_t SCY;
	//uint8_t SCX;
	void update(int cycles);
	uint8_t *render();
private:
	MMU& mmu;
	uint8_t gfx[160*144*1]{}; // 160x144 screen, 1 bytes per pixel
	int curr_scanline_cycles; // 456 per each individual scanline
};

#endif // !GPU_H
