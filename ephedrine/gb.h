#ifndef GB_H
#define GB_H

#include "cpu.h"
#include "mmu.h"
#include "ppu.h"

// Interrupt Flag
constexpr uint16_t IF   = 0xFF0F;
// Interrupt Enable
constexpr uint16_t IE   = 0xFFFF;
// Joypad Registers
constexpr uint16_t P1	= 0xFF00;
// Timer Registers
constexpr uint16_t DIV	= 0xFF04;
constexpr uint16_t TIMA = 0xFF05;
constexpr uint16_t TMA  = 0xFF06;
constexpr uint16_t TAC  = 0xFF07;
// PPU Registers
constexpr uint16_t LCDC = 0xFF40;
constexpr uint16_t STAT = 0xFF41;
constexpr uint16_t SCY  = 0xFF42;
constexpr uint16_t SCX  = 0xFF43;
constexpr uint16_t LY   = 0xFF44;
constexpr uint16_t LYC	= 0xFF45;
constexpr uint16_t BGP  = 0xFF47;

constexpr uint8_t INPUT_START = 0x08;
constexpr uint8_t INPUT_SELECT   = 0x04;
constexpr uint8_t INPUT_B = 0x02;
constexpr uint8_t INPUT_A = 0x01;
constexpr uint8_t INPUT_DOWN = 0x08;
constexpr uint8_t INPUT_UP = 0x04;
constexpr uint8_t INPUT_LEFT = 0x02;
constexpr uint8_t INPUT_RIGHT = 0x01;

class Gameboy
{
	public:
		Gameboy(std::vector<uint8_t> cart);
		void reset();
		void handle_interrupts();
		void handle_input(uint8_t joypad);
		void tick(int ticks);
		void load(std::vector<uint8_t> cart);
		static void set_timer(uint16_t t) { divider = t; }
		uint16_t get_timer() { return divider; }
		void timer_tick(int cycles);
		CPU cpu;
		MMU mmu;
		PPU ppu;
	private:
		const int max_cycles = 70224;
		int curr_screen_cycles = 0;
		static uint16_t divider;
		int timer_ticks;
		const int clocks[4] = { 1024, 16, 64, 256 };
};

#endif // !GB_H