#include <cstdint>
#include <iostream>
#include "gb.h"
#include "cpu.h"
#include "mmu.h"
#include "logger.h"
#include "bit_utility.h"
#include "instructions.h"

CPU::CPU(MMU &m) : mmu(m), pc(0x0100), sp(0xFFFE)
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

void CPU::handle_interrupts()
{
	if (!ime)
		return; // interrupts globally disabled
	// check register 0xFF0F to see which interrupt was generated
	constexpr uint16_t offset[]{ 0x0040, 0x0048, 0x0050, 0x0058, 0x0060 };
	uint16_t address = 0x0000;
	uint8_t mask = 0x01;
	uint8_t if_reg = mmu.read_byte(IF);
	for (uint8_t i = 0; i < 5; ++i) {
		if (mask && if_reg) {
			address = offset[i];
			// clear teh bit
			bit_clear(if_reg, i);
			mmu.write_byte(IF, if_reg);
			break;
		}
		mask <<= 1;
	}
	// no interrupts to handle, so bail
	if (address == 0x0000) return;
	// put current pc on stack and head to the proper service routine
	mmu.write_byte(sp, pc >> 8);
	mmu.write_byte(sp - 1, static_cast<uint8_t>(pc));
	sp -= 2;

	// set pc to the correct interrupt handler address
	pc = address;
}

