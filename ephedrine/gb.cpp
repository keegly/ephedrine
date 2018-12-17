#include <cstdint>
#include <iostream>
#include <array>
#include "gb.h"
//#include "instructions.h"


Gameboy::Gameboy()
{
	reset();
}

void Gameboy::reset()
{
	pc = 0x0000;
	ime = false;
	memory[0xFF05] = 00;
	memory[0xFF06] = 00;
	memory[0xFF07] = 00;
	memory[0xFF10] = 0x80;
	memory[0xFF11] = 0xBF;
	// turn on LCD
	memory[0xFF40] |= 1U << 7;

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

void Gameboy::handle_interrupts()
{
	if (!ime)
		return; // interrupts globally disabled
	// check register 0xFF0F to see which interrupt was generated
	const uint16_t offset[]{ 0x0040, 0x0048, 0x0050, 0x0058, 0x0060 };
	uint16_t address = 0x0000;
	uint8_t mask = 0x01;
	for (uint8_t i = 0; i < 5; ++i) {
		if (mask && memory[0xFF0F]) {
			address = offset[i];
			// clear teh bit
			memory[0xFF0F] &= ~(1U << i);
			break;
		}
		mask <<= 1;
	}
	if (address == 0x0000) return;
	// put current pc on stack
	memory[sp] = pc >> 8;
	memory[sp - 1] = pc;
	sp -= 2;

	// set pc to the correct interrupt handler address
	pc = address;
}

void Gameboy::step()
{
	// Fetch
	//opcode = instructions[memory[pc]];
	prev_opcode = opcode;
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
			// TODO
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::rl_c:
		{
			// if bit is 0 set Z
			// reset N
			// reset H
			// put bit in C
			//registers[(uint8_t)Register::f]
			uint8_t bit = (registers[(uint8_t)Register::c] << 1);
			// carry flag holds old bit 7
			registers[(uint8_t)Register::f] ^= (-bit ^ registers[(uint8_t)Register::f]) & (1U << 4);
			// update Z flag
			if (!bit) set_z();
			// reset subtract flag
			reset_n();
			// reset half carry flag
			reset_h();
			registers[(uint8_t)Register::c] <<= 1;
			pc += 2;
			cycles = 8;
			break;
		}
		case Prefix_CB::bit_7_h:
			if ((registers[(uint8_t)Register::h] >> 7) & 1U)
				reset_z();
			else
				set_z();
			pc += 2;
			cycles = 8;
			break;
		default:
			printf("Unknown opcode: 0x%0.2X 0x%0.2X\n", opcode, opcode2);
			//std::cerr << "Unknown Opcode: " << std::hex << opcode << std::dec << std::endl;
			break;
		}
		break;
	}
	case Instruction::nop:
		++pc;
		cycles = 4;
		break;
	case Instruction::stop:
		cycles = 4;
		break;
	// 8 bit loads
	case Instruction::ld_b_d8:
		registers[(uint8_t)Register::b] = memory[pc + 1];
		pc +=2;
		cycles = 8;
		break;
	case Instruction::ld_c_d8: // LD C,n
		registers[(uint8_t)Register::c] = memory[pc + 1];
		pc += 2;
		cycles = 8;
		break;
	case Instruction::ld_d_d8: // LD D,n
		registers[(uint8_t)Register::d] = memory[pc + 1];
		pc += 2;
		cycles = 8;
		break;
	case Instruction::ld_e_d8: // LD E,n
		registers[(uint8_t)Register::e] = memory[pc + 1];
		pc += 2;
		cycles = 8;
		break;
	case Instruction::ld_h_d8: // LD H,n
		registers[(uint8_t)Register::h] = memory[pc + 1];
		pc += 2;
		cycles = 8;
		break;
	case Instruction::ld_l_d8: // LD L,n
		registers[(uint8_t)Register::l] = memory[pc + 1];
		pc += 2;
		cycles = 8;
		break;
	case Instruction::ld_a_a:
		// ??
		registers[(uint8_t)Register::a] = registers[(uint8_t)Register::a];
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_a_b:
		registers[(uint8_t)Register::a] = registers[(uint8_t)Register::b];
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_a_c:
		registers[(uint8_t)Register::a] = registers[(uint8_t)Register::c];
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_a_d:
		registers[(uint8_t)Register::a] = registers[(uint8_t)Register::d];
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_a_e:
		registers[(uint8_t)Register::a] = registers[(uint8_t)Register::e];
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_a_h:
		registers[(uint8_t)Register::a] = registers[(uint8_t)Register::h];
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_a_l:
		registers[(uint8_t)Register::a] = registers[(uint8_t)Register::l];
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_b_a:
		registers[(uint8_t)Register::b] = registers[(uint8_t)Register::a];
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_c_a:
		registers[(uint8_t)Register::c] = registers[(uint8_t)Register::a];
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_d_a:
		registers[(uint8_t)Register::d] = registers[(uint8_t)Register::a];
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_h_a:
		registers[(uint8_t)Register::h] = registers[(uint8_t)Register::a];
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_a_de:
	{
		uint16_t de = (registers[(uint8_t)Register::d] << 8) + registers[(uint8_t)Register::e];
		registers[(uint8_t)Register::a] = memory[de];
		++pc;
		cycles = 8;
		break;
	}
	case Instruction::ld_a_at_hl:
	{
		uint16_t hl = (registers[(uint8_t)Register::h] << 8) + registers[(uint8_t)Register::l];
		registers[(uint8_t)Register::a] = memory[hl];
		++pc;
		cycles = 8;
		break;
	}
	case Instruction::ld_a16_a:
	{
		uint16_t address = (memory[pc + 1] << 8) + memory[pc];
		registers[(uint8_t)Register::a] = memory[address];
		cycles = 16;
		pc += 3;
		break;
	}
	case Instruction::ldd_hl_a: // ld (HL-), A
	{
		uint16_t address = (registers[(uint8_t)Register::h] << 8) + registers[(uint8_t)Register::l];
		memory[address] = registers[(uint8_t)Register::a];
		--registers[(uint8_t)Register::l];
		// check underflow and if so, dec H too
		if (registers[(uint8_t)Register::l] == UINT8_MAX)
			--registers[(uint8_t)Register::h];
		++pc;
		cycles = 8;
		break;
	}
	case Instruction::ld_a_d8: // ld A, #
		registers[(uint8_t)Register::a] = memory[pc +1];
		pc += 2;
		cycles = 8;
		break;
	case Instruction::ld_at_c_a: // ld (C), a
	{
		uint16_t address = 0xFF00 + registers[(uint8_t)Register::c];
 		memory[address] = registers[(uint8_t)Register::a];
		++pc;
		cycles = 8;
		break;
	}
	case Instruction::ld_hl_a:
	{
		uint16_t address = (registers[(uint8_t)Register::h] << 8) + registers[(uint8_t)Register::l];
		memory[address] = registers[(uint8_t)Register::a];
		++pc;
		cycles = 8;
		break;
	}
	case Instruction::ldh_a8_a:
	{
		uint16_t address = 0xFF00 + memory[pc + 1];
		memory[address] = registers[(uint8_t)Register::a];
		pc += 2;
		cycles = 12;
		break;
	}
	case Instruction::ldh_a_a8:
	{
		uint16_t address = 0xFF00 + memory[pc + 1];
		registers[(uint8_t)Register::a] = memory[address];
		pc += 2;
		cycles = 12;
		break;
	}
	case Instruction::ldi_hl_a:
	{
		uint16_t address = (registers[(uint8_t)Register::h] << 8) + registers[(uint8_t)Register::l];
		memory[address] = registers[(uint8_t)Register::a];
		++registers[(uint8_t)Register::l];
		if (registers[(uint8_t)Register::l] == 0)
			++registers[(uint8_t)Register::h];
		++pc;
		cycles = 8;
		break;
	}
	// 16 bit loads
	case Instruction::ld_sp_d16:
		sp = (memory[pc + 2] << 8) + memory[pc + 1];
		pc += 3;
		cycles = 12;
		break;
	case Instruction::ld_hl_d16:
		registers[(uint8_t)Register::h] = memory[pc + 2];
		registers[(uint8_t)Register::l] = memory[pc + 1];
		pc += 3;
		cycles = 12;
		break;
	case Instruction::ld_de_d16:
		registers[(uint8_t)Register::d] = memory[pc + 2];
		registers[(uint8_t)Register::e] = memory[pc + 1];
		pc += 3;
		cycles = 12;
		break;
	case Instruction::push_bc:
		memory[sp] = registers[(uint8_t)Register::b];
		memory[sp - 1] = registers[(uint8_t)Register::c];
		sp -= 2;
		++pc;
		cycles = 16;
		break;
	case Instruction::pop_bc:
		registers[(uint8_t)Register::b] = memory[sp + 2];
		registers[(uint8_t)Register::c] = memory[sp + 1];
		sp += 2;
		++pc;
		cycles = 12;
		break;
	// 8 bit ALU
	case Instruction::sub_d8:
	{
		uint8_t res = registers[(uint8_t)Register::a] - memory[pc + 1];
		// TODO: set flags
		if (res == 0) 
			set_z();
		else
			reset_z();
		if (res < 0)
			set_c();
		else
			reset_c();
		if ((registers[(uint8_t)Register::a] ^ (-memory[pc + 1]) ^ res) & 0x10)
			set_h();
		else
			reset_h();
		set_n();
		
		registers[(uint8_t)Register::a] = res;
		pc += 2;
		cycles = 8;
		break;
	}
	case Instruction::sub_b:
	{
		uint8_t res = registers[(uint8_t)Register::a] - registers[(uint8_t)Register::b];
		if (res == 0)
			set_z();
		else
			reset_z();
		if (res < 0)
			set_c();
		else
			reset_c();
		if ((registers[(uint8_t)Register::a] ^ (-registers[(uint8_t)Register::b]) ^ res) & 0x10)
			set_h();
		else
			reset_h();
		set_n();

		registers[(uint8_t)Register::a] = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::xor_a:
	{
		uint8_t res = registers[(uint8_t)Register::a] ^ registers[(uint8_t)Register::a];
		if (res == 0)
			set_z();
		else
			reset_z();
		reset_c();
		reset_h();
		reset_n();
		registers[(uint8_t)Register::a] = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::inc_b:
	{
		uint8_t res = registers[(uint8_t)Register::b] + 1;
		registers[(uint8_t)Register::b] = res;
		if (registers[(uint8_t)Register::b] == 0)
			set_z();
		else
			reset_z();
		if ((registers[(uint8_t)Register::b] ^ 1 ^ res) & 0x10)
			set_h();
		else
			reset_h();
		reset_n();
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::inc_c:
	{
		uint8_t res = registers[(uint8_t)Register::c] + 1;
		registers[(uint8_t)Register::c] = res;
		if (registers[(uint8_t)Register::c] == 0)
			set_z();
		else
			reset_z();
		if ((registers[(uint8_t)Register::c] ^ 1 ^ res) & 0x10)
			set_h();
		else
			reset_h();
		reset_n();
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::inc_h:
	{
		uint8_t res = registers[(uint8_t)Register::h] + 1;
		registers[(uint8_t)Register::h] = res;
		if (registers[(uint8_t)Register::h] == 0)
			set_z();
		else
			reset_z();
		if ((registers[(uint8_t)Register::h] ^ 1 ^ res) & 0x10)
			set_h();
		else
			reset_h();
		reset_n();
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::dec_a:
	{
		uint8_t res = registers[(uint8_t)Register::a] - 1;
		registers[(uint8_t)Register::a] = res;
		if (registers[(uint8_t)Register::a] == 0)
			set_z();
		else
			reset_z();
		if ((registers[(uint8_t)Register::a] ^ (-1) ^ res) & 0x10)
			set_h();
		else
			reset_h();
		set_n();
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::dec_b:
	{
		uint8_t res = registers[(uint8_t)Register::b] - 1;
		registers[(uint8_t)Register::b] = res;
		if (registers[(uint8_t)Register::b] == 0)
			set_z();
		else
			reset_z();
		if ((registers[(uint8_t)Register::b] ^ (-1) ^ res) & 0x10)
			set_h();
		else
			reset_h();
		set_n();
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::dec_c:
	{
		uint8_t res = registers[(uint8_t)Register::c] - 1;
		registers[(uint8_t)Register::c] = res;
		if (registers[(uint8_t)Register::c] == 0)
			set_z();
		else
			reset_z();
		if ((registers[(uint8_t)Register::c] ^ (-1) ^ res) & 0x10)
			set_h();
		else
			reset_h();
		set_n();
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::dec_d:
	{
		uint8_t res = registers[(uint8_t)Register::d] - 1;
		registers[(uint8_t)Register::d] = res;
		if (registers[(uint8_t)Register::d] == 0)
			set_z();
		else
			reset_z();
		if ((registers[(uint8_t)Register::d] ^ (-1) ^ res) & 0x10)
			set_h();
		else
			reset_h();
		set_n();
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::dec_e:
	{
		uint8_t res = registers[(uint8_t)Register::e] - 1;
		registers[(uint8_t)Register::e] = res;
		if (registers[(uint8_t)Register::e] == 0)
			set_z();
		else
			reset_z();
		if ((registers[(uint8_t)Register::e] ^ (-1) ^ res) & 0x10)
			set_h();
		else
			reset_h();
		set_n();
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::cp_d8:
	{
		uint8_t res = registers[(uint8_t)Register::a] - memory[pc + 1];
		if (res == 0)
			set_z();
		else
			reset_z();
		set_n();
		if ((registers[(uint8_t)Register::a] ^ (-memory[pc+1]) ^ res) & 0x10)
			set_h();
		else
			reset_h();
		if (res < 0)
			set_c();
		else
			reset_c();
		pc += 2;
		cycles = 8;
		break;
	}
	// 16 bit arithmetic
	case Instruction::inc_hl:
		++registers[(uint8_t)Register::l];
		if (registers[(uint8_t)Register::l] == 0)
			++registers[(uint8_t)Register::h];
		++pc;
		cycles = 8;
		break;
	case Instruction::inc_de:
		++registers[(uint8_t)Register::e];
		if (registers[(uint8_t)Register::e] == 0)
			++registers[(uint8_t)Register::d];
		++pc;
		cycles = 8;
		break;
	// Jumps
	case Instruction::jp_a16:
	{
		uint8_t c = memory[pc + 1];
		uint8_t d = memory[pc + 2];
		pc = (d << 8) + c;
		cycles = 16;
		break;
	}
	case Instruction::jr_r8:
		// must jump from the address of the NEXT instruction (ie 2 bytes after this one)
		pc += (int8_t)(memory[pc + 1] + 2);
		cycles = 12;
		break;
	case Instruction::jr_nz_r8:
		if ((registers[(uint8_t)Register::f] >> 7) & 1U) {
			pc += 2;
			cycles = 8;
		} else {
			int8_t offset = (int8_t)memory[pc + 1];
			pc += (2 + offset);
			cycles = 12;
		}
		break;
	case Instruction::jr_z_r8:
		if ((registers[(uint8_t)Register::f] >> 7) & 1U) {
			int8_t offset = (int8_t)memory[pc + 1];
			pc += (2 + offset);
			cycles = 12;
		}
		else {
			pc += 2;
			cycles = 8;
		}
		break;
	// Rotates and shifts
	case Instruction::rla:
	{
		uint8_t bit = (registers[(uint8_t)Register::a] >> 7) & 0x01;
		registers[(uint8_t)Register::a] <<= 1;
		// save old carry to put back in bit 0
		uint8_t carry = (registers[(uint8_t)Register::f] >> 4) & 0x01;
		// put bit in carry flag
		registers[(uint8_t)Register::f] ^= (-bit ^ registers[(uint8_t)Register::f]) & (1U << 4);
		if (bit == 0)
			set_z();
		else
			reset_z();
		reset_n();
		reset_h();
		// put the previous carry back in the 0th position
		registers[(uint8_t)Register::a] ^= (-carry ^ registers[(uint8_t)Register::a]) & (1U << 0);
		++pc;
		cycles = 4;
		break;
	}
	// Calls
	case Instruction::call_a16:
		memory[sp] = (pc + 3) >> 8;
		memory[sp - 1] = pc + 3;
		sp -= 2;
		pc = (memory[pc + 2] << 8) + memory[pc + 1];
		cycles = 12;
		break;
	// Returns
	case Instruction::ret:
	{
		pc = (memory[sp + 2] << 8) + memory[sp + 1];
		sp += 2;
		cycles = 8;
		break;
	}
	// Misc
	case Instruction::di:
		// TODO: disable interrupts after the next instruction
		//ime = false;
		++pc;
		cycles = 4;
		break;
	case Instruction::ei:
		// TODO: enable interrupts after the next instruction
		//ime = true;
		++pc;
		cycles = 4;
		break;
	case Instruction::halt:
		// suspend and wait for interrupts
		if (!ime) ++pc; //act as a nop if interrupts are disabled
		cycles = 4;
		break;
	default:
		printf("Unknown opcode 0x%0.2X at PC 0x%0.4X\n", opcode, pc);
		//std::cerr << "Unknown Opcode: " << std::hex << opcode << std::dec << std::endl;
		break;
	}
}