#include "gb.h"

Gameboy::Gameboy()
{
	//pc = 0x100;
	reset();
}

void Gameboy::reset()
{
	pc = 0x100;
	sp = 0xFFFE;
}

void Gameboy::step()
{
	// Fetch
	unsigned short opcode;
}