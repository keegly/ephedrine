#ifndef GPU_H
#define GPU_H

#include <cstdint>
#include <memory>
#include "mmu.h"

// Special Registers
constexpr uint16_t LCDC = 0xFF40;
constexpr uint16_t STAT = 0xFF41;
constexpr uint16_t SCY = 0xFF42;
constexpr uint16_t SCX = 0xFF43;
constexpr uint16_t LY = 0xFF44;

struct Tile {

};

class PPU {
public:
	PPU(MMU& m);
	bool vblank;
	void update(int cycles);
	std::unique_ptr<std::vector<uint8_t>> refresh();
	void print();
private:
	MMU& mmu;
	uint8_t pixels[160][144]{}; // 160x144 screen, 1 bytes per pixel
	int curr_scanline_cycles; // 456 per each individual scanline
	bool finished_current_line = false;
	int visible_sprites[144]; // visible sprites for each line
};

#endif // !GPU_H
