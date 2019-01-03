#include <iostream>
#include <cstdint>
#include <chrono>
#include <memory>
#include <string>
#include <sstream>
#include <thread>
#include <SDL.h>
#include <vector>
#include <fstream>

#include "gb.h"
//#include "instructions.h"


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
	bool quit = false;
	// C Style
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

	// C++
	std::vector<uint8_t> cart;
	//std::ifstream in("../06-ld r,r.gb", std::ios::binary);
	//std::ifstream in("../cpu_instrs.gb", std::ios::binary);
	std::ifstream in("../tetris.gb", std::ios::binary);
	in.seekg(0, std::ios::end);
	size_t sz = in.tellg();
	in.seekg(0, std::ios::beg);
	cart.resize(sz / sizeof(uint8_t));
	in.read((char *)cart.data(), sz);

	Gameboy gb{cart};
	

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
	}

	SDL_Window *win = SDL_CreateWindow("Ephedrine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 160, 144, SDL_WINDOW_SHOWN);
	if (win == nullptr) {
		std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
		if (ren == nullptr) {
			std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
			return 1;
		}
	//SDL_SetRenderDrawColor(ren, 256, 128, 0, SDL_ALPHA_OPAQUE);
	SDL_SetRenderDrawColor(ren, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(ren);
	
	SDL_Texture *tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_STATIC,160,144);
	//SDL_SetRenderDrawColor(ren, 256, 128, 0, SDL_ALPHA_OPAQUE);
	//auto *surface = SDL_CreateRGBSurfaceFrom(&gb.memory[0x8000], 160, 144, sizeof(uint8_t), 160 * sizeof(uint8_t), )
	//std::thread render_thread(update_screen, std::ref(win));
	
	const int MAX_CYCLES = 69905;
	int cycles = 0;
	SDL_Event event;
	while (!quit) {
		auto start = std::chrono::high_resolution_clock::now();
		
		while (cycles < MAX_CYCLES) {
			if (gb.cpu.cycles > 0) {
				--gb.cpu.cycles;
				continue; // still not done executing previous instruction
			}
			// One step of Fetch-Decode-Execute
			gb.cpu.step();
			// process interrupts
			// takes 5 machine cycles (20 CLK)
			//gb.handle_interrupts();
			cycles += gb.cpu.cycles;
			gb.ppu.update(gb.cpu.cycles);

			while (SDL_PollEvent(&event) > 0)
			{
				switch (event.type)
				{
				case SDL_QUIT: 
					quit = true; 
					break;
				default:
					break;
				}
			}
		}
		// enable interrupts after instruction AFTER the EI instruction :S
		//if (gb.get_prev_opcode() == 0xFA) { // EI TODO: add support for RETI instruction?
		//	gb.enable_interrupt();
		//}
		cycles = 0;
		
		//SDL_UpdateTexture(tex, nullptr, gpu.render(), 160);
		
		auto pixels = gb.ppu.render();
		SDL_UpdateTexture(tex, nullptr, pixels, 160);
		SDL_RenderClear(ren);
		SDL_RenderCopy(ren, tex, nullptr, nullptr);
		SDL_RenderPresent(ren);
		//SDL_Delay(1);
		auto end = std::chrono::high_resolution_clock::now();
		auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		//std::ostringstream s;
		//s << "Loop took " << duration.count() << " ms (" << duration_us.count() << " us) - VBLANK: " << vblank << "CurrLY: " << currLY + 1;
		//std::cout << s.str() << std::endl;
		//SDL_SetWindowTitle(win, s.str().c_str());
	}

	//render_thread.join();
	SDL_DestroyWindow(win);
	SDL_DestroyRenderer(ren);
	SDL_Quit();

	return 0;
}
