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
	Logger::logger->info("current scanline: {0} ({3} cycles) SCX: {1:02x} SCY: {2:02x}", currLY, scx, scy, curr_scanline_cycles);
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
	/*if (((lcdc >> 7) & 1U) == 0)
		return;*/

	const uint8_t currLY = mmu.read_byte(LY);

	if (currLY == 144) {
		// v blank (set bit 0 of 0xFF0F)
		// set mode 1 in 0xFF41
		uint8_t stat = mmu.read_byte(STAT);
		mmu.write_byte(STAT, stat |= 1U << 0);
		//gb.memory[0xFF0F] |= 1U << 0;
		vblank = true;
		Logger::logger->info("V-Blank");
	}

	if (curr_scanline_cycles <= 80) {
		Logger::logger->info("OAM DMA");
		// OAM DMA XFER
		// set mode
	}
	if (curr_scanline_cycles >= 80 && curr_scanline_cycles <= (80 + 172)) {
		// pixel xfer
		if (!finished_current_line) {
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
				tile_low = mmu.read_byte(tileaddr);
				tile_high = mmu.read_byte(tileaddr + 1);
				Logger::logger->info("{0:04x} - tile# {1}({2} - {3}) @ {4:04x} - scy: {5} - ybase(LY): {6}({7})", 
					bg_map_address, tile_num, tile_low, tile_high, tileaddr, scy, ybase, currLY);
				for (int bit = 0; bit < 8; ++bit) {
					uint8_t bit_low = (tile_low >> bit) & 1U;
					uint8_t bit_high = (tile_high >> bit) & 1U;
					uint8_t palette = bit_low | (bit_high << 1);
					// figure our pixel color here
					pixels[i * bit][ybase] = palette == 0 ? 0x0 : 0xFF;
				}
				// pixels[x][ybase]
				// tile % ybase = the line of the sprite we're on
				bg_map_address += 1;
			}
			/*if (((lcdc >> 4) & 1U) == 1)
			{
				tileaddr = (1) | (())
			}*/

			// if window enabled, render
			// if sprites enabled, render
			finished_current_line = true;
			Logger::logger->info("PPU::finished_current_line={0}", finished_current_line);
		}
	}
	if (curr_scanline_cycles >= (80 + 172) && curr_scanline_cycles < 456) {
		// H Blank
		Logger::logger->info("H-Blank");
		uint8_t stat = mmu.read_byte(STAT);
		stat &= ~(1UL << 0);
		stat &= ~(1UL << 1);
		mmu.write_byte(STAT, stat);
	}
	if (curr_scanline_cycles >= 456) {
		// Line is finished!
		// render our line (background + sprites)
		//gfx[160 * currLY];
		// inc Ly (next line)
		Logger::logger->info("Finished rendering line {0} in {1} cycles", currLY, curr_scanline_cycles);
		mmu.write_byte(LY, currLY + 1);
		curr_scanline_cycles = 0;
		finished_current_line = false;
	}

	if (currLY > 153) {
		mmu.write_byte(LY, 0);
		// reset STAT mode
		uint8_t stat = mmu.read_byte(STAT);
		stat &= ~(1U << 0);
		mmu.write_byte(STAT, stat);
		vblank = false;
		Logger::logger->info("Finished V-Blank");
	}
	
	curr_scanline_cycles += cycles;
}

std::unique_ptr<std::vector<uint8_t>> PPU::refresh()
{
	std::unique_ptr<std::vector<uint8_t>> pixels;
	//bg at 0x8000
	uint8_t scx = mmu.read_byte(SCX);
	uint8_t scy = mmu.read_byte(SCY);
	// draw the whole BG map, one line at a time
	uint16_t start_addr = 0x8000;
	for (int y = 0; y < 16; ++y) {
		for (int x = 0; x < 20; ++x) {

		}
	}
	// massage our data into something easy to use with sdl
	for (int i = 0; i < 23040; ++i) {

	}
	return pixels;
}