uint8_t CPU::step()
{
	//  2a3 all was fine
	//if (pc == 0x29a6)
	//if (pc == 0xc003)
	//	Logger::logger->info("pc: {0:04X}, sp: {1:04X}", pc, sp);
	/*if (pc > 0x023e)
		Logger::logger->info("pc: {0:04X}, sp: {1:04X}", pc, sp);*/

	// this is just after the bg map should be shown for tetris
	/*if (pc == 0x02c7 || pc == 0x0150)
		Logger::logger->info("pc: {0:04X}, sp: {1:04X}", pc, sp);*/
	//Instruction opcode;
	auto opcode = (Instruction)mmu.read_byte(pc);
	switch (opcode) {
	// CB prefixed opcodes
	case Instruction::prefix_cb:
	{
		//auto opcode2 = Prefix_CB(mmu.read_byte(pc + 1));
		switch (Prefix_CB(mmu.read_byte(pc + 1)))
		{
		case Prefix_CB::rlc_b:
		{
			// #TODO - verify
			uint8_t bit = bit_check(registers.b, 7);
			registers.c <<= 1;
			// carry flag holds old bit 7
			bit == 1U ? flags.c = true : flags.c = false;
			// update Z flag
			bit == 0 ? flags.z = true : flags.z = false;
			// reset subtract flag
			flags.n = false;
			// reset half carry flag
			flags.h = false;

			// put the bit we shifted out back in the 0th position
			registers.b ^= (-bit ^ registers.b) & (1U << 0);
			pc += 2;
			cycles = 8;
			break;
		}
		case Prefix_CB::rl_c:
		{
			// if bit is 0 set Z
			// reset N
			// reset H
			// put bit in C
			uint8_t bit = bit_check(registers.c, 7);
			registers.c <<= 1;
			uint8_t carry;
			flags.c == true ? carry = 0x01 : carry = 0x00;
			// carry flag holds old bit 7
			bit == 1U ? flags.c = true : flags.c = false;
			// update Z flag
			bit == 0 ? flags.z = true : flags.z = false;
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
		case Prefix_CB::rr_c:
		{
			uint8_t bit = bit_check(registers.c, 0);
			registers.c >>= 1;
			// return the old carry back to bit 7, and put our new bit in the carry
			if (flags.c)
				bit_set(registers.c, 7);
			else
				bit_clear(registers.c, 7);
			bit == 0 ? flags.c = false : flags.c = true;
			flags.h = false;
			flags.n = false;
			pc += 2;
			cycles = 8;
			break;
		}
		case Prefix_CB::rr_d:
		{
			uint8_t bit = bit_check(registers.d, 0);
			registers.d >>= 1;
			// return the old carry back to bit 7, and put our new bit in the carry
			if (flags.c)
				bit_set(registers.d, 7);
			else
				bit_clear(registers.d, 7);
			bit == 0 ? flags.c = false : flags.c = true;
			flags.h = false;
			flags.n = false;
			pc += 2;
			cycles = 8;
			break;
		}
		case Prefix_CB::rr_e:
		{
			uint8_t bit = bit_check(registers.e, 0);
			registers.e >>= 1;
			// return the old carry back to bit 7, and put our new bit in the carry
			if (flags.c)
				bit_set(registers.e, 7);
			else
				bit_clear(registers.e, 7);
			bit == 0 ? flags.c = false : flags.c = true;
			flags.h = false;
			flags.n = false;
			pc += 2;
			cycles = 8;
			break;
		}
		case Prefix_CB::srl_a:
		{
			// shift right into carry
			uint8_t bit = bit_check(registers.a, 0);
			registers.a >>= 1;
			bit == 0 ? flags.c = false : flags.c = true;
			bit == 0 ? flags.z = true : flags.z = false;
			flags.h = false;
			flags.n = false;
			pc += 2;
			cycles = 8;
			break;
		}
		case Prefix_CB::srl_b:
		{
			// shift right into carry
			uint8_t bit = bit_check(registers.b, 0);
			registers.b >>= 1;
			bit == 0 ? flags.c = false : flags.c = true;
			bit == 0 ? flags.z = true : flags.z = false;
			flags.h = false;
			flags.n = false;
			pc += 2;
			cycles = 8;
			break;
		}
		case Prefix_CB::swap_a:
		{
			uint8_t temp = registers.a;
			registers.a <<= 4;
			temp >>= 4;
			registers.a &= 0xF0;
			temp &= 0x0F;
			registers.a |= temp;
			flags.n = false;
			flags.h = false;
			flags.c = false;
			pc += 2;
			cycles = 8;
			break;
		}
		case Prefix_CB::bit_7_h:
			if (bit_check(registers.h, 7))
				flags.z = false;
			else
				flags.z = true;
			pc += 2;
			cycles = 8;
			break;
		default:
			//printf("Unknown opcode: 0x%0.2X 0x%0.2X\n", opcode, opcode2);
			Logger::logger->error("Unknown opcode 0x{0:02X} 0x{1:02X} at 0x{2:04X}", (uint8_t)opcode, (uint8_t)mmu.read_byte(pc + 1), pc);
			halted = true;
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
		registers.b = mmu.read_byte(pc + 1);
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
	case Instruction::ld_a_bc:
		registers.a = mmu.read_byte(registers.bc);
		++pc;
		cycles = 8;
		break;
	case Instruction::ld_a_de:
		registers.a = mmu.read_byte(registers.de);
		++pc;
		cycles = 8;
		break;
	case Instruction::ld_a_at_hl:
		registers.a = mmu.read_byte(registers.hl);
		++pc;
		cycles = 8;
		break;
	case Instruction::ld_a_a16:
	{
		uint16_t address = (mmu.read_byte(pc + 2) << 8) + mmu.read_byte(pc + 1);
		registers.a = mmu.read_byte(address);
		pc += 3;
		cycles = 16;
		break;
	}
	case Instruction::ld_b_a:
		registers.b = registers.a;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_b_b:
		registers.b = registers.b;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_b_c:
		registers.b = registers.c;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_b_d:
		registers.b = registers.d;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_b_e:
		registers.b = registers.e;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_b_h:
		registers.b = registers.h;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_b_l:
		registers.b = registers.l;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_b_hl:
		registers.b = mmu.read_byte(registers.hl);
		++pc;
		cycles = 8;
		break;
	case Instruction::ld_c_a:
		registers.c = registers.a;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_c_b:
		registers.c = registers.b;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_c_c:
		registers.c = registers.c;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_c_d:
		registers.c = registers.d;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_c_e:
		registers.c = registers.e;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_c_h:
		registers.c = registers.h;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_c_l:
		registers.c = registers.l;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_c_hl:
		registers.c = mmu.read_byte(registers.hl);
		++pc;
		cycles = 8;
		break;
	case Instruction::ld_d_a:
		registers.d = registers.a;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_d_b:
		registers.d = registers.b;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_d_c:
		registers.d = registers.c;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_d_d:
		registers.d = registers.d;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_d_e:
		registers.d = registers.e;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_d_h:
		registers.d = registers.h;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_d_l:
		registers.d = registers.l;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_d_hl:
		registers.d = mmu.read_byte(registers.hl);
		++pc;
		cycles = 8;
		break;
	case Instruction::ld_e_a:
		registers.e = registers.a;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_e_b:
		registers.e = registers.b;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_e_c:
		registers.e = registers.c;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_e_d:
		registers.e = registers.d;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_e_e:
		registers.e = registers.e;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_e_h:
		registers.e = registers.h;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_e_l:
		registers.e = registers.l;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_e_hl:
		registers.e = mmu.read_byte(registers.hl);
		++pc;
		cycles = 8;
		break;
	case Instruction::ld_h_a:
		registers.h = registers.a;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_h_b:
		registers.h = registers.b;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_h_c:
		registers.h = registers.c;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_h_d:
		registers.h = registers.d;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_h_e:
		registers.h = registers.e;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_h_h:
		registers.h = registers.h;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_h_l:
		registers.h = registers.l;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_h_hl:
		registers.h = mmu.read_byte(registers.hl);
		++pc;
		cycles = 8;
		break;
	case Instruction::ld_l_a:
		registers.l = registers.a;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_l_b:
		registers.l = registers.b;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_l_c:
		registers.l = registers.c;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_l_d:
		registers.l = registers.d;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_l_e:
		registers.l = registers.e;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_l_h:
		registers.l = registers.h;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_l_l:
		registers.l = registers.l;
		++pc;
		cycles = 4;
		break;
	case Instruction::ld_l_hl:
		registers.l = mmu.read_byte(registers.hl);
		++pc;
		cycles = 8;
		break;
	case Instruction::ld_a16_a:
	{
		uint16_t address = (mmu.read_byte(pc + 2) << 8) + mmu.read_byte(pc + 1);
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
	case Instruction::ld_de_a:
	{
		mmu.write_byte(registers.de, registers.a);
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
	case Instruction::ld_hl_b:
	{
		mmu.write_byte(registers.hl, registers.b);
		++pc;
		cycles = 8;
		break;
	}
	case Instruction::ld_hl_c:
	{
		mmu.write_byte(registers.hl, registers.c);
		++pc;
		cycles = 8;
		break;
	}
	case Instruction::ld_hl_d:
	{
		mmu.write_byte(registers.hl, registers.d);
		++pc;
		cycles = 8;
		break;
	}
	case Instruction::ld_hl_e:
	{
		mmu.write_byte(registers.hl, registers.e);
		++pc;
		cycles = 8;
		break;
	}
	case Instruction::ld_hl_h:
	{
		mmu.write_byte(registers.hl, registers.h);
		++pc;
		cycles = 8;
		break;
	}
	case Instruction::ld_hl_l:
	{
		mmu.write_byte(registers.hl, registers.l);
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
	case Instruction::ld_hl_d8:
	{
		uint8_t val = mmu.read_byte(pc + 1);
		mmu.write_byte(registers.hl, val);
		pc += 2;
		cycles = 12;
		break;
	}
	case Instruction::ldi_a_hl:
		registers.a = mmu.read_byte(registers.hl);
		++registers.hl;
		++pc;
		cycles = 8;
		break;
		// 16 bit loads
	case Instruction::ld_a16_sp:
	{
		uint16_t address = (mmu.read_byte(pc + 2) << 8) + mmu.read_byte(pc + 1);
		mmu.write_byte(address, pc);
		pc += 3;
		cycles = 20;
		break;
	}
	case Instruction::ld_sp_d16:
		sp = (mmu.read_byte(pc + 2) << 8) + mmu.read_byte(pc + 1);
		pc += 3;
		cycles = 12;
		break;
	case Instruction::ld_bc_d16:
		registers.bc = (mmu.read_byte(pc + 2) << 8) + mmu.read_byte(pc + 1);
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
		registers.d = mmu.read_byte(pc + 2);
		registers.e = mmu.read_byte(pc + 1);
		pc += 3;
		cycles = 12;
		break;
	case Instruction::push_af:
		// since we don't use the bits of the F register
		// and we need to save the flags state, set them now
		if (flags.z)
			set_z();
		else
			reset_z();
		if (flags.n)
			set_n();
		else
			reset_n();
		if (flags.h)
			set_h();
		else
			reset_h();
		if (flags.c)
			set_c();
		else
			reset_c();

		mmu.write_byte(sp, registers.a);
		mmu.write_byte(sp - 1, registers.f);
		sp -= 2;
		++pc;
		cycles = 16;
		break;
	case Instruction::push_bc:
		mmu.write_byte(sp, registers.b);
		mmu.write_byte(sp - 1, registers.c);
		sp -= 2;
		++pc;
		cycles = 16;
		break;
	case Instruction::push_de:
		mmu.write_byte(sp, registers.d);
		mmu.write_byte(sp - 1, registers.e);
		sp -= 2;
		++pc;
		cycles = 16;
		break;
	case Instruction::push_hl:
		mmu.write_byte(sp, registers.h);
		mmu.write_byte(sp - 1, registers.l);
		sp -= 2;
		++pc;
		cycles = 16;
		break;
	case Instruction::pop_af:
		registers.a = mmu.read_byte(sp + 2);
		registers.f = mmu.read_byte(sp + 1);
		// reset our flags
		if (bit_check(registers.f, 7)) {
			flags.z = true;
		}
		else {
			flags.z = false;
		}
		if (bit_check(registers.f, 6))
			flags.n = true;
		else
			flags.n = false;
		if (bit_check(registers.f, 5))
			flags.h = true;
		else
			flags.h = false;
		if (bit_check(registers.f, 4))
			flags.c = true;
		else
			flags.c = false;
		sp += 2;
		++pc;
		cycles = 12;
		break;
	case Instruction::pop_bc:
		registers.b = mmu.read_byte(sp + 2);
		registers.c = mmu.read_byte(sp + 1);
		sp += 2;
		++pc;
		cycles = 12;
		break;
	case Instruction::pop_de:
		registers.d = mmu.read_byte(sp + 2);
		registers.e = mmu.read_byte(sp + 1);
		sp += 2;
		++pc;
		cycles = 12;
		break;
	case Instruction::pop_hl:
		registers.h = mmu.read_byte(sp + 2);
		registers.l = mmu.read_byte(sp + 1);
		sp += 2;
		++pc;
		cycles = 12;
		break;
	case Instruction::ld_hl_sp_R8:
	{
		int8_t offset = mmu.read_byte(pc + 1);
		registers.hl = sp + offset;
		// TODO: Z and H flags
		flags.z = false;
		flags.n = false;
		pc += 2;
		cycles = 12;
		break;
	}
	case Instruction::ld_sp_hl:
		sp = registers.hl;
		cycles = 8;
		++pc;
		break;
		// 8 bit ALU
	case Instruction::add_a_c:
	{
		uint16_t res = registers.a + registers.c;

		if (res == 0)
			flags.z = true;
		else
			flags.z = false;
		if (res > UINT8_MAX)
			flags.c = true;
		else
			flags.c = false;
		// todo: fix half carry flag
		if ((registers.a ^ registers.c ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		flags.n = false;

		registers.a = static_cast<uint8_t>(res);
		pc += 2;
		cycles = 8;
		break;
	}
	case Instruction::add_a_hl:
	{
		uint8_t value = mmu.read_byte(registers.hl);
		uint16_t res = registers.a + value;

		if (res == 0)
			flags.z = true;
		else
			flags.z = false;
		if (res > UINT8_MAX)
			flags.c = true;
		else
			flags.c = false;
		// todo: fix half carry flag
		if ((registers.a ^ value ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		flags.n = false;

		registers.a = static_cast<uint8_t>(res);
		++pc;
		cycles = 8;
		break;
	}
	case Instruction::add_a_d8:
	{
		uint8_t value = mmu.read_byte(pc + 1);
		uint16_t res = registers.a + value;

		if (res == 0)
			flags.z = true;
		else
			flags.z = false;
		if (res > UINT8_MAX)
			flags.c = true;
		else
			flags.c = false;
		// todo: fix half carry flag
		if ((registers.a ^ value ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		flags.n = false;

		registers.a = static_cast<uint8_t>(res);
		pc += 2;
		cycles = 8;
		break;
	}
	case Instruction::adc_a_h:
	{
		uint8_t carry;
		flags.c == true ? carry = 1U : carry = 0U;
		uint16_t res = registers.a + registers.h + carry;

		if (res == 0)
			flags.z = true;
		else
			flags.z = false;
		if (res > UINT8_MAX)
			flags.c = true;
		else
			flags.c = false;
		// todo: fix half carry flag
		if ((registers.a ^ registers.h ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		flags.n = false;

		registers.a = static_cast<uint8_t>(res);
		pc += 2;
		cycles = 8;
		break;
	}
	case Instruction::adc_a_d8:
	{
		uint8_t value = mmu.read_byte(pc + 1);
		uint8_t carry;
		flags.c == true ? carry = 1U : carry = 0U;
		uint16_t res = registers.a + value + carry;

		if (res == 0)
			flags.z = true;
		else
			flags.z = false;
		if (res > UINT8_MAX)
			flags.c = true;
		else
			flags.c = false;
		// todo: fix half carry flag
		if ((registers.a ^ value ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		flags.n = false;

		registers.a = static_cast<uint8_t>(res);
		pc += 2;
		cycles = 8;
		break;
	}
	case Instruction::sbc_a_d8:
	{
		uint8_t value = mmu.read_byte(pc + 1);
		uint8_t carry;
		flags.c == true ? carry = 1U : carry = 0U;
		uint16_t res = registers.a - value - carry;

		res == 0 ? flags.z = true : flags.z = false;
		value > registers.a ? flags.c = true : flags.c = false;
		(registers.a ^ (-value) ^ res) & 0x10 ? flags.h = true : flags.h = false;

		flags.n = true;

		registers.a = static_cast<uint8_t>(res);
		pc += 2;
		cycles = 8;
		break;
	}
	case Instruction::add_a_a:
	{

		uint16_t res = registers.a + registers.a;

		if (registers.a == 0)
			flags.z = true;
		else
			flags.z = false;
		if (res > UINT8_MAX)
			flags.c = true;
		else
			flags.c = false;
		// todo: fix half carry flag
		if ((registers.a ^ registers.a ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		flags.n = false;

		registers.a += registers.a;
		pc += 2;
		cycles = 8;
		break;
	}
	case Instruction::and_d8:
		registers.a &= mmu.read_byte(pc + 1);

		if (registers.a == 0)
			flags.z = true;
		else
			flags.z = false;
		flags.n = false;
		flags.c = false;
		flags.h = true;
		pc += 2;
		cycles = 8;
		break;
	case Instruction::and_a:
		registers.a &= registers.a;

		if (registers.a == 0)
			flags.z = true;
		else
			flags.z = false;
		flags.n = false;
		flags.c = false;
		flags.h = true;
		++pc;
		cycles = 4;
		break;
	case Instruction::and_b:
		registers.a &= registers.b;

		if (registers.a == 0)
			flags.z = true;
		else
			flags.z = false;
		flags.n = false;
		flags.c = false;
		flags.h = true;
		++pc;
		cycles = 4;
		break;
	case Instruction::and_c:
		registers.a &= registers.c;

		if (registers.a == 0)
			flags.z = true;
		else
			flags.z = false;
		flags.n = false;
		flags.c = false;
		flags.h = true;
		++pc;
		cycles = 4;
		break;
	case Instruction::sub_d8:
	{
		uint8_t val = mmu.read_byte(pc + 1);
		uint8_t res = registers.a - val;
		// TODO: set flags
		if (res == 0)
			flags.z = true;
		else
			flags.z = false;
		if (val > registers.a)
			flags.c = true;
		else
			flags.c = false;
		if ((registers.a ^ (-val) ^ res) & 0x10)
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
		if (registers.b > registers.a)
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
	case Instruction::sub_c:
	{
		uint8_t res = registers.a - registers.c;
		if (res == 0)
			flags.z = true;
		else
			flags.z = false;
		if (registers.c > registers.a)
			flags.c = true;
		else
			flags.c = false;
		if ((registers.a ^ (-registers.c) ^ res) & 0x10)
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
	case Instruction::xor_c:
	{
		uint8_t res = registers.a ^ registers.c;
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
	case Instruction::xor_d:
	{
		uint8_t res = registers.a ^ registers.d;
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
	case Instruction::xor_e:
	{
		uint8_t res = registers.a ^ registers.e;
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
	case Instruction::xor_h:
	{
		uint8_t res = registers.a ^ registers.h;
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
	case Instruction::xor_l:
	{
		uint8_t res = registers.a ^ registers.l;
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
	case Instruction::xor_hl:
		registers.a ^= mmu.read_byte(registers.hl);
		if (registers.a == 0)
			flags.z = true;
		else
			flags.z = false;
		flags.c = false;
		flags.h = false;
		flags.n = false;

		++pc;
		cycles = 4;
		break;
	case Instruction::xor_d8:
		registers.a ^= mmu.read_byte(pc + 1);
		if (registers.a == 0)
			flags.z = true;
		else
			flags.z = false;
		flags.c = false;
		flags.h = false;
		flags.n = false;

		pc += 2;
		cycles = 8;
		break;
	case Instruction::or_a:
		registers.a |= registers.a;
		if (registers.a == 0)
			flags.z = true;
		else
			flags.z = false;
		flags.c = false;
		flags.h = false;
		flags.n = false;
		++pc;
		cycles = 4;
		break;
	case Instruction::or_b:
		registers.a |= registers.b;
		if (registers.a == 0)
			flags.z = true;
		else
			flags.z = false;
		flags.c = false;
		flags.h = false;
		flags.n = false;
		++pc;
		cycles = 4;
		break;
	case Instruction::or_c:
		registers.a |= registers.c;
		if (registers.a == 0)
			flags.z = true;
		else
			flags.z = false;
		flags.c = false;
		flags.h = false;
		flags.n = false;
		++pc;
		cycles = 4;
		break;
	case Instruction::or_hl:
		registers.a |= mmu.read_byte(registers.hl);
		if (registers.a == 0)
			flags.z = true;
		else
			flags.z = false;
		flags.c = false;
		flags.h = false;
		flags.n = false;
		++pc;
		cycles = 8;
		break;
	case Instruction::or_d8:
		registers.a |= mmu.read_byte(pc + 1);
		if (registers.a == 0)
			flags.z = true;
		else
			flags.z = false;
		flags.c = false;
		flags.h = false;
		flags.n = false;
		pc += 2;
		cycles = 8;
		break;
	case Instruction::inc_a:
	{
		uint8_t res = registers.a + 1;

		if (res == 0)
			flags.z = true;
		else
			flags.z = false;
		if ((registers.a ^ 1 ^ res) & 0x10)
			flags.h = true;
		else
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

		if (res == 0)
			flags.z = true;
		else
			flags.z = false;
		if ((registers.b ^ 1 ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		flags.n = false;

		registers.b = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::inc_c:
	{
		uint8_t res = registers.c + 1;

		if (res == 0)
			flags.z = true;
		else
			flags.z = false;
		if ((registers.c ^ 1 ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		flags.n = false;

		registers.c = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::inc_d:
	{
		uint8_t res = registers.d + 1;

		if (res == 0)
			flags.z = true;
		else
			flags.z = false;
		if ((registers.d ^ 1 ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		flags.n = false;

		registers.d = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::inc_e:
	{
		uint8_t res = registers.e + 1;

		if (res == 0)
			flags.z = true;
		else
			flags.z = false;
		if ((registers.e ^ 1 ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		flags.n = false;

		registers.e = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::inc_h:
	{
		uint8_t res = registers.h + 1;

		if (res == 0)
			flags.z = true;
		else
			flags.z = false;
		if ((registers.h ^ 1 ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		flags.n = false;

		registers.h = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::inc_l:
	{
		uint8_t res = registers.l + 1;

		if (res == 0)
			flags.z = true;
		else
			flags.z = false;
		if ((registers.l ^ 1 ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		flags.n = false;

		registers.l = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::inc_at_hl:
	{
		uint8_t val = mmu.read_byte(registers.hl);
		uint8_t res = val + 1;
		mmu.write_byte(registers.hl, res);
		if (res == 0)
			flags.z = true;
		else
			flags.z = false;
		if ((val ^ 1 ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		flags.n = false;
		++pc;
		cycles = 12;
		break;
	}
	case Instruction::dec_a:
	{
		uint8_t res = registers.a - 1;

		if (res == 0)
			flags.z = true;
		else
			flags.z = false;
		if ((registers.a ^ (-1) ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		flags.n = true;

		registers.a = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::dec_b:
	{
		uint8_t res = registers.b - 1;

		if (res == 0)
			flags.z = true;
		else
			flags.z = false;
		if ((registers.b ^ (-1) ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		flags.n = true;

		registers.b = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::dec_c:
	{
		uint8_t res = registers.c - 1;

		if (res == 0)
			flags.z = true;
		else
			flags.z = false;
		if ((registers.c ^ (-1) ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		flags.n = true;

		registers.c = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::dec_d:
	{
		uint8_t res = registers.d - 1;

		if (res == 0)
			flags.z = true;
		else
			flags.z = false;
		if ((registers.d ^ (-1) ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		flags.n = true;

		registers.d = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::dec_e:
	{
		uint8_t res = registers.e - 1;

		if (res == 0)
			flags.z = true;
		else
			flags.z = false;
		if ((registers.e ^ (-1) ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		flags.n = true;

		registers.e = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::dec_h:
	{
		uint8_t res = registers.h - 1;

		if (res == 0)
			flags.z = true;
		else
			flags.z = false;
		if ((registers.h ^ (-1) ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		flags.n = true;

		registers.h = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::dec_l:
	{
		uint8_t res = registers.l - 1;

		if (res == 0)
			flags.z = true;
		else
			flags.z = false;
		if ((registers.l ^ (-1) ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		flags.n = true;

		registers.l = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::dec_at_hl:
	{
		uint8_t val = mmu.read_byte(registers.hl);
		uint8_t res = val - 1;
		mmu.write_byte(registers.hl, res);
		if (res == 0)
			flags.z = true;
		else
			flags.z = false;
		if ((val ^ -1 ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		flags.n = false;
		++pc;
		cycles = 12;
		break;
	}
	case Instruction::cp_e:
	{
		uint8_t res = registers.a - registers.e;
		if (res == 0)
			flags.z = true;
		else
			flags.z = false;
		flags.n = true;
		if ((registers.a ^ (-registers.e) ^ res) & 0x10)
			flags.h = true;
		else
			flags.h = false;
		if (registers.e > registers.a)
			flags.c = true;
		else
			flags.c = false;
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
		if (mmu.read_byte(pc + 1) > registers.a)
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
		if (mmu.read_byte(registers.hl) > registers.a)
			flags.c = true;
		else
			flags.c = false;
		++pc;
		cycles = 8;
		break;
	}
	// 16 bit arithmetic
	case Instruction::inc_bc:
		++registers.bc;
		++pc;
		cycles = 8;
		break;
	case Instruction::inc_de:
		++registers.de;
		++pc;
		cycles = 8;
		break;
	case Instruction::inc_hl:
		++registers.hl;
		++pc;
		cycles = 8;
		break;
	case Instruction::dec_bc:
		--registers.bc;
		++pc;
		cycles = 8;
		break;
	case Instruction::add_hl_de:
	{
		uint32_t res = registers.hl + registers.de;

		if (res > UINT16_MAX)
			flags.c = true;
		else
			flags.c = false;

		// TODO: Verify
		if ((registers.hl ^ registers.de ^ res) & 0x1000)
			flags.h = true;
		else
			flags.h = false;
		flags.n = false;

		registers.hl += registers.de;
		++pc;
		cycles = 8;
		break;
	}
	case Instruction::add_hl_hl:
	{
		uint32_t res = registers.hl + registers.hl;

		if (res > UINT16_MAX)
			flags.c = true;
		else
			flags.c = false;

		// TODO: Verify
		if ((registers.hl ^ registers.hl ^ res) & 0x1000)
			flags.h = true;
		else
			flags.h = false;
		flags.n = false;

		registers.hl = static_cast<uint16_t>(res);
		++pc;
		cycles = 8;
		break;
	}
	// Jumps
	case Instruction::jp_a16:
	{
		uint8_t c = mmu.read_byte(pc + 1);
		uint8_t d = mmu.read_byte(pc + 2);
		pc = (d << 8) + c;
		cycles = 16;
		break;
	}
	case Instruction::jp_z_a16:
	{
		if (flags.z) {
			uint8_t c = mmu.read_byte(pc + 1);
			uint8_t d = mmu.read_byte(pc + 2);
			pc = (d << 8) + c;
			cycles = 16;
		}
		else {
			pc += 3;
			cycles = 12;
		}
		break;
	}
	case Instruction::jp_hl:
		pc = registers.hl;
		cycles = 16;
		break;
	case Instruction::jp_nz_a16:
		if (flags.z) {
			pc += 3;
			cycles = 12;
		}
		else {
			pc = (mmu.read_byte(pc + 2) << 8) + mmu.read_byte(pc + 1);
			cycles = 16;
		}
		break;
	case Instruction::jr_r8:
		// must jump from the address of the NEXT instruction (ie 2 bytes after this one)
		pc += (int8_t)(mmu.read_byte(pc + 1) + 2);
		cycles = 12;
		break;
	case Instruction::jr_c_r8:
		if (flags.c) {
			pc += (int8_t)(mmu.read_byte(pc + 1) + 2);
			cycles = 12;
		}
		else {
			pc += 2;
			cycles = 8;
		}

		break;
	case Instruction::jr_nc_r8:
		if (flags.c) {
			pc += 2;
			cycles = 8;
		}
		else {
			auto offset = (int8_t)mmu.read_byte(pc + 1);
			pc += (2 + offset);
			cycles = 12;
		}
		break;
	case Instruction::jr_nz_r8:
		if (flags.z) {
			pc += 2;
			cycles = 8;
		}
		else {
			auto offset = (int8_t)mmu.read_byte(pc + 1);
			pc += (2 + offset);
			cycles = 12;
		}
		break;
	case Instruction::jr_z_r8:
		if (flags.z) {
			auto offset = (int8_t)mmu.read_byte(pc + 1);
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
	case Instruction::rra:
	{
		uint8_t bit = bit_check(registers.a, 0);
		registers.a >>= 1;
		// return the old carry back to bit 7, and put our new bit in the carry
		if (flags.c)
			bit_set(registers.a, 7);
		else
			bit_clear(registers.a, 7);
		bit == 0 ? flags.c = false : flags.c = true;
		flags.h = false;
		flags.n = false;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::rlca:
	{
		uint8_t bit = bit_check(registers.a, 7);
		registers.a <<= 1;
		registers.a ^= (-bit ^ registers.a) & (1U << 0);
		//flags.c = bit;
		bit == 0 ? flags.c = false : flags.c = true;
		flags.z = false;
		flags.n = false;
		flags.h = false;
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
	case Instruction::call_nz_a16:
		if (!flags.z) {
			uint16_t address = (mmu.read_byte(pc + 2) << 8) + mmu.read_byte(pc + 1);
			mmu.write_byte(sp, (pc + 3) >> 8);
			mmu.write_byte(sp - 1, pc + 3);
			sp -= 2;
			cycles = 24;
			pc = address;
		}
		else {
			pc += 3;
			cycles = 12;
		}
		break;
	// Restarts
	case Instruction::rst_28h:
		mmu.write_byte(sp, (pc + 1) >> 8);
		mmu.write_byte(sp - 1, pc + 1);
		sp -= 2;
		pc = 0x0028;
		cycles = 16;
		break;
	// Returns
	case Instruction::ret:
	{
		pc = (mmu.read_byte(sp + 2) << 8) + mmu.read_byte(sp + 1);
		sp += 2;
		cycles = 8;
		break;
	}
	case Instruction::reti:
	{
		pc = (mmu.read_byte(sp + 2) << 8) + mmu.read_byte(sp + 1);
		ime = true;
		sp += 2;
		cycles = 16;
		break;
	}
	case Instruction::ret_z:
	{
		if (flags.z) {
			pc = (mmu.read_byte(sp + 2) << 8) + mmu.read_byte(sp + 1);
			sp += 2;
			cycles = 20;
		}
		else {
			++pc;
			cycles = 8;
		}
		break;
	}
	case Instruction::ret_nz:
	{
		if (!flags.z) {
			pc = (mmu.read_byte(sp + 2) << 8) + mmu.read_byte(sp + 1);
			sp += 2;
			cycles = 20;
		} else {
			++pc;
			cycles = 8;
		}
		break;
	}
	case Instruction::ret_c:
	{
		if (flags.c) {
			pc = (mmu.read_byte(sp + 2) << 8) + mmu.read_byte(sp + 1);
			sp += 2;
			cycles = 20;
		}
		else {
			++pc;
			cycles = 8;
		}
		break;
	}
	case Instruction::ret_nc:
	{
		if (!flags.c) {
			pc = (mmu.read_byte(sp + 2) << 8) + mmu.read_byte(sp + 1);
			sp += 2;
			cycles = 20;
		}
		else {
			++pc;
			cycles = 8;
		}
		break;
	}
	// Misc
	case Instruction::cpl:
		registers.a = ~registers.a;
		flags.n = true;
		flags.h = true;
		++pc;
		break;
	case Instruction::di:
		// TODO: disable interrupts after the next instruction
		ime = false;
		++pc;
		cycles = 4;
		break;
	case Instruction::ei:
		// TODO: enable interrupts after the next instruction
		ime = true;
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