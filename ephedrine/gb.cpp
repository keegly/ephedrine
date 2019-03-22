#include <memory>
#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include "spdlog/spdlog.h"

#include "gb.h"
#include "cpu.h"
#include "mmu.h"
#include "ppu.h"

//#include "instructions.h"

uint16_t Gameboy::divider_ = 0;
std::array<uint8_t, 2> Gameboy::joypad{ {0xf, 0xf} };

Gameboy::Gameboy() : mmu(std::make_shared<MMU>()), ppu(std::make_shared<PPU>(mmu))
{
	// no game
	divider_ = 0xABCC;
	cpu = std::make_shared<CPU>(this->mmu);
}

Gameboy::Gameboy(std::vector<uint8_t> cart, std::string game) : mmu(std::make_shared<MMU>(cart)), ppu(std::make_shared<PPU>(mmu)), game_(game)
{
	divider_ = 0xABCC;
	// inexplicably this only works down here?
	// if done in the list above, it ends up as nullptr
	cpu = std::make_shared<CPU>(this->mmu);
	//std::ifstream ifs{ game_ + ".sav", std::ios::binary };
	//if (ifs) {
	//	boost::archive::binary_iarchive ia(ifs);
	//	ia >> *mmu;
	//}
}

Gameboy::~Gameboy()
{
	// save the "battery buffered" external ram to disk
	//if (mmu->cart_ram_modified) {
	//	std::ofstream ofs{ game_ + ".sav", std::ios::binary };
	//	if (ofs) {
	//		boost::archive::binary_oarchive oa(ofs);
	//		oa << *mmu;
	//	}
	//	// unneccessary?
	//	mmu->cart_ram_modified = false;
	//}
}
void Gameboy::Load(std::vector<uint8_t> cartridge)
{
	mmu->load(cartridge);
	mmu->boot_rom_enabled = false;
}

void Gameboy::SaveState()
{
	// TODO
}

void Gameboy::LoadState()
{
	// TODO
}

void Gameboy::TimerTick(int cycles)
{
	// divider is always counting regardless
	this->divider_ += cycles;
	mmu->SetRegister(DIV, divider_ >> 8);
	uint8_t timer_ctrl = mmu->ReadByte(TAC);

	if (!bit_check(timer_ctrl, 2))
		return;

	// timer enabled
	uint8_t timer_modulo = mmu->ReadByte(TMA);
	uint8_t timer_counter = mmu->ReadByte(TIMA);
	if (timer_ticks_ <= clocks_[timer_ctrl & 0x03]) {
		timer_ticks_ += cycles;
	}
	else {
		if (timer_counter == 0xFF) {
			timer_counter = timer_modulo;
			// request timer interrupt (bit 2)
			uint8_t int_req = mmu->ReadByte(IF);
			bit_set(int_req, 2);
			mmu->WriteByte(IF, int_req);
		}
		else {
			++timer_counter;
		}
		//timer_counter == 0xFF ? timer_counter = timer_modulo : ++timer_counter;
		//spdlog::get("stdout")->debug("timer counter: {0}, modulo: {1}, clocks_:{2}", timer_counter, timer_modulo, clocks_[timer_ctrl & 0x03]);
		mmu->WriteByte(TIMA, timer_counter);
		timer_ticks_ = 0;
	}
}

void Gameboy::handle_input(std::array<uint8_t, 2> jp)
{
	//std::lock_guard<std::mutex> lg(mutex);
	// update our internal joypad
	joypad = jp;
	// only request joypad int if there's a button pressed
	if (joypad[0] < 0x0F || joypad[1] < 0x0F) {
		uint8_t int_flag = mmu->ReadByte(IF);
		// bit 4 is joypad interrupt request - remove magic number usage here
		// TODO
		bit_set(int_flag, 4);
		mmu->WriteByte(IF, int_flag);
	}
}

int Gameboy::tick(int ticks)
{
	// std::lock_guard<std::mutex> lg(mutex);
	// A vertical refresh happens every 70224 clocks(140448 in GBC double speed mode) : 59, 7275 Hz
	while (current_screen_cycles_ <= ticks) {
		// step x ticks
		cpu->step();
		//handle interrupts
		// if we're on the HALT opcode, need to handle interrupts
		// a little differently
		if (!cpu->halted)
			cpu->handle_interrupts();
		TimerTick(cpu->cycles);
		ppu->Update(cpu->cycles);
		current_screen_cycles_ += cpu->cycles;
	}
	current_screen_cycles_ = 0;

	return cpu->cycles;
}
