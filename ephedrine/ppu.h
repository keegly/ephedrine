#ifndef PPU_H
#define PPU_H

#include <cstdint>
#include <memory>
#include <array>
#include "mmu.h"


// Modes
constexpr uint8_t PPU_MODE_HBLANK	  = 0x00;
constexpr uint8_t PPU_MODE_VBLANK	  = 0x01;
constexpr uint8_t PPU_MODE_OAM_SEARCH = 0x02;
constexpr uint8_t PPU_MODE_LCD_XFER	  = 0x03;

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
	std::unique_ptr<uint8_t[]> render() const;
	std::unique_ptr<uint8_t[]> render_bg();
	std::unique_ptr<uint8_t[]> render_tiles();
	void print();
	inline uint8_t get_mode();
private:
	MMU& mmu;
	Pixel pixels[144][160]; // 160x144 screen, 3 bytes per pixel
	Pixel get_color(uint8_t tile);
	Pixel palette[4]{
		{(uint8_t)0xff, (uint8_t)0xff, (uint8_t)0xff},
		{ (uint8_t)211,(uint8_t)211,(uint8_t)211 },
		{(uint8_t)102,(uint8_t)102,(uint8_t)102},
		{(uint8_t)0,(uint8_t)0,(uint8_t)0}
	};
	void set_mode(uint8_t mode);
	int curr_scanline_cycles; // 456 per each individual scan line
	bool finished_current_line;
	bool oam_done;
	int visible_sprites[144]; // visible sprites for each line
};

#endif // !PPU_H
