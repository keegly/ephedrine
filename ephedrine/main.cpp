#include <SDL.h>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
// Testing
#include "bit_utility.h"
#include "catch.hpp"
#include "gb.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

// UI
#include <cstdio>
#include "GL/gl3w.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"

// Load a game into a vector, to be easily used by our gb
std::unique_ptr<std::vector<uint8_t>> Load(std::ifstream &rom) {
  std::vector<uint8_t> cart;
  using namespace std::chrono_literals;
  constexpr auto tickrate = 16.7427ms;
  auto start = std::chrono::high_resolution_clock::now();
  rom.seekg(0, std::ios::end);
  const auto sz = rom.tellg();
  assert(sz != -1);
  rom.seekg(0, std::ios::beg);
  cart.resize(static_cast<size_t>(sz) / sizeof(uint8_t));
  rom.read((char *)cart.data(), sz);
  auto end = std::chrono::high_resolution_clock::now();
  auto duration_us =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  spdlog::get("stdout")->info("Loading duration: {0} us", duration_us.count());
  spdlog::get("stdout")->info("cart size 0x{0:x} bytes", cart.size());
  return std::make_unique<std::vector<uint8_t>>(cart);
}

/* Various ImGui "modules" here, broken out in to their own individual functions
 */
// CPU registers and individual stepping options
void ShowCPUDebug(Gameboy &gb, bool &running) {
  Registers reg_state = gb.cpu.GetRegisters();
  Flags flag_state = gb.cpu.GetFlags();
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
              1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
  ImGui::Columns(2, "register_columns", true);
  ImGui::Separator();
  ImGui::Text("AF= 0x%0.4X", reg_state.af);
  ImGui::Text("BC= 0x%0.4X", reg_state.bc);
  ImGui::Text("DE= 0x%0.4X", reg_state.de);
  ImGui::Text("HL= 0x%0.4X", reg_state.hl);
  ImGui::Text("PC= 0x%0.4X", gb.cpu.GetPC());
  ImGui::Text("SP= 0x%0.4X", gb.cpu.GetSP());
  ImGui::Checkbox("z", &flag_state.z);
  ImGui::SameLine();
  ImGui::Checkbox("n", &flag_state.n);
  ImGui::SameLine();
  ImGui::Checkbox("h", &flag_state.h);
  ImGui::SameLine();
  ImGui::Checkbox("c", &flag_state.c);
  ImGui::NextColumn();
  ImGui::Text("IE: 0x%0.2x", gb.mmu.GetRegister(IE));
  ImGui::Text("IF: 0x%0.2x", gb.mmu.GetRegister(IF));
  ImGui::Checkbox("running", &running);
  ImGui::SameLine();
  if (ImGui::Button("Step")) gb.Tick(1);
  if (ImGui::Button("Step 1 frame"))
    gb.Tick(gb.max_cycles_per_vertical_refresh);
  if (ImGui::Button("Step until Z")) {
    bool z = false;
    while (!z) {
      gb.Tick(1);
      z = gb.cpu.GetFlags().z;
    }
  }
  //  ImGui::EndColumns();
  ImGui::Columns(1);
  // list box printing the last 100 (?) executed instructions
  auto executed_instructions = gb.cpu.GetExecutedInstructions();
  for (const auto &instruction : executed_instructions) {
    if (instruction.operand.has_value()) {
      ImGui::Text("%s %hX", instruction.name.data(),
                  instruction.operand.value());
    } else {
      ImGui::Text("%s", instruction.name.data());
    }
  }
}

// PPU Registers and relevant
void ShowPPUDebug(Gameboy &gb, bool &framelimit, bool &ui_draw_bg_map,
                  bool &ui_draw_tile_map) {
  ImGui::Checkbox("Background Map", &ui_draw_bg_map);
  ImGui::Checkbox("Tile Map", &ui_draw_tile_map);
  ImGui::Checkbox("Framelimiter", &framelimit);
  ImGui::Text("Mode: %0.2x", gb.mmu.ReadByte(STAT));
  ImGui::Text("Vblank: %d", gb.ppu.IsVBlank());
  ImGui::Text("lcdc= 0x%0.2X", gb.mmu.GetRegister(LCDC));
  ImGui::Text("stat= 0x%0.2X", gb.mmu.GetRegister(STAT));
  ImGui::Text("ly= 0x%0.2X", gb.mmu.GetRegister(LY));
}

