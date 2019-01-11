#include <cstdint>
#include <iostream>
#include "cpu.h"
#include "mmu.h"
#include "logger.h"
#include "instructions.h"

CPU::CPU(MMU &m) : mmu(m), pc(0x0000), sp(0xFFFE)
{
	registers.af = 0x01B0;
	registers.bc = 0x0013;
	registers.de = 0x00D8;
	registers.hl = 0x014D;
}

void CPU::print()
{
	Logger::logger->debug("Registers: af:{0:04x} bc:{1:04x} de:{2:04x} hl:{3:04x}", registers.af, registers.bc, registers.de, registers.hl);
	Logger::logger->debug("pc: {0:04x} sp: {1:x}", pc, sp);
	Logger::logger->debug("Z: {0} C: {1}", flags.z, flags.c);

}

uint8_t CPU::step()
{
	uint8_t op = mmu.read_byte(pc);
	//auto inst = instructions[op];
	char c;
	//if (pc == 0x0040) std::cin >> c; // bg tile map written
	//if (pc == 0x0055) std::cin >> c; // bg scroll count and turning on lcd
	//if (pc == 0x005d) std::cin >> c; // nintendo logo scaled and copied to vram
	//if (pc == 0x0095) std::cin >> c;
	//if (pc == 0x0040) std::cin >> c;
	// fetch?
	Instruction opcode;
	opcode = (Instruction)mmu.read_byte(pc);
	switch (opcode) {
		// CB prefixed opcodes
	case Instruction::prefix_cb:
	{
		auto opcode2 = Prefix_CB(mmu.read_byte(pc + 1));
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
			//uint8_t bit = (registers.c << 1);
			uint8_t bit = (registers.c >> 7) & 0x01;
			registers.c <<= 1;
			uint8_t carry;
			flags.c == true ? carry = 0x01 : carry = 0x00;
			// carry flag holds old bit 7
			bit == 1U ? flags.c = true : flags.c = false;
			//flags.c = bit;
			// update Z flag
			bit == 0 ? flags.z = true : flags.z = false;
			//flags.z = false;
			// reset subtract flag
			flags.n = false;
			// reset half carry flag
			flags.h = false;

			// put the previous carry back in the 0th position
			registers.c ^= (-carry ^ registers.c) & (1U << 0);
			pc += 2;
			cycles = 8;
			break;
		}
		case Prefix_CB::bit_7_h:
			if ((registers.h >> 7) & 1U)
				flags.z = false;
			else
				flags.z = true;
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
		registers.b = mmu.read_byte(pc+1);
		pc += 2;
		cycles = 8;
		break;
	case Instruction::ld_c_d8: // LD C,n
		registers.c = mmu.read_byte(pc + 1);
		pc += 2;
		cycles = 8;
		break;
	case Instruction::ld_d_d8: // LD D,n
		registers.d = mmu.read_byte(pc + 1);
		pc += 2;
		cycles = 8;
		break;
	case Instruction::ld_e_d8: // LD E,n
		registers.e = mmu.read_byte(pc + 1);
		pc += 2;
		cycles = 8;
		break;
	case Instruction::ld_h_d8: // LD H,n
		registers.h = mmu.read_byte(pc + 1);
		pc += 2;
		cycles = 8;
		break;
	case Instruction::ld_l_d8: // LD L,n
		registers.l = mmu.read_byte(pc + 1);
		pc += 2;
		cycles = 8;
		break;
	case Instruction::ld_a_a:
		// ??
		registers.a = registers.a;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_a_b:
		registers.a = registers.b;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_a_c:
		registers.a = registers.c;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_a_d:
		registers.a = registers.d;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_a_e:
		registers.a = registers.e;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_a_h:
		registers.a = registers.h;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_a_l:
		registers.a = registers.l;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_b_a:
		registers.b = registers.a;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_c_a:
		registers.c = registers.a;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_d_a:
		registers.d = registers.a;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_h_a:
		registers.h = registers.a;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_a_de:
	{
		registers.a = mmu.read_byte(registers.de);
		++pc;
		cycles = 8;
		break;
	}
	case Instruction::ld_a_at_hl:
	{
		registers.a = mmu.read_byte(registers.hl);
		++pc;
		cycles = 8;
		break;
	}
	case Instruction::ld_a16_a:
	{
		uint16_t address = (mmu.read_byte(pc+2) << 8) + mmu.read_byte(pc + 1);
		mmu.write_byte(address, registers.a);
		cycles = 16;
		pc += 3;
		break;
	}
	case Instruction::ldd_hl_a: // ld (HL-), A
	{
		mmu.write_byte(registers.hl, registers.a);
		--registers.hl;
		++pc;
		cycles = 8;
		break;
	}
	case Instruction::ld_a_d8: // ld A, #
		registers.a = mmu.read_byte(pc + 1);
		pc += 2;
		cycles = 8;
		break;
	case Instruction::ld_at_c_a: // ld (C), a
	{
		// TODO: check this
		uint16_t address = 0xFF00 + registers.c;
		mmu.write_byte(address, registers.a);
		++pc;
		cycles = 8;
		break;
	}
	case Instruction::ld_hl_a:
	{
		mmu.write_byte(registers.hl, registers.a);
		++pc;
		cycles = 8;
		break;
	}
	case Instruction::ldh_a8_a:
	{
		uint16_t address = 0xFF00 + mmu.read_byte(pc + 1);
		mmu.write_byte(address, registers.a);
		pc += 2;
		cycles = 12;
		break;
	}
	case Instruction::ldh_a_a8:
	{
		uint16_t address = 0xFF00 + mmu.read_byte(pc + 1);
		registers.a = mmu.read_byte(address);
		pc += 2;
		cycles = 12;
		break;
	}
	case Instruction::ldi_hl_a:
	{
		mmu.write_byte(registers.hl, registers.a);
		++registers.hl;
		++pc;
		cycles = 8;
		break;
	}
	// 16 bit loads
	case Instruction::ld_sp_d16:
		sp = (mmu.read_byte(pc + 2) << 8) + mmu.read_byte(pc + 1);
		pc += 3;
		cycles = 12;
		break;
	case Instruction::ld_hl_d16:
		registers.h = mmu.read_byte(pc + 2);
		registers.l = mmu.read_byte(pc + 1);
		pc += 3;
		cycles = 12;
		break;
	case Instruction::ld_de_d16:
		registers.d = mmu.read_byte(pc+2);
		registers.e = mmu.read_byte(pc+1);
		pc += 3;
		cycles = 12;
		break;
	case Instruction::push_bc:
		mmu.write_byte(sp, registers.b);
		mmu.write_byte(sp - 1, registers.c);
		sp -= 2;
		++pc;
		cycles = 16;
		break;
	case Instruction::pop_bc:
		registers.b = mmu.read_byte(sp + 2);
		registers.c = mmu.read_byte(sp + 1);
		sp += 2;
		++pc;
		cycles = 12;
		break;
		// 8 bit ALU
	case Instruction::add_a_hl:
	{
		uint8_t value = mmu.read_byte(registers.hl);
		uint16_t res = registers.a + value;
		registers.a += value;

		if (registers.a == 0)
			flags.z = true;
		else
			flags.z = false;
		if (res > UINT8_MAX)
			flags.c = true;
		else
			flags.c = false;
		// todo: fix half carry flag
		if ((registers.a ^ (value) ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		flags.n = false;

		++pc;
		cycles = 8;
		break;
	}
	case Instruction::sub_d8:
	{
		uint8_t res = registers.a - mmu.read_byte(pc+1);
		// TODO: set flags
		if (res == 0)
			flags.z = true;
		else
			flags.z = false;
		if (res < 0)
			flags.c = true;
		else
			flags.c = false;
		if ((registers.a ^ (-mmu.read_byte(pc + 1)) ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		flags.n = true;

		registers.a = res;
		pc += 2;
		cycles = 8;
		break;
	}
	case Instruction::sub_b:
	{
		uint8_t res = registers.a - registers.b;
		if (res == 0)
			flags.z = true;
		else
			flags.z = false;
		if (res < 0)
			flags.c = true;
		else
			flags.c = false;
		if ((registers.a ^ (-registers.b) ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		flags.n = true;

		registers.a = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::xor_a:
	{
		uint8_t res = registers.a ^ registers.a;
		if (res == 0)
			flags.z = true;
		else
			flags.z = false;
		flags.c = false;
		flags.h = false;
		flags.n = false;
		registers.a = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::inc_b:
	{
		uint8_t res = registers.b + 1;
		registers.b = res;
		if (registers.b == 0)
			flags.z = true;
		else
			flags.z = false;
		if ((registers.b ^ 1 ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		flags.n = false;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::inc_c:
	{
		uint8_t res = registers.c + 1;
		registers.c = res;
		if (registers.c == 0)
			flags.z = true;
		else
			flags.z = false;
		if ((registers.c ^ 1 ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		flags.n = false;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::inc_h:
	{
		uint8_t res = registers.h + 1;
		registers.h = res;
		if (registers.h == 0)
			flags.z = true;
		else
			flags.z = false;
		if ((registers.h ^ 1 ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		flags.n = false;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::dec_a:
	{
		uint8_t res = registers.a - 1;
		registers.a = res;
		if (registers.a == 0)
			flags.z = true;
		else
			flags.z = false;
		if ((registers.a ^ (-1) ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		flags.n = true;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::dec_b:
	{
		uint8_t res = registers.b - 1;
		registers.b = res;
		if (registers.b == 0)
			flags.z = true;
		else
			flags.z = false;
		if ((registers.b ^ (-1) ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		flags.n = true;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::dec_c:
	{
		uint8_t res = registers.c - 1;
		registers.c = res;
		if (registers.c == 0)
			flags.z = true;
		else
			flags.z = false;
		if ((registers.c ^ (-1) ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		flags.n = true;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::dec_d:
	{
		uint8_t res = registers.d - 1;
		registers.d = res;
		if (registers.d == 0)
			flags.z = true;
		else
			flags.z = false;
		if ((registers.d ^ (-1) ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		flags.n = true;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::dec_e:
	{
		uint8_t res = registers.e - 1;
		registers.e = res;
		if (registers.e == 0)
			flags.z = true;
		else
			flags.z = false;
		if ((registers.e ^ (-1) ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		flags.n = true;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::cp_d8:
	{
		uint8_t res = registers.a - mmu.read_byte(pc + 1);
		if (res == 0)
			flags.z = true;
		else
			flags.z = false;
		flags.n = true;
		if ((registers.a ^ (-mmu.read_byte(pc + 1)) ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		if (res < 0)
			flags.c = true;
		else
			flags.c = false;
		pc += 2;
		cycles = 8;
		break;
	}
	case Instruction::cp_hl:
	{
		uint8_t res = registers.a - mmu.read_byte(registers.hl);
		if (res == 0)
			flags.z = true;
		else
			flags.z = false;
		flags.n = true;
		if ((registers.a ^ (-registers.hl) ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		if (res < 0)
			flags.c = true;
		else
			flags.c = false;
		++pc;
		cycles = 8;
		break;
	}
	// 16 bit arithmetic
	case Instruction::inc_hl:
		++registers.hl;
		++pc;
		cycles = 8;
		break;
	case Instruction::inc_de:
		++registers.de;
		++pc;
		cycles = 8;
		break;
		// Jumps
	case Instruction::jp_a16:
	{
		uint8_t c = mmu.read_byte(pc + 1);
		uint8_t d = mmu.read_byte(pc + 2);
		pc = (d << 8) + c;
		cycles = 16;
		break;
	}
	case Instruction::jr_r8:
		// must jump from the address of the NEXT instruction (ie 2 bytes after this one)
		pc += (int8_t)(mmu.read_byte(pc + 1) + 2);
		cycles = 12;
		break;
	case Instruction::jr_nz_r8:
		if (flags.z) {
			pc += 2;
			cycles = 8;
		}
		else {
			int8_t offset = (int8_t)mmu.read_byte(pc + 1);
			pc += (2 + offset);
			cycles = 12;
		}
		break;
	case Instruction::jr_z_r8:
		if (flags.z) {
			int8_t offset = (int8_t)mmu.read_byte(pc + 1);
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
		// TODO: verify
		uint8_t bit = (registers.a >> 7) & 0x01;
		registers.a <<= 1;
		// save old carry to put back in bit 0
		//uint8_t carry = (registers[(uint8_t)Register::f] >> 4) & 0x01;
		uint8_t carry;
		flags.c == true ? carry = 0x01 : carry = 0x00;
		// put bit in carry flag
		bit == 0 ? flags.c = false : flags.c = true;
		flags.z = false;
		flags.n = false;
		flags.h = false;
		// put the previous carry back in the 0th position
		registers.a ^= (-carry ^ registers.a) & (1U << 0);
		++pc;
		cycles = 4;
		break;
	}
	// Calls
	case Instruction::call_a16:
		mmu.write_byte(sp, (pc + 3) >> 8);
		mmu.write_byte(sp - 1, pc + 3);
		sp -= 2;
		pc = (mmu.read_byte(pc + 2) << 8) + mmu.read_byte(pc + 1);
		cycles = 12;
		break;
		// Returns
	case Instruction::ret:
	{
		pc = (mmu.read_byte(sp + 2) << 8) + mmu.read_byte(sp + 1);
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
		Logger::logger->error("Unknown opcode 0x{0:02x} at PC 0x{1:04x}", (uint8_t)opcode, pc);
		halted = true;
		//printf("Unknown opcode 0x%0.2X at PC 0x%0.4X\n", opcode, pc);
		//std::cerr << "Unknown Opcode: " << std::hex << opcode << std::dec << std::endl;
		break;
	}

	// Decode
	return (uint8_t)opcode;
}