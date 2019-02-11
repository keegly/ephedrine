#include "ppu.h"
#include "gb.h"
#include "cpu.h"
#include "bit_utility.h"


PPU::PPU(MMU& m) : mmu(m)
{
	curr_scanline_cycles = 0;
	oam_done = false;
	finished_current_line = false;
	mmu.write_byte(LCDC, 0x91);
	mmu.write_byte(STAT, 0x85);
}

void PPU::print()
{
	uint8_t currLY = mmu.read_byte(LY);
	uint8_t scx = mmu.read_byte(SCX);
	uint8_t scy = mmu.read_byte(SCY);
//	Logger::logger->debug("current scanline: {0} ({3} cycles) SCX: {1:02x} SCY: {2:02x}", currLY, scx, scy, curr_scanline_cycles);
}

uint8_t PPU::get_mode()
{
	return (mmu.read_byte(STAT) & 0x03);
}

void PPU::set_mode(uint8_t mode)
{
	mmu.set_ppu_mode(mode);
	// set STAT interrupt flag?
}

/**
 * Take a 2 bit tile code (0-3) and return the correct
 * color according to the current palette settings
 */
Pixel PPU::get_color(uint8_t tile)
{
	uint8_t bgp = mmu.read_byte(BGP);
	Pixel pixel;

	switch (tile) {
	case 0:
		// bits 0-1 of BGP
		bgp &= 3U;
		pixel = palette[bgp];
		break;
	case 1:
		// bits 2-3
		bgp = (bgp >> 2) & 3U;
		pixel = palette[bgp];
		break;
	case 2:
		// bits 4-5
		bgp = (bgp >> 4) & 3U;
		pixel = palette[bgp];
		break;
	case 3:
		// bits 6-7
		bgp = (bgp >> 6) & 3U;
		pixel = palette[bgp];
		break;
	default:
//		Logger::logger->error("Tile palette error - invalid value: {0}", tile);
		break;
	}

	return pixel;
}

/**
 * Refresh LCD one scan line at a time
 * Once LY = 144, V-blank until 153 then reset LY to 0 and repeat
 * If disabled, LY should stay at 0
 * OAM search - 20 clks - 80 machine cycles??
 * Pixel xfer - 43+ clks - 172 cycles
 * H-Blank - 51- clks - 204 cycles
 * Total - 456 cycles
 */
void PPU::update(int cycles)
{
	uint8_t lcdc = mmu.read_byte(LCDC);
	// LCD Disabled
	if (!bit_check(lcdc, 7))
		return;

	uint8_t currLY = mmu.read_byte(LY);

	if (curr_scanline_cycles <= 80 && currLY < 144 && !oam_done) {
		// OAM DMA XFER
		// set mode
		set_mode(PPU_MODE_OAM_SEARCH);
		oam_done = true;
	}
	if (curr_scanline_cycles >= 80 && curr_scanline_cycles <= (80 + 172) && currLY < 144 && !finished_current_line) {
		set_mode(PPU_MODE_LCD_XFER);
		// bg pixel xfer, if bit 0 of LCDC is set (bg enable)
		if (bit_check(lcdc, 0)) {
			uint8_t scx = mmu.read_byte(SCX);
			uint8_t scy = mmu.read_byte(SCY);
			// what line are we on?
			uint8_t ybase = scy + currLY;
			// find current bg map position
			uint16_t bg_map_address = (0x9800 | (lcdc << 0x10) | ((ybase & 0xf8) << 2) | ((scx & 0xf8) >> 3));
			// which tells us the current bg map tile number
			// leftmost?
			uint8_t tile_num = mmu.read_byte(bg_map_address);
			// which we can use to grab the actual tile bytes
			// TODO: use the register instead  of hardcoding 8000
			// grab and discard the first byte becuase that's what hardware does?
			uint16_t tileaddr = 0x8000 + (tile_num * 16);
			uint8_t tile_low = mmu.read_byte(tileaddr);
			uint8_t tile_high = mmu.read_byte(tileaddr + 1);
			int count = 0;
			// background (20 tiles wide)
			for (int i = 0; i < 20; ++i) {
				tile_num = mmu.read_byte(bg_map_address);
				tileaddr = 0x8000 + (tile_num * 16);
				// get the right vertical row of the tile
				tileaddr = tileaddr + ((ybase % 8) * 2);
				tile_low = mmu.read_byte(tileaddr);
				tile_high = mmu.read_byte(tileaddr + 1);

				for (int bit = 7; bit >= 0; --bit) {
					uint8_t bit_low = bit_check(tile_low, bit);
					//uint8_t bit_low = (tile_low >> bit) & 1U;
					uint8_t bit_high = (tile_high >> bit) & 1U;
					uint8_t palette = (bit_high << 1) | bit_low;

					// combine these two bits to get our palette code
					// figure our pixel color here
					Pixel pixel = get_color(palette);
					pixels[currLY][count] = pixel;
					++count;
				}
				bg_map_address += 1;
			}

			// if window enabled, render
			// if sprites enabled, render
			finished_current_line = true;
			//Logger::logger->debug("PPU::finished_current_line={0}", finished_current_line);
		}
	}
	if (curr_scanline_cycles >= (80 + 172) && curr_scanline_cycles < 456 && currLY < 144) {
		// H Blank
		this->set_mode(PPU_MODE_HBLANK);
	}
	if (curr_scanline_cycles >= 456) {
		// inc Ly (next line)
		mmu.write_byte(LY, currLY + 1);
		curr_scanline_cycles = 0;
		oam_done = false;
		finished_current_line = false;
	}

	if (currLY == 144) {
		if (!vblank) {
			// v blank (set bit 0 of 0xFF0F)
			uint8_t int_flag = mmu.read_byte(IF);
			bit_set(int_flag, 0);
			mmu.write_byte(IF, int_flag);
			set_mode(PPU_MODE_VBLANK);
			vblank = true;
		}
	}

	if (currLY > 153) {
		mmu.write_byte(LY, 0);
		// reset STAT mode
		/*uint8_t stat = mmu.read_byte(STAT);
		stat &= ~(1U << 0);
		mmu.write_byte(STAT, stat);*/
		vblank = false;
	}

	curr_scanline_cycles += cycles;
}

