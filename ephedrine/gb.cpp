#include <memory>

#include "spdlog/spdlog.h"

#include "gb.h"
#include "cpu.h"
#include "mmu.h"
#include "ppu.h"

//#include "instructions.h"

uint16_t Gameboy::divider = 0;
std::array<uint8_t, 2> Gameboy::joypad{ {0xf, 0xf} };

Gameboy::Gameboy(std::vector<uint8_t> cart) : mmu(cart), cpu(mmu), ppu(mmu)
{
	 divider = 0xABCC;
}

void Gameboy::load(std::vector<uint8_t> cartridge)
{
	mmu.load(cartridge);
	mmu.boot_rom_enabled = false;
}

void Gameboy::timer_tick(int cycles)
{
	// divider is always counting regardless
	this->divider += cycles;
	mmu.set_register(DIV, divider >> 8);
	uint8_t timer_ctrl = mmu.read_byte(TAC);

	if (!bit_check(timer_ctrl, 2))
		return;

	// timer enabled
	uint8_t timer_modulo = mmu.read_byte(TMA);
	uint8_t timer_counter = mmu.read_byte(TIMA);
	if (timer_ticks <= clocks[timer_ctrl & 0x03]) {
		timer_ticks += cycles;
	}
	else {
		if (timer_counter == 0xFF) {
			timer_counter = timer_modulo;
			// request timer interrupt (bit 2)
			uint8_t int_req = mmu.read_byte(IF);
			bit_set(int_req, 2);
			mmu.write_byte(IF, int_req);
		}
		else {
			++timer_counter;
		}
		//timer_counter == 0xFF ? timer_counter = timer_modulo : ++timer_counter;
		//spdlog::get("stdout")->debug("timer counter: {0}, modulo: {1}, clocks:{2}", timer_counter, timer_modulo, clocks[timer_ctrl & 0x03]);
		mmu.write_byte(TIMA, timer_counter);
		timer_ticks = 0;
	}
}

void Gameboy::handle_input(std::array<uint8_t, 2> jp)
{
	//std::lock_guard<std::mutex> lg(mutex);
	// update our internal joypad
	joypad = jp;
	// only request joypad int if there's a button pressed
	if (joypad[0] < 0x0F || joypad[1] < 0x0F)  {
		uint8_t int_flag = mmu.read_byte(IF);
		// bit 4 is joypad interrupt request - remove magic number usage here
		// #TODO
		bit_set(int_flag, 4);
		mmu.write_byte(IF, int_flag);
	}
}

int Gameboy::tick(int ticks)
{
	// std::lock_guard<std::mutex> lg(mutex);
	// A vertical refresh happens every 70224 clocks(140448 in GBC double speed mode) : 59, 7275 Hz
	while (curr_screen_cycles <= ticks) {
		// step x ticks
		cpu.step();
		//handle interrupts
		cpu.handle_interrupts();
		timer_tick(cpu.cycles);
		ppu.update(cpu.cycles);
		curr_screen_cycles += cpu.cycles;
	}
	curr_screen_cycles = 0;
	return cpu.cycles;
}
