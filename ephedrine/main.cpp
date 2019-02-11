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
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "gb.h"
#include "bit_utility.h"
#include "spdlog/sinks/basic_file_sink.h"

// UI
#include <stdio.h>
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include "GL/gl3w.h"

int main(int argc, char** argv) {
	auto logger = spdlog::stdout_color_mt("stdout");
	auto file_logger = spdlog::basic_logger_mt("file logger", "logs/cpu.txt");
	logger->set_level(spdlog::level::debug);
	bool quit = false;
	std::vector<uint8_t> cart;
	std::ifstream in("../gb-test-roms-master/cpu_instrs/cpu_instrs.gb", std::ios::binary);
	// test roms
	//std::ifstream in("../gb-test-roms-master/cpu_instrs/individual/01-special.gb", std::ios::binary);
	//std::ifstream in("../gb-test-roms-master/cpu_instrs/individual/02-interrupts.gb", std::ios::binary);
	//std::ifstream in("../gb-test-roms-master/cpu_instrs/individual/11-op a,(hl).gb", std::ios::binary);

	/* Passed */
	//std::ifstream in("../gb-test-roms-master/cpu_instrs/individual/03-op sp,hl.gb", std::ios::binary);
	//std::ifstream in("../gb-test-roms-master/cpu_instrs/individual/04-op r,imm.gb", std::ios::binary);
	//std::ifstream in("../gb-test-roms-master/cpu_instrs/individual/05-op rp.gb", std::ios::binary);
	//std::ifstream in("../gb-test-roms-master/cpu_instrs/individual/06-ld r,r.gb", std::ios::binary);
	//std::ifstream in("../gb-test-roms-master/cpu_instrs/individual/07-jr,jp,call,ret,rst.gb", std::ios::binary);
	//std::ifstream in("../gb-test-roms-master/cpu_instrs/individual/08-misc instrs.gb", std::ios::binary);
	//std::ifstream in("../gb-test-roms-master/cpu_instrs/individual/09-op r,r.gb", std::ios::binary);
	//std::ifstream in("../gb-test-roms-master/cpu_instrs/individual/10-bit ops.gb", std::ios::binary);

	// PPU testing
	//std::ifstream in("../opus5.gb", std::ios::binary);
	//std::ifstream in("../tellinglys.gb", std::ios::binary);
	//std::ifstream in("../tetris.gb", std::ios::binary);
	//std::ifstream in("../Dr. Mario.gb", std::ios::binary);
	//std::ifstream in("../Pokemon - Blue Version.gb", std::ios::binary);
	in.seekg(0, std::ios::end);
	auto sz = in.tellg();
	in.seekg(0, std::ios::beg);
	cart.resize(sz / sizeof(uint8_t));
	in.read((char *)cart.data(), sz);
	logger->info("cart size 0x{0:x} bytes", cart.size());
	//std::unique_ptr<Gameboy> gb{ new Gameboy{cart} };
	auto gb = std::make_unique<Gameboy>(cart);

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
	}

	SDL_Window *win = SDL_CreateWindow("Ephedrine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 160 * 2, 144 * 2, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (win == nullptr) {
		std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_Window *bgmap = SDL_CreateWindow("BG Map",512, 512, 256, 256, SDL_WINDOW_HIDDEN);
	SDL_Renderer *bgren = SDL_CreateRenderer(bgmap, -1, SDL_RENDERER_ACCELERATED);

	SDL_Window *tilewin = SDL_CreateWindow("Tiles", 256, 256, 128, 256, SDL_WINDOW_HIDDEN);
	SDL_Renderer *tileren = SDL_CreateRenderer(tilewin, -1, SDL_RENDERER_ACCELERATED);

	SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
		if (ren == nullptr) {
			std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
			return 1;
		}

	SDL_SetRenderDrawColor(ren, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(ren);
	SDL_SetRenderDrawColor(bgren, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(bgren);
	SDL_SetRenderDrawColor(tileren, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(tileren);

	SDL_Texture *tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC,160,144);
	SDL_Texture *bgtex = SDL_CreateTexture(bgren, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, 256, 256);
	SDL_Texture *tiletex = SDL_CreateTexture(tileren, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, 128, 256);

	SDL_Window *window = SDL_CreateWindow("Imgui", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	SDL_GLContext gl_context = SDL_GL_CreateContext(window);
	gl3wInit();

	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
	ImGui_ImplOpenGL3_Init("#version 130");

	using namespace std::chrono_literals;
	// A vertical refresh happens every 70224 cycles (17556 clocks) (140448 in GBC double speed mode): 59,7275 Hz
	constexpr auto tickrate = 16.7427ms;
	constexpr int max_cycles = 70224;
	int curr_screen_cycles = 0;
	SDL_Event event;
	bool running = true;
	bool bg_enabled = false;
	uint8_t mask = 0x0F;
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

			/*while (SDL_PollEvent(&event)) {
				ImGui_ImplSDL2_ProcessEvent(&event);
			}*/
			while (SDL_PollEvent(&event))
			{
				ImGui_ImplSDL2_ProcessEvent(&event);
				switch (event.type)
				{
				case SDL_QUIT || event.key.keysym.sym == SDLK_ESCAPE:
					quit = true;
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
					{
					case SDLK_r:
						running ^= running;
						logger->info("Execution: {0}", running ? "resumed" : "paused");
						break;
					case SDLK_p:
						gb->cpu.print();
						gb->ppu.print();
						//Logger::logger->debug("--------------------------------");
						break;
					case SDLK_z:
						//mask |= INPUT_A;
						bitmask_clear(mask, INPUT_A);
						break;
					case SDLK_x:
						bitmask_clear(mask, INPUT_B);
						break;
					case SDLK_DOWN:
						bitmask_clear(mask, INPUT_DOWN);
						break;
					case SDLK_UP:
						bitmask_clear(mask, INPUT_UP);
						break;
					case SDLK_LEFT:
						bitmask_clear(mask, INPUT_LEFT);
						break;
					case SDLK_RIGHT:
						bitmask_clear(mask, INPUT_RIGHT);
						break;
					case SDLK_RETURN:
						bitmask_clear(mask, INPUT_START);
						break;
					case SDLK_RSHIFT:
						bitmask_clear(mask, INPUT_SELECT);
						break;
					default:
						break;
					}
					break;
				case SDL_KEYUP:
					switch (event.key.keysym.sym)
					{
					case SDLK_z:
						//mask |= INPUT_A;
						bitmask_set(mask, INPUT_A);
						break;
					case SDLK_x:
						bitmask_set(mask, INPUT_B);
						break;
					case SDLK_DOWN:
						bitmask_set(mask, INPUT_DOWN);
						break;
					case SDLK_UP:
						bitmask_set(mask, INPUT_UP);
						break;
					case SDLK_LEFT:
						bitmask_set(mask, INPUT_LEFT);
						break;
					case SDLK_RIGHT:
						bitmask_set(mask, INPUT_RIGHT);
						break;
					case SDLK_RETURN:
						bitmask_set(mask, INPUT_START);
						break;
					case SDLK_RSHIFT:
						bitmask_set(mask, INPUT_SELECT);
						break;
					default:
						break;
					}
					break;
				default:
					break;
				}
			}

			gb->handle_input(mask);

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplSDL2_NewFrame(window);
			ImGui::NewFrame();

			ImGui::Begin("Emu");
			/*GLuint gl_tex;
			glGenTextures(1, &gl_tex);
			glBindTexture(GL_TEXTURE_2D, gl_tex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_INT_8_8_8_8, gb->ppu.render().get());
			ImGui::Image((void *)gl_tex, ImVec2(160, 144));*/
			std::stringstream str{};
			str << gb->cpu;
			ImGui::Text("%s", str.str().data());
			ImGui::Text("Joypad Register status: 0x%.2x", gb->mmu.get_register(P1));
			ImGui::Text("IF Reg: 0x%0.2x", gb->mmu.get_register(IF));
			ImGui::Text("IE Reg: 0x%0.2x", gb->mmu.get_register(IE));
			ImGui::Text("LCD Registers: LCDC: 0x%0.2x STAT: 0x%0.2x LY: %u",
				gb->mmu.get_register(LCDC), gb->mmu.get_register(STAT),
				gb->mmu.get_register(LY));
			ImGui::End();

			ImGui::Render();
			SDL_GL_MakeCurrent(window, gl_context);
			glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
			glClearColor(0, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT);
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			SDL_GL_SwapWindow(window);
		}
		// enable interrupts after instruction AFTER the EI instruction :S
		//if (gb.get_prev_opcode() == 0xFA) { // EI TODO: add support for RETI instruction?
		//	gb.enable_interrupt();
		//}
		curr_screen_cycles = 0;



		//auto pixels = gb->ppu.render().get();
		SDL_UpdateTexture(tex, nullptr, gb->ppu.render().get(), 160 * 3);
		SDL_RenderClear(ren);
		SDL_RenderCopy(ren, tex, nullptr, nullptr);
		SDL_RenderPresent(ren);

		/*auto bg = gb->ppu.render_bg();
		SDL_UpdateTexture(bgtex, nullptr, bg.get(), 256 * 3);
		SDL_RenderClear(bgren);
		SDL_RenderCopy(bgren, bgtex, nullptr, nullptr);
		SDL_RenderPresent(bgren);*/

		/*auto tiles = gb->ppu.render_tiles();
		SDL_UpdateTexture(bgtex, nullptr, tiles.get(), 128 * 3);
		SDL_RenderClear(tileren);
		SDL_RenderCopy(tileren, tiletex, nullptr, nullptr);
		SDL_RenderPresent(tileren);*/
		//SDL_Delay(1);
		auto end = std::chrono::high_resolution_clock::now();
		auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		std::ostringstream s;
		s << "Loop took " << duration.count() << " ms (" << duration_us.count() << " us)" << std::endl;
		SDL_SetWindowTitle(win, s.str().c_str());

		if (duration < tickrate) {
			// sleep for remaining time
			std::this_thread::sleep_for(tickrate - duration_us);
		}
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(window);

	SDL_DestroyWindow(win);
	SDL_DestroyRenderer(ren);
	SDL_DestroyTexture(tex);

	SDL_DestroyWindow(bgmap);
	SDL_DestroyRenderer(bgren);
	SDL_DestroyTexture(bgtex);
	SDL_Quit();

	return 0;
}
