#include <iostream>
#include <array>
#include <chrono>
#include <string>
#include <sstream>
#include <thread>
//#include <SDL.h>

#include "cpu.h"
#include "mmu.h"
#include "gb.h"
#include "instructions.h"


//void update_screen(SDL_Window *win)
//{
//	SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
//	if (ren == nullptr) {
//		std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
//		return;
//	}
//	SDL_SetRenderDrawColor(ren, 256, 128, 0, SDL_ALPHA_OPAQUE);
//	//SDL_Texture *tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_STATIC, 160, 144);
//	while (true) {
//		SDL_RenderClear(ren);
//		//SDL_RenderCopy(ren, tex, nullptr, nullptr);
//		SDL_RenderPresent(ren);
//		SDL_Delay(1);
//	}
//	//SDL_DestroyTexture(tex);
//	SDL_DestroyRenderer(ren);
//}

int main(int argc, char** argv) {
	Gameboy gb = {};
	bool quit = false;
	FILE *cartridge;
	long size;
	uint8_t *buffer;
	size_t result;
	//fopen_s(&cartridge, "../06-ld r,r.gb", "rb");
	//fopen_s(&cartridge, "../cpu_instrs.gb", "rb");
	fopen_s(&cartridge, "../tetris.gb", "rb");
	fseek(cartridge, 0, SEEK_END);
	size = ftell(cartridge);
	rewind(cartridge);

	buffer = (uint8_t*)malloc(sizeof(uint8_t)*size);

	result = fread(buffer, 1, size, cartridge);
	gb.load(buffer, size);

	//auto sys = make_sdl
	/*if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
	}

	SDL_Window *win = SDL_CreateWindow("Hello World!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 512, SDL_WINDOW_SHOWN);
	if (win == nullptr) {
		std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		return 1;
	}*/
	
	//SDL_Texture *tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_STATIC,160,144);
	//SDL_SetRenderDrawColor(ren, 256, 128, 0, SDL_ALPHA_OPAQUE);
	//auto *surface = SDL_CreateRGBSurfaceFrom(&gb.memory[0x8000], 160, 144, sizeof(uint8_t), 160 * sizeof(uint8_t), )
	//std::thread render_thread(update_screen, std::ref(win));
	
	int total_cycles = 0;
	bool vblank = false;
	while (!quit) {
		auto start = std::chrono::high_resolution_clock::now();
		// don't draw every frame?
		//SDL_UpdateTexture(tex, nullptr, &gb.memory[0x8000], 10);
		// Render
		//SDL_RenderClear(ren);
		//SDL_RenderCopy(ren, tex, nullptr, nullptr);
		//SDL_RenderPresent(ren);
		//for (int i = 0; i < 0x2000; ++i) {
		//	if (i % 2)
		//		pixels[i] = 0xFF; //gb.memory[i + 0x8000];
		//}

		if (gb.cycles > 0) {
			--gb.cycles;
			continue; // still not done executing previous instruction
		}
		// One step of Fetch-Decode-Execute
		gb.step();
		// process interrupts
		//gb.handle_interrupts();
		// enable interrupts after instruction AFTER the EI instruction :S
		if (gb.get_prev_opcode() == 0xFA) { // EI TODO: add support for RETI instruction?
			gb.enable_interrupt();
		}

		// refresh LCD
		// do each line for 456 cycles
		// one LY = 144, V-blank until 153 then reset LY to 0 and repeat
		uint8_t currLY = gb.get_LY();
		if (total_cycles >= 456) {
			// inc LY
			gb.set_LY(currLY + 1);
			total_cycles = 0;
		}
		if (currLY > 143) {
			// v blank (set bit 0 of 0xFF0F)
			gb.memory[0xFF0F] |= 1U << 0;
			vblank = true;
		} else {
			// reset bit 0
			gb.memory[0xFF0F] &= ~(1U << 0);
			vblank = false;
		}
		total_cycles += gb.cycles;
		//gb.memory[0xFF40] &= 
		//SDL_Delay(1);
		auto end = std::chrono::high_resolution_clock::now();
		auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		std::ostringstream s;
		//s << "Loop took " << duration.count() << " ms (" << duration_us.count() << " us) - VBLANK: " << vblank << "CurrLY: " << currLY + 1;
		//std::cout << s.str() << std::endl;
		//SDL_SetWindowTitle(win, s.str().c_str());
	}

	/*render_thread.join();
	SDL_DestroyWindow(win);
	SDL_Quit();*/

	return 0;
}
