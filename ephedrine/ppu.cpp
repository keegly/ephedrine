#include "ppu.h"
#include "gb.h"
#include "cpu.h"
#include "bit_utility.h"
#include "spdlog/spdlog.h"
#include <queue>


PPU::PPU(std::shared_ptr<MMU> m) : mmu_(m), visible_sprites_(std::vector<Sprite>())
{
	current_scanline_cycles_ = 0;
	oam_search_finished_ = false;
	finished_current_line_ = false;
	mmu_->write_byte(LCDC, 0x91);
	mmu_->write_byte(STAT, 0x85);
}

void PPU::print()
{
	uint8_t currLY = mmu_->read_byte(LY);
	uint8_t scx = mmu_->read_byte(SCX);
	uint8_t scy = mmu_->read_byte(SCY);
	//	Logger::logger->debug("current scanline: {0} ({3} cycles) SCX: {1:02x} SCY: {2:02x}", currLY, scx, scy, current_scanline_cycles_);
}

void PPU::set_mode(uint8_t mode)
{
	mmu_->set_ppu_mode(mode);
	// set STAT interrupt flag?
}

/**
 * Take a 2 bit tile code (0-3) and return the correct
 * color according to the current palette_ settings
 */
Pixel PPU::get_color(uint8_t tile) const
{
	uint8_t bgp = mmu_->read_byte(BGP);
	Pixel pixel;

	switch (tile) {
	case 0:
		// bits 0-1 of BGP
		bgp &= 3U;
		pixel = palette_[bgp];
		break;
	case 1:
		// bits 2-3
		bgp = (bgp >> 2) & 3U;
		pixel = palette_[bgp];
		break;
	case 2:
		// bits 4-5
		bgp = (bgp >> 4) & 3U;
		pixel = palette_[bgp];
		break;
	case 3:
		// bits 6-7
		bgp = (bgp >> 6) & 3U;
		pixel = palette_[bgp];
		break;
	default:
		spdlog::get("stdout")->error("Tile palette error - invalid value: {0}", tile);
		break;
	}

	pixel.palette = tile;
	pixel.sprite = false;
	return pixel;
}