/**
 * Convert our internal graphics representation to a simple
 * pixel array for use by SDL or whatever
 */
std::unique_ptr<uint8_t[]> PPU::render() const
{
	auto pixels = std::make_unique<uint8_t[]>(160 * 144 * 3);

	int count = 0;
	for (const auto & pixel : this->pixels) {
		for (const auto & x : pixel) {
			pixels[count] = x.r;
			pixels[count + 1] = x.g;
			pixels[count + 2] = x.b;
			count += 3;
		}
	}

	return pixels;
}

/**
 * Render the whole background tile map
 */
std::unique_ptr<uint8_t[]> PPU::render_bg()
{
	auto pixels = std::make_unique<uint8_t[]>(256 * 256 * 3);

	uint16_t tileaddr;
	uint8_t tile_num;
	uint8_t tile_low;
	uint8_t tile_high;
	int count = 0;

	// background (32 tiles wide)
	for (int y = 0; y < 256; ++y) {
		uint16_t bg_map_address = (0x9800 | ((y & 0xf8) << 2));
		for (int x = 0; x < 32; ++x) {
			tile_num = mmu.read_byte(bg_map_address);
			tileaddr = 0x8000 + (tile_num * 16);
			// get the right vertical row of the tile
			tileaddr = tileaddr + ((y % 8) * 2);
			tile_low = mmu.read_byte(tileaddr);
			tile_high = mmu.read_byte(tileaddr + 1);

			for (int bit = 7; bit >= 0; --bit) {
				uint8_t bit_low = (tile_low >> bit) & 1U;
				uint8_t bit_high = (tile_high >> bit) & 1U;
				uint8_t palette = (bit_high << 1) | bit_low;

				// combine these two bits to get our palette code
				// figure our pixel color here
				Pixel pixel = get_color(palette);
				pixels[count] = pixel.r;
				pixels[count + 1] = pixel.g;
				pixels[count + 2] = pixel.b;
				count += 3;
			}
			// next tile on this row
			bg_map_address += 1;
		}
	}

	return pixels;
}

/**
 * Render the tile data, to aid in debugging
 */
std::unique_ptr<uint8_t[]> PPU::render_tiles()
{
	auto pixels = std::make_unique<uint8_t[]>(128 * 256 * 3);

	uint8_t tile_low;
	uint8_t tile_high;
	int count = 0;

	for (int y = 0; y < 256; ++y) {
		for (int x = 0; x < 16; ++x) {
			uint16_t tileaddr = 0x8000 + (x * 16);
			tileaddr = tileaddr + ((y % 8) * 2);
			tile_low = mmu.read_byte(tileaddr);
			tile_high = mmu.read_byte(tileaddr + 1);

			for (int bit = 7; bit >= 0; --bit) {
				uint8_t bit_low = (tile_low >> bit) & 1U;
				uint8_t bit_high = (tile_high >> bit) & 1U;
				uint8_t palette = (bit_high << 1) | bit_low;

				// combine these two bits to get our palette code
				// figure our pixel color here
				Pixel pixel = get_color(palette);
				pixels[count] = pixel.r;
				pixels[count + 1] = pixel.g;
				pixels[count + 2] = pixel.b;
				count += 3;
			}
		}
	}

	return pixels;
}