// Sprite viewer
void ShowSpriteDebug(Gameboy &gb) {
  auto sprites = gb.ppu.GetAllSprites();
  for (Sprite &s : *sprites) {
    ImGui::Text(
        "Y: 0x%0.2X X: 0x%0.2X Tile: 0x%0.2X Flags: 0x%0.2X OAM Addr: "
        "0x%0.4X",
        s.y, s.x, s.tile, s.flags, s.oam_addr);
    if (ImGui::IsItemHovered()) {
      // TODO: make background lighter colored
      ImGui::BeginTooltip();
      auto sprite_pixels = *gb.ppu.RenderSprite(s);
      GLuint sprite_tex;
      glGenTextures(1, &sprite_tex);
      glBindTexture(GL_TEXTURE_2D, sprite_tex);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 8, 8, 0, GL_RGBA,
                   GL_UNSIGNED_BYTE, sprite_pixels.data());
      ImGui::Image((void *)sprite_tex, ImVec2(64, 64));
      ImGui::EndTooltip();
    }
    bool flip_x = bit_check(s.flags, 5);
    bool flip_y = bit_check(s.flags, 6);
    ImGui::Checkbox("Flip X", &flip_x);
    ImGui::SameLine();
    ImGui::Checkbox("Flip Y", &flip_y);
  }
}

// MMU Memory inspector
void ShowMMUDebug(Gameboy &gb) {
  ImGuiTabBarFlags tbf = ImGuiTabBarFlags_None;
  if (ImGui::BeginTabBar("Mem Inspector", tbf)) {
    if (ImGui::BeginTabItem("Memory")) {
      static ImU32 start_address = 0;
      static ImU32 end_address = 0xff;
      static bool follow_pc = false;
      static std::vector<uint8_t> memory{};
      static std::vector<uint8_t> prev_memory{};
      ImGui::PushItemWidth(50);
      ImGui::InputScalar("Start Address", ImGuiDataType_U32, &start_address,
                         nullptr, nullptr, "%04X",
                         ImGuiInputTextFlags_CharsHexadecimal);
      ImGui::SameLine();
      ImGui::InputScalar("End Address", ImGuiDataType_U32, &end_address,
                         nullptr, nullptr, "%04X",
                         ImGuiInputTextFlags_CharsHexadecimal);
      ImGui::SameLine();
      ImGui::Checkbox("Follow PC", &follow_pc);
      if (follow_pc) {
        start_address = gb.cpu.GetPC();
        start_address + 0xFF > 0xFFFF ? end_address = 0xFFFF
                                      : end_address = start_address + 0xFF;
      }
      ImGui::PopItemWidth();
      if (end_address >= start_address && end_address <= 0xFFFF) {
        prev_memory = memory;
        memory = *gb.mmu.DebugShowMemory(start_address, end_address);
      }
      ImGui::Columns(17, nullptr, false);
      for (unsigned int i = 0; i < memory.size(); ++i) {
        if (i % 0x10 == 0) {
          ImGui::TextColored(ImVec4(255, 255, 255, 255),
                             "%0.4X:", i + start_address);
          ImGui::NextColumn();
        }
        /*   char label[12];
           sprintf_s(label, "%0.2X", memory[i]);*/
        ImVec4 color;
        if (prev_memory.size() > 0 && prev_memory.size() == memory.size() &&
            prev_memory[i] != memory[i]) {
          // red
          color = ImVec4(255, 0, 0, 255);
        } else {
          color = ImVec4(255, 255, 255, 255);
        }
        ImGui::TextColored(color, "%0.2X", memory[i]);
        ImGui::SetColumnWidth(ImGui::GetColumnIndex(), 25);
        // if (ImGui::Selectable(label)) {
        //  // TODO: pop up dialog to edit byte in place?
        //}
        if (ImGui::IsItemHovered()) {
          ImGui::BeginTooltip();
          ImGui::Text("%0.4X", i + start_address);
          ImGui::EndTooltip();
        }
        ImGui::NextColumn();
      }
      ImGui::EndTabItem();
    }
    // auto ram_banks = gb->mmu.DebugRamBanks();
    // auto bank = ram_banks[0];
    // if (ImGui::BeginTabItem("Ram Bank")) {
    //  for (unsigned int i = 0; i < bank.size(); ++i) {
    //    if (i % 0x10 == 0) {
    //      ImGui::TextColored(ImVec4(255, 255, 255, 255), "%0.4X:", i);
    //      ImGui::NextColumn();
    //    }
    //    /*   char label[12];
    //       sprintf_s(label, "%0.2X", memory[i]);*/
    //    ImVec4 color;
    //    color = ImVec4(255, 255, 255, 255);
    //    ImGui::TextColored(color, "%0.2X", bank[i]);
    //    ImGui::SetColumnWidth(ImGui::GetColumnIndex(), 25);
    //    // if (ImGui::Selectable(label)) {
    //    //  // TODO: pop up dialog to edit byte in place?
    //    //}
    //    if (ImGui::IsItemHovered()) {
    //      ImGui::BeginTooltip();
    //      ImGui::Text("%0.4X", i);
    //      ImGui::EndTooltip();
    //    }
    //    ImGui::NextColumn();
    //  }
    //  ImGui::EndTabItem();
    //}
    ImGui::EndTabBar();
  }
}

