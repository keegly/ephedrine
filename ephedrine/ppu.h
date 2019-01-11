#ifndef GPU_H
#define GPU_H

#include <cstdint>
#include <memory>
#include <array>
#include "mmu.h"

// Special Registers
constexpr uint16_t LCDC = 0xFF40;
constexpr uint16_t STAT = 0xFF41;
constexpr uint16_t SCY  = 0xFF42;
constexpr uint16_t SCX  = 0xFF43;
constexpr uint16_t LY   = 0xFF44;
constexpr uint16_t BGP  = 0xFF47;

struct Pixel {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

class PPU {
public:
	PPU(MMU& m);
	bool vblank;
	void update(int cycles);
	std::unique_ptr<uint8_t[]> refresh();
	std::unique_ptr<uint8_t[]> refresh_bg();
	void print();
private:
	MMU& mmu;
	Pixel pixels[160][144]; // 160x144 screen, 3 bytes per pixel 
	Pixel get_color(uint8_t tile);
	const Pixel palette[4]{
		{(uint8_t)0xff, (uint8_t)0xff, (uint8_t)0xff},
		{ (uint8_t)211,(uint8_t)211,(uint8_t)211 },
		{(uint8_t)102,(uint8_t)102,(uint8_t)102},
		{(uint8_t)0,(uint8_t)0,(uint8_t)0}
	};
	std::array<Pixel, 4> p{ {
		{(uint8_t)0xff, (uint8_t)0xff, (uint8_t)0xff},
		{ (uint8_t)211,(uint8_t)211,(uint8_t)211 },
		{(uint8_t)102,(uint8_t)102,(uint8_t)102},
		{(uint8_t)0,(uint8_t)0,(uint8_t)0}
	} };
	int curr_scanline_cycles; // 456 per each individual scanline
	bool finished_current_line = false;
	bool oam_done = false;
	int visible_sprites[144]; // visible sprites for each line
};

#endif // !GPU_H
