#include <cstdint>
#include <iostream>

#include "gb.h"
//#include "instructions.h"


Gameboy::Gameboy()
{
	reset();
}

void Gameboy::reset()
{
	pc = 0x0000;
	memory[0xFF05] = 00;
	memory[0xFF06] = 00;
	memory[0xFF07] = 00;
	memory[0xFF10] = 0x80;
	memory[0xFF11] = 0xBF;

	// Load boot rom into memory
	for (int i = 0; i < 256; ++i) {
		memory[i] = rom[i];
	}
}

void Gameboy::load(uint8_t *cartridge, long size)
{
	for (int i = 0x100; i < size; ++i)
		memory[i] = cartridge[i];
}

void Gameboy::step()
{
	// Fetch
	//opcode = instructions[memory[pc]];
	opcode = Instruction(memory[pc]);

	// Decode
	switch (opcode) {
		// CB prefixed opcodes
	case Instruction::prefix_cb:
	{
		auto opcode2 = Prefix_CB(memory[pc + 1]);
		switch (opcode2)
		{
		case Prefix_CB::rlc_b:
			break;
		case Prefix_CB::bit_7_h:
			if ((registers[(uint8_t)Register::h] >> 7) & 1U)
				registers[(uint8_t)Register::f] &= ~(1U << 7); //reset bit 7
			pc += 2;
			break;
		default:
			printf("Unknown opcode: 0xCB 0x%0.2X\n", opcode2);
			//std::cerr << "Unknown Opcode: " << std::hex << opcode << std::dec << std::endl;
			break;
		}
		break;
	}
	case Instruction::nop:
		++pc;
		break;
	case Instruction::stop:
		break;
	// 8 bit loads
	// TODO: these should all be Increment PC by 1??
	case Instruction::ld_b_d8:
		registers[(uint8_t)Register::b] = memory[pc + 1];
		++pc;
		break;
	case Instruction::ld_c_d8: // LD C,n
		registers[(uint8_t)Register::c] = memory[pc + 1];
		++pc;
		break;
	case Instruction::ld_d_d8: // LD D,n
		registers[(uint8_t)Register::d] = memory[pc + 1];
		++pc;
		break;
	case Instruction::ld_e_d8: // LD E,n
		registers[(uint8_t)Register::e] = memory[pc + 1];
		++pc;
		break;
	case Instruction::ld_h_d8: // LD H,n
		registers[(uint8_t)Register::h] = memory[pc + 1];
		++pc;
		break;
	case Instruction::ld_l_d8: // LD L,n
		registers[(uint8_t)Register::l] = memory[pc + 1];
		++pc;
		break;
	case Instruction::ld_a_a:
		// ??
		registers[(uint8_t)Register::a] = registers[(uint8_t)Register::a];
		++pc;
		break;
	case Instruction::ld_a_b:
		registers[(uint8_t)Register::a] = registers[(uint8_t)Register::b];
		++pc;
		break;
	case Instruction::ld_a_c:
		registers[(uint8_t)Register::a] = registers[(uint8_t)Register::c];
		++pc;
		break;
	case Instruction::ld_a_d:
		registers[(uint8_t)Register::a] = registers[(uint8_t)Register::d];
		++pc;
		break;
	case Instruction::ld_a_e:
		registers[(uint8_t)Register::a] = registers[(uint8_t)Register::e];
		++pc;
		break;
	case Instruction::ld_a_h:
		registers[(uint8_t)Register::a] = registers[(uint8_t)Register::h];
		++pc;
		break;
	case Instruction::ld_a_l:
		registers[(uint8_t)Register::a] = registers[(uint8_t)Register::l];
		++pc;
		break;
	case Instruction::ld_b_a:
		registers[(uint8_t)Register::b] = registers[(uint8_t)Register::a];
		++pc;
		break;
	case Instruction::ld_a_at_hl:
	{
		uint16_t hl = (registers[(uint8_t)Register::h] << 8) + registers[(uint8_t)Register::l];
		registers[(uint8_t)Register::a] = memory[hl];
		++pc;
		break;
	}
	case Instruction::ld_a16_a:
	{
		++pc;
		uint16_t address = (memory[pc + 1] << 8) + memory[pc];
		registers[(uint8_t)Register::a] = memory[address];
		break;
	}
	case Instruction::ldd_hl_a:
	{
		uint16_t address = (registers[(uint8_t)Register::h] << 8) + registers[(uint8_t)Register::l];
		memory[address] = registers[(uint8_t)Register::a];
		--registers[(uint8_t)Register::l];
		// check underflow and if so, dec H too
		if (registers[(uint8_t)Register::l] == UINT8_MAX)
			--registers[(uint8_t)Register::h];
		++pc;
		break;
	}
	// 16 bit loads
	case Instruction::ld_sp_d16:
		++pc;
		sp = (memory[pc + 1] << 8) + memory[pc];
		pc += 2;
		break;
	case Instruction::ld_hl_d16:
		registers[(uint8_t)Register::h] = memory[pc + 2];
		registers[(uint8_t)Register::l] = memory[pc + 1];
		pc += 3;
		break;
	case Instruction::ld_de_d16:
		registers[(uint8_t)Register::d] = memory[pc + 2];
		registers[(uint8_t)Register::e] = memory[pc + 1];
		pc += 3;
		break;
	// 8 bit ALU
	case Instruction::sub_d8:
	{
		registers[(uint8_t)Register::a] -= memory[pc + 1];
		// TODO: set flags
		pc += 2;
		break;
	}
	case Instruction::xor_a:
		registers[(uint8_t)Register::a] ^= registers[(uint8_t)Register::a];
		++pc;
		break;
	// Jumps
	case Instruction::jp_a16:
	{
		++pc;
		uint8_t c = memory[pc];
		uint8_t d = memory[pc + 1];
		pc = (d << 8) + c;
		break;
	}
	case Instruction::jr_r8:
		pc += memory[pc + 1];
		break;
	case Instruction::jr_nz_r8:
		//TODO:  jump if Z flag is reset (0)
		if ((registers[(uint8_t)Register::f] >> 7) & 1U)
			pc += 2;
		else
		{
			int8_t offset = (int8_t)memory[pc + 1];
			pc += (2 + offset);
		}
		break;
	// Misc
	case Instruction::di:
		// TODO: disable interrupts after the next instruction
		ime = false;
		++pc;
		break;
	case Instruction::ei:
		// TODO: enable interrupts after the next instruction
		ime = true;
		++pc;
		break;
	default:
		printf("Unknown opcode: 0x%0.2X\n", opcode);
		//std::cerr << "Unknown Opcode: " << std::hex << opcode << std::dec << std::endl;
		break;
	}
}