int main(int argc, char **argv) {
  auto logger = spdlog::stdout_color_mt("stdout");
  auto file_logger = spdlog::basic_logger_mt("file logger", "F:/logs/cpu.txt");
  logger->set_level(spdlog::level::debug);
  file_logger->set_level(spdlog::level::trace);
  bool quit = false;
  std::vector<uint8_t> cart;
  auto gb{std::make_unique<Gameboy>()};

  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
  }

  SDL_Window *win = SDL_CreateWindow("Ephedrine", 300, 200, 160 * 2, 144 * 2,
                                     SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
  SDL_Window *bg_map_win = SDL_CreateWindow(
      "BG Map", 650, 200, 256, 256, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
  SDL_Window *tile_map_win =
      SDL_CreateWindow("Tile Map", 650, 500, 128 * 2, 192 * 2,
                       SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
  if (win == nullptr || bg_map_win == nullptr || tile_map_win == nullptr) {
    std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
    return 1;
  }

  SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
  SDL_Renderer *bg_map_ren =
      SDL_CreateRenderer(bg_map_win, -1, SDL_RENDERER_ACCELERATED);
  SDL_Renderer *tile_map_ren =
      SDL_CreateRenderer(tile_map_win, -1, SDL_RENDERER_ACCELERATED);
  if (ren == nullptr || bg_map_ren == nullptr || tile_map_ren == nullptr) {
    std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
    return 1;
  }

  SDL_SetRenderDrawColor(ren, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(ren);

  SDL_Texture *tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA32,
                                       SDL_TEXTUREACCESS_STATIC, 160, 144);
  SDL_Texture *bg_map_tex = SDL_CreateTexture(
      bg_map_ren, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, 256, 256);
  SDL_Texture *tile_map_tex = SDL_CreateTexture(
      tile_map_ren, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, 128, 192);

  SDL_Window *window =
      SDL_CreateWindow("Imgui", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       1024, 768, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  SDL_GLContext gl_context = SDL_GL_CreateContext(window);
  gl3wInit();

  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
  ImGui_ImplOpenGL3_Init("#version 410 core");

  // std::thread gb_thread{ tick, std::ref(gb) };

  using namespace std::chrono_literals;
  // A vertical refresh happens every 70224 cycles (17556 clocks) (140448 in GBC
  // double speed mode): 59,7275 Hz
  constexpr auto tickrate = 16.7427ms;
  SDL_Event event;
  int cycles = 0;
  bool running = false;
  // GUI Checkboxes
  bool framelimit = false;
  bool ui_draw_bg_map = true;
  bool ui_draw_tile_map = true;
  std::array<uint8_t, 2> joypad = {0xf, 0xf};
  auto roms_dir = std::filesystem::current_path().parent_path() / "roms";
  while (!quit) {
    auto start = std::chrono::high_resolution_clock::now();
    cycles = 0;
    if (running) {
      cycles += gb->Tick(
          gb->max_cycles_per_vertical_refresh);  // one full screen refresh
                                                 // worth of cycles
    }
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL2_ProcessEvent(&event);
      switch (event.type) {
        case SDL_QUIT:
          quit = true;
          logger->info("Quitting");
          break;
        case SDL_KEYDOWN:
          switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
              quit = true;
              break;
            case SDLK_F1:
              gb->SaveState();
              logger->info("Saving state");
              break;
            case SDLK_F3:
              logger->info("Loading state");
              gb->LoadState();
              break;
            case SDLK_z:
              bitmask_clear(joypad[0], INPUT_B);
              break;
            case SDLK_x:
              bitmask_clear(joypad[0], INPUT_A);
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
          switch (event.key.keysym.sym) {
            case SDLK_z:
              bitmask_set(joypad[0], INPUT_B);
              break;
            case SDLK_x:
              bitmask_set(joypad[0], INPUT_A);
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

    gb->HandleInput(joypad);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();
    // ImGui::ShowDemoWindow();

    if (ImGui::Begin("CPU Debug")) {
      ShowCPUDebug(*gb, running);
    }
    ImGui::End();

    ImGui::Begin("Sprites");
    ShowSpriteDebug(*gb);
    ImGui::End();

    ImGui::Begin("Games");
    if (ImGui::Button("Open File")) {
      ImGui::OpenPopup("Open File");
    }

    if (ImGui::BeginPopupModal("Open File", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
      // display files in current directory
      // button to go up directory tree
      // only show .gb files?
      ImGui::Text(roms_dir.string().c_str());

      // for now just list roms in rom dir
      ImGui::SameLine();
      if (ImGui::Button("^")) {
        roms_dir = roms_dir.parent_path();
      }
      // for (auto &p :
      // std::filesystem::recursive_directory_iterator(roms_dir))
      // {
      for (auto &p : std::filesystem::directory_iterator(roms_dir)) {
        if (p.is_directory()) {
          if (ImGui::Selectable(p.path().string().c_str(), true,
                                ImGuiSelectableFlags_DontClosePopups)) {
            roms_dir /= p.path();
            break;
          }
        }
        // else show all .gb files
        if (p.path().extension() == ".gb") {
          if (ImGui::Selectable(p.path().string().c_str())) {
            auto file = std::ifstream{p.path(), std::ios::binary};
            auto cart = Load(file);
            gb = std::make_unique<Gameboy>(*cart, p.path().stem().string());
            running = true;
          }
        }
      }

      if (ImGui::Button("Cancel")) {
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }
    ImGui::End();

    if (ImGui::Begin("PPU Debug")) {
      ShowPPUDebug(*gb, framelimit, ui_draw_bg_map, ui_draw_tile_map);
      if (ui_draw_bg_map) {
        SDL_ShowWindow(bg_map_win);
      } else {
        SDL_HideWindow(bg_map_win);
      }
      if (ui_draw_tile_map) {
        SDL_ShowWindow(tile_map_win);
      } else {
        SDL_HideWindow(tile_map_win);
      }
    }
    ImGui::End();

    if (ImGui::Begin("MMU Debug")) {
      ShowMMUDebug(*gb);
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
    // if (gb.get_prev_opcode() == 0xFA) { // EI TODO: add support for RETI
    // instruction? 	gb.enable_interrupt();
    //}

    SDL_UpdateTexture(tex, nullptr, gb->ppu.Render().get(), 160 * 4);
    SDL_RenderClear(ren);
    SDL_RenderCopy(ren, tex, nullptr, nullptr);
    SDL_RenderPresent(ren);

    SDL_UpdateTexture(bg_map_tex, nullptr,
                      gb->ppu.RenderBackgroundTileMap().get(), 256 * 4);
    SDL_RenderClear(bg_map_ren);
    SDL_RenderCopy(bg_map_ren, bg_map_tex, nullptr, nullptr);
    SDL_RenderPresent(bg_map_ren);

    SDL_UpdateTexture(tile_map_tex, nullptr, gb->ppu.RenderTiles().get(),
                      128 * 4);
    SDL_RenderClear(tile_map_ren);
    SDL_RenderCopy(tile_map_ren, tile_map_tex, nullptr, nullptr);
    SDL_RenderPresent(tile_map_ren);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration_us =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::ostringstream s;
    s << "Loop took " << duration.count() << " ms (" << duration_us.count()
      << " us)" << std::endl;
    SDL_SetWindowTitle(win, s.str().c_str());

    if (framelimit && duration < tickrate) {
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

  SDL_Quit();

  return 0;
}
