#include "ppu.h"
#include "logger.h"


PPU::PPU(MMU& m) : mmu(m)
{
	curr_scanline_cycles = 0;
}

void PPU::print()
{
	uint8_t currLY = mmu.read_byte(LY);
	uint8_t scx = mmu.read_byte(SCX);
	uint8_t scy = mmu.read_byte(SCY);
	Logger::logger->debug("current scanline: {0} ({3} cycles) SCX: {1:02x} SCY: {2:02x}", currLY, scx, scy, curr_scanline_cycles);
}

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
		break;
	}

	return pixel;
}

void PPU::update(int cycles)
{
	// refresh LCD
	// do each line for 456 cycles
	// one LY = 144, V-blank until 153 then reset LY to 0 and repeat
	// if disabled, LY should stay at 0
	// OAM search - 20 clks - 80 machine cycles??
	// Pixel xfer - 43+ clks - 172 cycles
	// H-Blank - 51- clks - 204 cycles
	// Total - 456 cycles.
	// Do nothing if LCD disabled
	const uint8_t lcdc = mmu.read_byte(LCDC);
	if (((lcdc >> 7) & 1U) == 0)
		return;

	const uint8_t currLY = mmu.read_byte(LY);

	if (curr_scanline_cycles <= 80 && currLY < 144 && !oam_done) {
		//Logger::logger->debug("OAM DMA");
		// OAM DMA XFER
		// set mode
		oam_done = true;
	}
	if (curr_scanline_cycles >= 80 && curr_scanline_cycles <= (80 + 172) && currLY < 144 && !finished_current_line) {
		// bg pixel xfer, if bit 0 of LCDC is set (bg enable)
		if (((lcdc >> 0) & 1U) == 1) {
			const uint8_t scx = mmu.read_byte(SCX);
			const uint8_t scy = mmu.read_byte(SCY);
			// what line are we on?
			uint8_t ybase = scy + currLY;
			// find current bg map position
			uint16_t bg_map_address = (0x9800 | (lcdc << 10) | ((ybase & 0xf8) << 2) | ((scx & 0xf8) >> 3));
			// which tells us the current bg map tile number
			// leftmost?
			uint8_t tile_num = mmu.read_byte(bg_map_address);
			// which we can use to grab the actual tile bytes
			// todo: use the register instead  of hardcoding 8000
			// grab and discard the first byte becuase that's what hardware does?
			uint16_t tileaddr = 0x8000 + (tile_num * 16);
			uint8_t tile_low = mmu.read_byte(tileaddr);
			uint8_t tile_high = mmu.read_byte(tileaddr + 1);
			// background (20 tiles wide)
			for (int i = 0; i < 20; ++i) {
				tile_num = mmu.read_byte(bg_map_address);
				tileaddr = 0x8000 + (tile_num * 16);
				// get the right vertical row of the tile
				//tileaddr = tileaddr + ((ybase % 8) * 2);
				tile_low = mmu.read_byte(tileaddr);
				tile_high = mmu.read_byte(tileaddr + 1);
				//Logger::logger->debug("{0:04x} - tile# {1}({2} - {3}) @ {4:04x} - scy: {5} - ybase(LY): {6}({7})",
				//	bg_map_address, tile_num, tile_low, tile_high, tileaddr, scy, ybase, currLY);
				for (int bit = 7; bit >= 0; --bit) {
					uint8_t bit_low = (tile_low >> bit) & 1U;
					uint8_t bit_high = (tile_high >> bit) & 1U;
					uint8_t palette = (bit_high << 1) | bit_low;

					// combine these two bits to get our palette code
					// figure our pixel color here
					Pixel pixel = get_color(palette);
					pixels[i*bit][currLY].r = pixel.r;
					pixels[i*bit][currLY].g = pixel.g;
					pixels[i*bit][currLY].b = pixel.b;
				}
				//bg_map_address += 1;
			}

			// if window enabled, render
			// if sprites enabled, render
			finished_current_line = true;
			//Logger::logger->debug("PPU::finished_current_line={0}", finished_current_line);
		}
	}
	if (curr_scanline_cycles >= (80 + 172) && curr_scanline_cycles < 456 && currLY < 144) {
		// H Blank
		//Logger::logger->debug("H-Blank");
		uint8_t stat = mmu.read_byte(STAT);
		stat &= ~(1UL << 0);
		stat &= ~(1UL << 1);
		mmu.write_byte(STAT, stat);
	}
	if (curr_scanline_cycles >= 456) {
		// inc Ly (next line)
		mmu.write_byte(LY, currLY + 1);
		curr_scanline_cycles = 0;
		oam_done = false;
		finished_current_line = false;
	}

	if (currLY == 144) {
		// v blank (set bit 0 of 0xFF0F)
		// set mode 1 in 0xFF41
		uint8_t stat = mmu.read_byte(STAT);
		mmu.write_byte(STAT, stat |= 1U << 0);
		//gb.memory[0xFF0F] |= 1U << 0;
		vblank = true;
		//Logger::logger->debug("V-Blank");
	}

	if (currLY > 153) {
		mmu.write_byte(LY, 0);
		// reset STAT mode
		uint8_t stat = mmu.read_byte(STAT);
		stat &= ~(1U << 0);
		mmu.write_byte(STAT, stat);
		vblank = false;
		//Logger::logger->debug("Finished V-Blank");
	}

	curr_scanline_cycles += cycles;
}

std::unique_ptr<uint8_t[]> PPU::refresh()
{
	auto pixels = std::make_unique<uint8_t[]>(160 * 144 * 3);

	//bg at 0x8000
	//uint8_t scx = mmu.read_byte(SCX);
	//uint8_t scy = mmu.read_byte(SCY);
	// draw the whole BG map, one line at a time
	//uint16_t start_addr = 0x8000;
	int count = 0;
	for (int y = 0; y < 144; ++y) {
		for (int x = 0; x < 160; ++x) {
			pixels[count] = this->pixels[x][y].r;
			pixels[count + 1] = this->pixels[x][y].g;
			pixels[count + 2] = this->pixels[x][y].b;
			count += 3;
		}
	}

	return pixels;
}

std::unique_ptr<uint8_t[]> PPU::refresh_bg()
{
	//uint8_t pixels[256 * 256 * 3]{};
	auto pixels = std::make_unique<uint8_t[]>(256 * 256 * 3);
	// find current bg map position
	//uint16_t bg_map_address = 0x9800;
	// which tells us the current bg map tile number
	// leftmost?

	uint8_t tile_num;
	uint16_t tileaddr;
	uint8_t tile_low;
	uint8_t tile_high;
	int count = 0;
	int line_count = 0;

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
			//Logger::logger->debug("{0:04x} - tile# {1}({2} - {3}) @ {4:04x} - scy: {5} - ybase(LY): {6}({7})",
			//	bg_map_address, tile_num, tile_low, tile_high, tileaddr, scy, ybase, currLY);
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
	//return std::unique_ptr<uint8_t[]>(pixels);
}