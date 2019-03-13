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

// Sprite Attributes/Flags
constexpr uint8_t OBJ_BG_PRIORITY = 0x80; // bit 7
constexpr uint8_t Y_FLIP = 0x40;		  // bit 6
constexpr uint8_t X_FLIP = 0x20;		  // bit 5
constexpr uint8_t PAL_NUM = 0x10;		  // bit 4o

struct Pixel {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
	uint8_t palette;
	bool sprite;
};

struct Sprite {
	uint8_t y;
	uint8_t x;
	uint8_t tile;
	uint8_t flags;
	uint16_t oam_addr;
};
enum class PixelSource {
		bgp,
		ogp0,
		ogp1
	};
struct PixelFIFO {
	uint8_t tile;
	PixelSource source;
};

class PPU {
public:
	PPU(std::shared_ptr<MMU> m);
	bool vblank;
	void update(int cycles);
	std::unique_ptr<uint8_t[]> render() const;
	std::unique_ptr<uint8_t[]> render_bg() const;
	std::unique_ptr<uint8_t[]> render_tiles() const;
	void print();
	inline uint8_t get_mode();
	// debugging ui
	std::vector<Sprite> get_visible_sprites() const {
		return visible_sprites;
	}
	template<typename OStream>
	friend OStream &operator<<(OStream &os, const PPU &p)
	{
		os << "Curr scanline cycles: " << std::dec << p.curr_scanline_cycles << " VBl: " << p.vblank;
		return os;
	}
private:
	std::shared_ptr<MMU> mmu;
	//std::array<std::array<Pixel, 160>, 144> pixels;
	Pixel pixels[144][160]; // 160x144 screen, 4 bytes per pixel
	Pixel get_color(uint8_t tile) const;
	Pixel get_sprite_color(uint8_t tile, bool obp1);
	Pixel palette[4]{
		{(uint8_t)224, (uint8_t)248, (uint8_t)208, (uint8_t)0xff},  // white
		{(uint8_t)136,(uint8_t)192,(uint8_t)112, (uint8_t)0xff},  // light grey
		{(uint8_t)52,(uint8_t)104,(uint8_t)86, (uint8_t)0xff},   // dark grey
		{(uint8_t)8,(uint8_t)24,(uint8_t)32, (uint8_t)0xff} // black
	};
	void set_mode(uint8_t mode);
	int curr_scanline_cycles; // 456 per each individual scan line
	bool finished_current_line;
	bool oam_done;
	// visible sprites for each line
	std::vector<Sprite> visible_sprites;
};

#endif // !PPU_H
