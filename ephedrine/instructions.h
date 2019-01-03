#pragma once

struct DecodedInstruction {
	uint8_t opcode;
	std::string name;
	// addressing mde
	// source?
	// dest?
	int length;
	int cycles;
	//void (*fp)(void);
};

const DecodedInstruction instructions[] {
	{0x00, "nop", 1, 4},
	{0x01, "ld BC, d16", 3, 12},
	{0x02, "ld (BC), A", 1, 8},
	{0x03, "inc BC", 1, 8},
	{0x04, "inc B", 1, 4},
	{0x05, "dec B", 1, 5},
	{0x06, "ld B, d8", 2, 8}
};

const DecodedInstruction cb_instructions[]{
	{0x00, "rlc b", 1, 4}
};