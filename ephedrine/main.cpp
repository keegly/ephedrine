#include <iostream>
#include <cstdint>
#include <chrono>
#include <string>
#include <sstream>
#include <SDL.h>
#include <vector>
#include <fstream>
#include <memory>
#include <thread>
#include <map>
#include <filesystem>
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


void tick(std::shared_ptr<Gameboy> gb)
{
	bool quit = false;
	using namespace std::chrono_literals;
	constexpr auto tickrate = 16.7427ms;
	while (!quit) {
		auto start = std::chrono::high_resolution_clock::now();
		gb->tick(1);
		auto end = std::chrono::high_resolution_clock::now();
		auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		//std::cout << "One tick (70224) cycles completed in: " << duration_us.count() << " us.\n";

		if (duration < tickrate) {
			// sleep for remaining time
			//auto slp = tickrate - duration_us;
			//std::cout << "One tick (70224) cycles completed in: " << duration_us.count() << " us. Sleeping for " << slp.count() << " ms\n";
			std::this_thread::sleep_for(tickrate - duration_us);
		}
	}
}

int main(int argc, char** argv) {
	auto logger = spdlog::stdout_color_mt("stdout");
	auto file_logger = spdlog::basic_logger_mt("file logger", "logs/cpu.txt");
	logger->set_level(spdlog::level::debug);
	file_logger->set_level(spdlog::level::trace);
	bool quit = false;
	std::vector<uint8_t> cart;
	//auto pgm_dir = std::filesystem::current_path() + "/roms/";
	std::map<std::ifstream, std::string> roms{};
	roms.insert(std::pair<std::ifstream, std::string>(std::ifstream("../roms/Super Mario Land 2  - 6 Golden Coins.gb", std::ios::binary), "Super Mario Land 2 - 6 Golden Coins"));
	//std::ifstream in("../gb-test-roms-master/cpu_instrs/cpu_instrs.gb", std::ios::binary);
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
	std::ifstream in("../roms/Super Mario Land 2 - 6 Golden Coins.gb", std::ios::binary);
	//std::ifstream in("../Super_Mario_Land.gb", std::ios::binary);
	//std::ifstream in("../Pokemon - Blue Version.gb", std::ios::binary);
	//std::ifstream in("../Legend_of_Zelda,_The_-_Link's_Awakening.gb", std::ios::binary);
	//std::ifstream in("../Tennis.gb", std::ios::binary);
	//std::ifstream in("../James Bond.gb", std::ios::binary);
	//std::ifstream in("../Donkey Kong Land.gb", std::ios::binary);
	in.seekg(0, std::ios::end);
	auto sz = in.tellg();
	assert(sz != -1);
	in.seekg(0, std::ios::beg);
	cart.resize(static_cast<size_t>(sz) / sizeof(uint8_t));
	in.read((char *)cart.data(), sz);
	logger->info("cart size 0x{0:x} bytes", cart.size());
	//std::unique_ptr<Gameboy> gb{ new Gameboy{cart} };
	//auto gb{ std::make_unique<Gameboy>(cart, "Super Mario Land 2 - 6 Golden Coins") };
	auto gb{ std::make_unique<Gameboy>() };
	//auto gb = std::make_shared<Gameboy>(cart);

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
	}

	SDL_Window *win = SDL_CreateWindow("Ephedrine", 300, 200, 160 * 2, 144 * 2, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	SDL_Window *bg_map_win = SDL_CreateWindow("BG Map", 650, 200, 256, 256, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	SDL_Window *tile_map_win = SDL_CreateWindow("Tile Map", 650, 500, 128 * 2, 256 * 2, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (win == nullptr || bg_map_win == nullptr || tile_map_win == nullptr) {
		std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	SDL_Renderer *bg_map_ren = SDL_CreateRenderer(bg_map_win, -1, SDL_RENDERER_ACCELERATED);
	SDL_Renderer *tile_map_ren = SDL_CreateRenderer(tile_map_win, -1, SDL_RENDERER_ACCELERATED);
	if (ren == nullptr || bg_map_ren == nullptr || tile_map_ren == nullptr) {
		std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_SetRenderDrawColor(ren, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(ren);

	SDL_Texture *tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, 160, 144);
	SDL_Texture *bg_map_tex = SDL_CreateTexture(bg_map_ren, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, 256, 256);
	SDL_Texture *tile_map_tex = SDL_CreateTexture(tile_map_ren, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, 128, 256);

	SDL_Window *window = SDL_CreateWindow("Imgui", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	SDL_GLContext gl_context = SDL_GL_CreateContext(window);
	gl3wInit();

	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
	ImGui_ImplOpenGL3_Init("#version 410 core");

	//std::thread gb_thread{ tick, std::ref(gb) };

	using namespace std::chrono_literals;
	// A vertical refresh happens every 70224 cycles (17556 clocks) (140448 in GBC double speed mode): 59,7275 Hz
	constexpr auto tickrate = 16.7427ms;
	SDL_Event event;
	int cycles = 0;
	bool running = false;
	bool framelimit = true;
	std::array<uint8_t, 2> joypad = { 0xf, 0xf };
	Registers reg_state;
	Flags flag_state;
	while (!quit) {
		auto start = std::chrono::high_resolution_clock::now();
		if (running) {
				cycles += gb->tick(gb->max_cycles_per_vertical_refresh); // one full screen refresh worth of cycles
		}

		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL2_ProcessEvent(&event);
			switch (event.type)
			{
			case SDL_QUIT:
				quit = true;
				logger->info("Quitting");
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
				case SDLK_ESCAPE:
					quit = true;
					break;
				case SDLK_F1:
					gb->save_state();
					break;
				case SDLK_F3:
					gb->load_state();
					break;
				case SDLK_r:
					running ^= running;
					logger->info("Execution: {0}", running ? "resumed" : "paused");
					break;
				case SDLK_p:
					gb->cpu->print();
					gb->ppu->print();
					//Logger::logger->debug("--------------------------------");
					break;
				case SDLK_f:
					framelimit = !framelimit;
					logger->info("Frame Limiter: {0}", framelimit);
					break;
				case SDLK_z:
					//mask |= INPUT_A;
					bitmask_clear(joypad[0], INPUT_A);
					break;
				case SDLK_x:
					bitmask_clear(joypad[0], INPUT_B);
					break;
				case SDLK_DOWN:
					bitmask_clear(joypad[1], INPUT_DOWN);
					break;
				case SDLK_UP:
					bitmask_clear(joypad[1], INPUT_UP);
					break;
				case SDLK_LEFT:
					bitmask_clear(joypad[1], INPUT_LEFT);
					break;
				case SDLK_RIGHT:
					bitmask_clear(joypad[1], INPUT_RIGHT);
					break;
				case SDLK_RETURN:
					bitmask_clear(joypad[0], INPUT_START);
					break;
				case SDLK_RSHIFT:
					bitmask_clear(joypad[0], INPUT_SELECT);
					break;
				default:
					break;
				}
				break;
			case SDL_KEYUP:
				switch (event.key.keysym.sym)
				{
				case SDLK_z:
					bitmask_set(joypad[0], INPUT_A);
					break;
				case SDLK_x:
					bitmask_set(joypad[0], INPUT_B);
					break;
				case SDLK_DOWN:
					bitmask_set(joypad[1], INPUT_DOWN);
					break;
				case SDLK_UP:
					bitmask_set(joypad[1], INPUT_UP);
					break;
				case SDLK_LEFT:
					bitmask_set(joypad[1], INPUT_LEFT);
					break;
				case SDLK_RIGHT:
					bitmask_set(joypad[1], INPUT_RIGHT);
					break;
				case SDLK_RETURN:
					bitmask_set(joypad[0], INPUT_START);
					break;
				case SDLK_RSHIFT:
					bitmask_set(joypad[0], INPUT_SELECT);
					break;
				default:
					break;
				}
				break;
			default:
				break;
			}
		}

		gb->handle_input(joypad);
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(window);
		ImGui::NewFrame();

		ImGui::Begin("Debug");
		auto sprites = gb->ppu->get_visible_sprites();
		reg_state = gb->cpu->get_registers();
		flag_state = gb->cpu->get_flags();
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Columns(2, "register_columns", true);
		ImGui::Separator();
		ImGui::Text("AF= 0x%0.4X", reg_state.af);
		ImGui::Text("BC= 0x%0.4X", reg_state.bc);
		ImGui::Text("DE= 0x%0.4X", reg_state.de);
		ImGui::Text("HL= 0x%0.4X", reg_state.hl);
		ImGui::Text("PC= 0x%0.4X", gb->cpu->get_pc());
		ImGui::Text("SP= 0x%0.4X", gb->cpu->get_sp());
		ImGui::Checkbox("z", &flag_state.z);
		ImGui::SameLine();
		ImGui::Checkbox("n", &flag_state.n);
		ImGui::SameLine();
		ImGui::Checkbox("h", &flag_state.h);
		ImGui::SameLine();
		ImGui::Checkbox("c", &flag_state.c);
		ImGui::NextColumn();
		ImGui::Text("lcdc= 0x%0.2X", gb->mmu->get_register(LCDC));
		ImGui::Text("stat= 0x%0.2X", gb->mmu->get_register(STAT));
		ImGui::Text("ly= 0x%0.2X", gb->mmu->get_register(LY));
		ImGui::Text("IE: 0x%0.2x", gb->mmu->get_register(IE));
		ImGui::Text("IF: 0x%0.2x", gb->mmu->get_register(IF));
		ImGui::Checkbox("running", &running);
		ImGui::SameLine();
		if (ImGui::Button("Step"))
			gb->tick(1);
		if (ImGui::Button("Step 1/4 frame"))
			gb->tick(gb->max_cycles_per_vertical_refresh/4);
		if (ImGui::Button("Step 1 frame"))
			gb->tick(gb->max_cycles_per_vertical_refresh);
		if (ImGui::Button("Step until Z")) {
			bool z = false;
			while (!z) {
				gb->tick(1);
				flag_state = gb->cpu->get_flags();
				z = flag_state.z;
			}
		}
		// Display our list of games to choose from
		for (auto const&[key, val] : roms) {
			
		}

		ImGui::End();
		ImGui::Begin("Sprites");
		for (Sprite &s : *sprites.get()) {
			ImGui::Text("Y: 0x%0.2X X: 0x%0.2X Tile: 0x%0.2X Flags: 0x%0.2X", s.y, s.x, s.tile, s.flags);
		}
		ImGui::End();
		ImGui::Render();
		SDL_GL_MakeCurrent(window, gl_context);
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(window);
		// enable interrupts after instruction AFTER the EI instruction :S
		//if (gb.get_prev_opcode() == 0xFA) { // EI TODO: add support for RETI instruction?
		//	gb.enable_interrupt();
		//}

		//auto pixels_ = gb->ppu.render().get();
		SDL_UpdateTexture(tex, nullptr, gb->ppu->render().get(), 160 * 4);
		SDL_RenderClear(ren);
		SDL_RenderCopy(ren, tex, nullptr, nullptr);
		SDL_RenderPresent(ren);

		SDL_UpdateTexture(bg_map_tex, nullptr, gb->ppu->render_bg().get(), 256 * 4);
		SDL_RenderClear(bg_map_ren);
		SDL_RenderCopy(bg_map_ren, bg_map_tex, nullptr, nullptr);
		SDL_RenderPresent(bg_map_ren);

		SDL_UpdateTexture(tile_map_tex, nullptr, gb->ppu->render_tiles().get(), 128 * 4);
		SDL_RenderClear(tile_map_ren);
		SDL_RenderCopy(tile_map_ren, tile_map_tex, nullptr, nullptr);
		SDL_RenderPresent(tile_map_ren);
		auto end = std::chrono::high_resolution_clock::now();
		auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		std::ostringstream s;
		s << "Loop took " << duration.count() << " ms (" << duration_us.count() << " us)" << std::endl;
		SDL_SetWindowTitle(win, s.str().c_str());

		if (framelimit && duration < tickrate) {
			// sleep for remaining time
			std::this_thread::sleep_for(tickrate - duration_us);
		}
	}

//	gb_thread.join();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(window);

	SDL_DestroyWindow(win);
	SDL_DestroyRenderer(ren);
	SDL_DestroyTexture(tex);

	SDL_Quit();

	return 0;
}