Pixel PPU::get_sprite_color(uint8_t tile, bool obp_select)
{
	//uint8_t obp;
	uint8_t obp = mmu_->read_byte(OBP0);
	if (obp_select) {
		obp = mmu_->read_byte(OBP1);
	}
	Pixel pixel{};

	switch (tile) {
	case 0:
		// bits 0-1 (transparent)
		pixel.r = 255;
		pixel.g = 255;
		pixel.b = 255;
		pixel.a = 0;
		break;
	case 1:
		// bits 2-3
		obp = (obp >> 2) & 3U;
		pixel = palette_[obp];
		break;
	case 2:
		// bits 4-5
		obp = (obp >> 4) & 3U;
		pixel = palette_[obp];
		break;
	case 3:
		// bits 6-7
		obp = (obp >> 6) & 3U;
		pixel = palette_[obp];
		break;
	default:
		spdlog::get("stdout")->error("Sprite Tile palette error - invalid value: {0}", tile);
		break;
	}

	pixel.palette = tile;
	pixel.sprite = true;
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
	uint8_t lcdc = mmu_->read_byte(LCDC);
	// LCD Disabled
	if (!bit_check(lcdc, 7)) {
		mmu_->write_byte(LY, 0);
		current_scanline_cycles_ = 0;
		return;
	}

	uint8_t currLY = mmu_->read_byte(LY);
	uint8_t stat = mmu_->read_byte(STAT);

	if (current_scanline_cycles_ <= 80 && currLY < 144 && !oam_search_finished_) {
		// OAM DMA XFER
		set_mode(PPU_MODE_OAM_SEARCH);
		// reset our data from the prev line (if any)
		visible_sprites_.clear();
		// TODO: if sprites enabled?
		// loop through our (up to) 40 sprites in the OAM table
		for (int i = 0xFE00; i < 0xFE9C; i += 4U) {
			Sprite s{ s.y = mmu_->read_byte(i), s.x = mmu_->read_byte(i + 1),
				s.tile = mmu_->read_byte(i + 2), s.flags = mmu_->read_byte(i + 3),
			s.oam_addr = i };
			// sprite off screen
			if (s.y <= 0 || s.y >= 160)
				continue;
			// part of this sprite is on our line
			// sprite height ??
			// TODO - change 8 here to sprite height - 8/16
			if (s.x != 0 && ((currLY + 16 >= s.y) && (currLY + 16 < s.y + 8)) && visible_sprites_.size() < 10) {
				visible_sprites_.push_back(s);
				//spdlog::get("stdout")->debug("Visible sprite: x: {0} y: {1}, tile: {2}, flags_: {3:02X}", s.x, s.y, s.tile, s.flags_);
			}
			//spdlog::get("stdout")->debug("visible_sprites_ sz: {0}", visible_sprites_.size());
		}

		oam_search_finished_ = true;
	}
	if (current_scanline_cycles_ >= 80 && current_scanline_cycles_ <= (80 + 172) && currLY < 144 && !finished_current_line_) {
		set_mode(PPU_MODE_LCD_XFER);
		// bg pixel xfer, if bit 0 of LCDC is set (bg enable)
		if (bit_check(lcdc, 0)) {
			uint8_t scx = mmu_->read_byte(SCX);
			uint8_t scy = mmu_->read_byte(SCY);
			// what line are we on?
			uint8_t ybase = scy + currLY;
			// find current bg map position
			uint16_t bg_map_address = (0x9800 | (lcdc << 0x10) | ((ybase & 0xf8) << 2) | ((scx & 0xf8) >> 3));
			// leftmost bg address (to help with screen wrap)
			uint16_t bg_map_base = (0x9800 | (lcdc << 0x10) | ((ybase & 0xf8) << 2));
			// which tells us the current bg map tile number
			// leftmost?
			uint8_t tile_num = mmu_->read_byte(bg_map_address);
			// which we can use to grab the actual tile bytes
			// grab the first byte and discard (like real h/w?)
			uint16_t tileset;
			uint16_t tileaddr;
			if (bit_check(lcdc, 4)) {
				tileset = 0x8000;
				tileaddr = tileset + (tile_num * 16);
			}
			else {
				tileset = 0x9000;
				tileaddr = tileset + (static_cast<int8_t>(tile_num) * 16);
			}
			uint8_t tile_low = mmu_->read_byte(tileaddr);
			uint8_t tile_high = mmu_->read_byte(tileaddr + 1);
			std::queue<uint8_t> p{};
			//std::queue<PixelFIFO> pixel_fifo{};
			//std::queue<uint8_t> sprite_fifo{};
			//while (count < 160) {
			//	// fetch bg pixels_
			//	// place in queue
			//	// check if sprite at this location
			//	// push pixels_ one at a time, checking for sprite each location
			//	// also checking if window on, and if so fetching/drawing those instead
			//	// once less than 8 pxiels in fifo, fetch another bg tile and fill
			//	// do for 160 total pixels_
			//	while (pixel_fifo.size() < 8) {
			//		tile_num = mmu_->read_byte(bg_map_address);
			//		// w
			//		if (bit_check(lcdc, 4)) {
			//			tileaddr = tileset + (tile_num * 16);
			//		}
			//		else {
			//			tileaddr = tileset + (static_cast<int8_t>(tile_num) * 16);
			//		}
			//		// get the right vertical row of the tile
			//		tileaddr = tileaddr + ((ybase % 8) * 2);
			//		tile_low = mmu_->read_byte(tileaddr);
			//		tile_high = mmu_->read_byte(tileaddr + 1);

			//		for (int bit = 7; bit >= 0; --bit) {
			//			// check for sprite
			//			for (Sprite &s : visible_sprites_) {
			//				if (s.x == count + scx)
			//					// fetch and combine this sprite with teh 8 bg pixels_
			//			}
			//			uint8_t bit_low = bit_check(tile_low, bit);
			//			uint8_t bit_high = (tile_high >> bit) & 1U;
			//			uint8_t palette_ = (bit_high << 1) | bit_low;
			//			PixelFIFO pf;
			//			pf.source = PPU::PixelSource::bgp;
			//			pf.tile = palette_;
			//			pixel_fifo.push(pf);
			//			// combine these two bits to get our palette_ code
			//			// figure our pixel color here
			//			++count;
			//		}
			//		bg_map_address += 1;
			//	}
			//}
			// background (20 tiles wide)
			while (p.size() < 160) {
				tile_num = mmu_->read_byte(bg_map_address);
				// which
				if (bit_check(lcdc, 4)) {
					tileaddr = tileset + (tile_num * 16);
				}
				else {
					tileaddr = tileset + (static_cast<int8_t>(tile_num) * 16);
				}
				// get the right vertical row of the tile
				tileaddr = tileaddr + ((ybase % 8) * 2);
				tile_low = mmu_->read_byte(tileaddr);
				tile_high = mmu_->read_byte(tileaddr + 1);
				for (int bit = 7; bit >= 0; --bit) {
					uint8_t bit_low = bit_check(tile_low, bit);
					uint8_t bit_high = (tile_high >> bit) & 1U;
					uint8_t palette = (bit_high << 1) | bit_low;
					p.push(palette);

					if (p.size() == 160)
						break;
				}
				// first tile so compensate for any horizontal scrolling
				if (p.size() <= 8) {
					for (int x = 0; x < (scx % 8); ++x) {
						p.pop();
					}
				}
				bg_map_address += 1;

				if (bg_map_address > (bg_map_base + 0x1F))
					bg_map_address = bg_map_base;
			}

			// push all pixels_ to "lcd"
			for (int i = 0; i < 160; ++i) {
				Pixel pixel = get_color(p.front());
				pixels_[currLY][i] = pixel;
				p.pop();
			}

			// if window enabled, render
			if (bit_check(lcdc, 5)) {
				std::queue<uint8_t> empty{};
				std::swap(p, empty);
				uint16_t window_tile_map = 0;
				bit_check(lcdc, 6) ? window_tile_map = 0x9C00 : window_tile_map = 0x9800;
				uint8_t window_x_scroll = mmu_->read_byte(WX);
				uint8_t window_y_scroll = mmu_->read_byte(WY);
				if (currLY >= window_y_scroll) {
					uint8_t tile_num = mmu_->read_byte(window_tile_map);
					// which we can use to grab the actual tile bytes
					uint16_t tileset;
					uint16_t tileaddr;
					if (bit_check(lcdc, 4)) {
						tileset = 0x8000;
						tileaddr = tileset + (tile_num * 16);
					}
					else {
						tileset = 0x9000;
						tileaddr = tileset + (static_cast<int8_t>(tile_num) * 16);
					}
					uint8_t tile_low = mmu_->read_byte(tileaddr);
					uint8_t tile_high = mmu_->read_byte(tileaddr + 1);

					while (p.size() < (160)) {
						tile_num = mmu_->read_byte(window_tile_map);
						// which
						if (bit_check(lcdc, 4)) {
							tileaddr = tileset + (tile_num * 16);
						}
						else {
							tileaddr = tileset + (static_cast<int8_t>(tile_num) * 16);
						}
						// get the right vertical row of the tile
						tileaddr = tileaddr + (((window_y_scroll + currLY) % 8) * 2);
						tile_low = mmu_->read_byte(tileaddr);
						tile_high = mmu_->read_byte(tileaddr + 1);
						for (int bit = 7; bit >= 0; --bit) {
							uint8_t bit_low = bit_check(tile_low, bit);
							uint8_t bit_high = (tile_high >> bit) & 1U;
							uint8_t palette = (bit_high << 1) | bit_low;
							p.push(palette);

							if (p.size() == (160 - (window_x_scroll - 7))) {
								break;
							}
						}
						window_tile_map += 1;

						if (bg_map_address > (bg_map_base + 0x1F))
							bg_map_address = bg_map_base;
					}
				}

				int count = p.size();
				for (int i = 0; i < count; ++i) {
					Pixel pixel = get_color(p.front());
					int x_pos = (window_x_scroll - 7) + i;
					if (x_pos >= 0 && x_pos < 160) {
						pixels_[currLY][x_pos] = pixel;
					}
					p.pop();
				}
			}

			// if sprites enabled, render
			if (bit_check(lcdc, 1)) {
				// 8x8 or 8x16
				uint8_t height;
				uint8_t tile_low;
				uint8_t tile_high;
				bit_check(lcdc, 2) ? height = 16 : height = 8;
				for (const Sprite &s : visible_sprites_) {
					uint16_t tileaddr = 0x8000 + (s.tile * 16);
					// flip y
					if (bit_check(s.flags, 6)) {
						tileaddr += (((s.y - 16) - currLY)) * 2;
						//tileaddr += ((currLY)-(s.y - 16)) * 2;
						spdlog::get("stdout")->debug("s.y: {0}, currLY: {1}, tileaddr: {2:04x}, base tile: {3:04x}", s.y, currLY, tileaddr, 0x8000 + (s.tile * 16));
					}
					else {
						tileaddr += ((currLY)-(s.y - 16)) * 2;
					}
					// TODO: sprite flag handling
					// y flipping
					//spdlog::get("stdout")->debug("sprite tile address: {0:04x} OAM: {3:04X} LY: {1} sprite y pos: {2}", tileaddr, currLY, s.y, s.oam_addr);
					tile_low = mmu_->read_byte(tileaddr);
					tile_high = mmu_->read_byte(tileaddr + 1);
					int index = 0;
					// if bit is 1, sprite behind BG colors 1-3
					bool sprite_bg_priority = bit_check(s.flags, 7);
					// x flipping
					if (bit_check(s.flags, 5)) {
						for (int bit = 0; bit <= 7; ++bit) {
							uint8_t bit_low = bit_check(tile_low, bit);
							uint8_t bit_high = bit_check(tile_high, bit);
							uint8_t palette = (bit_high << 1) | bit_low;
							Pixel pixel = get_sprite_color(palette, bit_check(s.flags, 4));
							int x_pos = (s.x - 8) + bit;
							if (pixel.a == 0 || x_pos < 0 || x_pos >= 160) {
								continue;
							}
							if ((sprite_bg_priority && pixels_[currLY][x_pos].palette == 0) || !sprite_bg_priority) {
								pixels_[currLY][x_pos] = pixel;
							}
						}
					}
					else {
						for (int bit = 7; bit >= 0; --bit) {
							uint8_t bit_low = bit_check(tile_low, bit);
							uint8_t bit_high = bit_check(tile_high, bit);
							uint8_t palette = (bit_high << 1) | bit_low;
							Pixel pixel = get_sprite_color(palette, bit_check(s.flags, 4));
							int x_pos = (s.x - 8) + index;
							if (pixel.a == 0 || x_pos < 0 || x_pos >= 160) {
								++index;
								continue;
							}
							if ((sprite_bg_priority && pixels_[currLY][x_pos].palette == 0) || !sprite_bg_priority) {
								this->pixels_[currLY][x_pos] = pixel;
							}
							++index;
						}
					}
				}
			}
			finished_current_line_ = true;
		}
	}
	if (current_scanline_cycles_ >= (80 + 172) && current_scanline_cycles_ < 456 && currLY < 144) {
		// H Blank
		this->set_mode(PPU_MODE_HBLANK);
	}
	if (current_scanline_cycles_ >= 456) {
		// inc Ly (next line)
		mmu_->write_byte(LY, currLY + 1);
		uint8_t lyc = mmu_->read_byte(LYC);
		if (bit_check(stat, 6) && (currLY + 1) == lyc) {
			bit_set(stat, 2);
			mmu_->set_register(STAT, stat);
			// also request interrupt here?
			uint8_t int_flag = mmu_->get_register(IF);
			bit_set(int_flag, 1);
			mmu_->set_register(IF, int_flag);
		}
		else {
			bit_clear(stat, 2);
			mmu_->set_register(STAT, stat);
			uint8_t int_flag = mmu_->get_register(IF);
			bit_clear(int_flag, 1);
			mmu_->set_register(IF, int_flag);
		}
		current_scanline_cycles_ = 0;
		oam_search_finished_ = false;
		finished_current_line_ = false;
	}

	if (currLY == 144) {
		if (!vblank) {
			// v blank (set bit 0 of 0xFF0F)
			uint8_t int_flag = mmu_->read_byte(IF);
			bit_set(int_flag, 0);
			mmu_->write_byte(IF, int_flag);
			set_mode(PPU_MODE_VBLANK);
			vblank = true;
		}
	}

	if (currLY > 153) {
		mmu_->write_byte(LY, 0);
		// reset STAT mode
		/*uint8_t stat = mmu_->read_byte(STAT);
		stat &= ~(1U << 0);
		mmu_->write_byte(STAT, stat);*/
		vblank = false;
	}

	current_scanline_cycles_ += cycles;
}

