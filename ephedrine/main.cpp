#include <iostream>
#include <cstdint>
#include <chrono>
#include <string>
#include <sstream>
#include <SDL.h>
#include <vector>
#include <fstream>
#include <memory>

// Testing
#include "catch.hpp"
#include "logger.h"
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
	std::vector<uint8_t> cart; 
	//std::ifstream in("../06-ld r,r.gb", std::ios::binary);
	std::ifstream in("../cpu_instrs.gb", std::ios::binary);
	//std::ifstream in("../tetris.gb", std::ios::binary);
	in.seekg(0, std::ios::end);
	size_t sz = in.tellg();
	in.seekg(0, std::ios::beg);
	cart.resize(sz / sizeof(uint8_t));
	in.read((char *)cart.data(), sz);
	Logger::logger->info("cart size 0x{0:x} bytes", sz);
	std::unique_ptr<Gameboy> gb{ new Gameboy{cart} };
	// give us time to attach debugger
//	std::string s;
//	std::cin >> s;

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
	
	using namespace std::chrono_literals;
	constexpr auto tickrate = 16.66ms;
	// A vertical refresh happens every 70224 cycles (17556 clocks) (140448 in GBC double speed mode): 59,7275 Hz 
	constexpr int max_cycles = 70224;
	int curr_screen_cycles = 0;
	SDL_Event event;
	bool running = true;
	while (!quit) {
		auto start = std::chrono::high_resolution_clock::now();
		
		if (running) {
			while (curr_screen_cycles < max_cycles) {
				if (gb->cpu.cycles > 0) {
					--gb->cpu.cycles;
					continue; // still not done executing previous instruction
				}
				gb->tick(1);
				curr_screen_cycles += gb->cpu.cycles;

			}
		}
			while (SDL_PollEvent(&event) > 0)
			{
				switch (event.type)
				{
				case SDL_QUIT: 
					quit = true; 
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
					{
					case SDLK_r:
						running = !running;
						Logger::logger->info("Execution: {0}", running ? "resumed" : "paused");
						break;
					case SDLK_p:
						gb->cpu.print();
						gb->ppu.print();
						Logger::logger->info("--------------------------------");
						break;
					default:
						break;
					}
					break;
				default:
					break;
				}
			}
		// enable interrupts after instruction AFTER the EI instruction :S
		//if (gb.get_prev_opcode() == 0xFA) { // EI TODO: add support for RETI instruction?
		//	gb.enable_interrupt();
		//}
		curr_screen_cycles = 0;
		
		//SDL_UpdateTexture(tex, nullptr, gpu.render(), 160);
		
		/*auto pixels = gb->ppu.refresh()->data();
		SDL_UpdateTexture(tex, nullptr, pixels, 160);*/
		SDL_RenderClear(ren);
		SDL_RenderCopy(ren, tex, nullptr, nullptr);
		SDL_RenderPresent(ren);
		//SDL_Delay(1);
		auto end = std::chrono::high_resolution_clock::now();
		auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		std::ostringstream s;
		//s << "Loop took " << duration.count() << " ms (" << duration_us.count() << " us)" << std::endl;
		//std::cout << s.str() << std::endl;
		//SDL_SetWindowTitle(win, s.str().c_str());

		if (duration < tickrate) {
			// sleep for remaining time
			std::this_thread::sleep_for(tickrate - duration_us);
		}
	}

	//render_thread.join();
	SDL_DestroyWindow(win);
	SDL_DestroyRenderer(ren);
	SDL_Quit();

	return 0;
}