/**
 * Convert our internal graphics representation to a simple
 * pixel array for use by SDL or whatever
 */
std::unique_ptr<uint8_t[]> PPU::render() const
{
	auto pixels = std::make_unique<uint8_t[]>(160 * 144 * 4);

	int count = 0;
	for (const auto & pixel : this->pixels_) {
		for (const auto & x : pixel) {
			pixels[count] = x.r;
			pixels[count + 1] = x.g;
			pixels[count + 2] = x.b;
			pixels[count + 3] = x.a;
			count += 4;
		}
	}
	return pixels;
}

/**
 * Render the whole background tile map
 */
std::unique_ptr<uint8_t[]> PPU::render_bg() const
{
	auto pixels = std::make_unique<uint8_t[]>(256 * 256 * 4);

	uint16_t tileaddr;
	uint8_t tile_num;
	uint8_t tile_low;
	uint8_t tile_high;
	int count = 0;

	// background (32 tiles wide)
	for (int y = 0; y < 256; ++y) {
		uint16_t bg_map_address = (0x9800 | ((y & 0xf8) << 2));
		for (int x = 0; x < 32; ++x) {
			tile_num = mmu_->read_byte(bg_map_address);
			uint8_t lcdc = mmu_->read_byte(LCDC);
			if (bit_check(lcdc, 4)) {
				tileaddr = 0x8000 + (tile_num * 16);
			}
			else {
				tileaddr = 0x9000 + ((int8_t)tile_num * 16);
			}
			// get the right vertical row of the tile
			tileaddr = tileaddr + ((y % 8) * 2);
			tile_low = mmu_->read_byte(tileaddr);
			tile_high = mmu_->read_byte(tileaddr + 1);

			for (int bit = 7; bit >= 0; --bit) {
				uint8_t bit_low = (tile_low >> bit) & 1U;
				uint8_t bit_high = (tile_high >> bit) & 1U;
				uint8_t palette = (bit_high << 1) | bit_low;

				// combine these two bits to get our palette_ code
				// figure our pixel color here
				Pixel pixel = get_color(palette);
				pixels[count] = pixel.r;
				pixels[count + 1] = pixel.g;
				pixels[count + 2] = pixel.b;
				pixels[count + 3] = pixel.a;
				count += 4;
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
std::unique_ptr<uint8_t[]> PPU::render_tiles() const
{
	auto pixels = std::make_unique<uint8_t[]>(128 * 256 * 4);

	uint8_t tile_low;
	uint8_t tile_high;
	uint16_t tile_row = 0;
	int count = 0;

	for (int y = 0; y < 256; ++y) {
		for (int x = 0; x < 16; ++x) {
			uint16_t tileaddr = 0x8000 + tile_row + (x * 16);
			tileaddr = tileaddr + ((y % 8) * 2);
			tile_low = mmu_->read_byte(tileaddr);
			tile_high = mmu_->read_byte(tileaddr + 1);

			for (int bit = 7; bit >= 0; --bit) {
				uint8_t bit_low = (tile_low >> bit) & 1U;
				uint8_t bit_high = (tile_high >> bit) & 1U;
				uint8_t palette = (bit_high << 1) | bit_low;

				// combine these two bits to get our palette_ code
				// figure our pixel color here
				Pixel pixel = get_color(palette);
				pixels[count] = pixel.r;
				pixels[count + 1] = pixel.g;
				pixels[count + 2] = pixel.b;
				pixels[count + 3] = pixel.a;
				count += 4;
			}
		}
		if (y % 8 == 0 && y != 0)
			tile_row += 0x0100;
	}

	return pixels;
}
