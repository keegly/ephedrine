#include <cstdint>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include "gb.h"
#include "cpu.h"
#include "mmu.h"
#include "bit_utility.h"
#include "instructions.h"
#include "spdlog/spdlog.h"

CPU::CPU(MMU &m) : mmu(m), pc(0x0100), sp(0xFFFE)
{
	registers.af = 0x01B0;
	registers.bc = 0x0013;
	registers.de = 0x00D8;
	registers.hl = 0x014D;
}

void CPU::print()
{
	spdlog::get("stdout")->debug("Registers: af:{0:04x} bc:{1:04x} de:{2:04x} hl:{3:04x}", registers.af, registers.bc, registers.de, registers.hl);
	spdlog::get("stdout")->debug("pc: {0:04x} sp: {1:x}", pc, sp);
	spdlog::get("stdout")->debug("Z: {0} C: {1}", flags.z, flags.c);

}

void CPU::handle_interrupts()
{
	uint8_t if_reg = mmu.read_byte(IF);
	uint8_t ie_reg = mmu.read_byte(IE);

	if (!ime || (if_reg & 0x1F) == 0 || (ie_reg & 0x1F) == 0)
		return; // interrupts globally disabled or nothing to handle
	// check register 0xFF0F to see which interrupt was generated
	constexpr uint16_t offset[]{ 0x0040, 0x0048, 0x0050, 0x0058, 0x0060 };
	uint16_t address = 0x0000;
	for (uint8_t i = 0; i < 5; ++i) {
		if (bit_check(if_reg, i) /*&& bit_check(ie_reg, i)*/)  {
			// put current pc on stack and head to the proper service routine
			////spdlog::get("stdout")->debug("int handler: {0:04X}, jumping from {1:04x}", offset[i], pc);
			mmu.write_byte(--sp, pc >> 8);
			mmu.write_byte(--sp, static_cast<uint8_t>(pc));
			pc = offset[i];
			// clear teh bit
			bit_clear(if_reg, i);
			mmu.write_byte(IF, if_reg);
			return;
		}
	}
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
	//spdlog::get("file logger")->info("pc: {0:04X} - op: {1:02X}", pc, (uint8_t)opcode);
	switch (opcode) {
	// CB prefixed opcodes
	case Instruction::prefix_cb:
	{
		switch (Prefix_CB(mmu.read_byte(pc + 1))) {
		case Prefix_CB::rlc_b:
			rlc(registers.b);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::rlc_c:
			rlc(registers.c);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::rlc_d:
			rlc(registers.d);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::rlc_e:
			rlc(registers.e);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::rlc_h:
			rlc(registers.h);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::rlc_l:
			rlc(registers.l);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::rlc_hl:
		{
			uint8_t val = mmu.read_byte(registers.hl);
			rlc(val);
			mmu.write_byte(registers.hl, val);
			pc += 2;
			cycles = 16;
			break;
		}
		case Prefix_CB::rlc_a:
			rlc(registers.a);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::rrc_b:
			rrc(registers.b);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::rrc_c:
			rrc(registers.c);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::rrc_d:
			rrc(registers.d);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::rrc_e:
			rrc(registers.e);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::rrc_h:
			rrc(registers.h);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::rrc_l:
			rrc(registers.l);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::rrc_hl:
		{
			uint8_t val = mmu.read_byte(registers.hl);
			rrc(val);
			mmu.write_byte(registers.hl, val);
			pc += 2;
			cycles = 16;
			break;
		}
		case Prefix_CB::rrc_a:
			rrc(registers.a);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::rl_b:
			rl(registers.b);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::rl_c:
			rl(registers.c);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::rl_d:
			rl(registers.d);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::rl_e:
			rl(registers.e);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::rl_h:
			rl(registers.h);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::rl_l:
			rl(registers.l);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::rl_hl:
		{
			uint8_t val = mmu.read_byte(registers.hl);
			rl(val);
			mmu.write_byte(registers.hl, val);
			pc += 2;
			cycles = 16;
			break;
		}
		case Prefix_CB::rl_a:
			rl(registers.a);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::rr_b:
			rr(registers.b);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::rr_c:
			rr(registers.c);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::rr_d:
			rr(registers.d);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::rr_e:
			rr(registers.e);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::rr_h:
			rr(registers.h);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::rr_l:
			rr(registers.l);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::rr_hl:
		{
			uint8_t val = mmu.read_byte(registers.hl);
			rr(val);
			mmu.write_byte(registers.hl, val);
			pc += 2;
			cycles = 16;
			break;
		}
		case Prefix_CB::rr_a:
			rr(registers.a);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::sla_b:
			sla(registers.b);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::sla_c:
			sla(registers.c);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::sla_d:
			sla(registers.d);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::sla_e:
			sla(registers.e);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::sla_h:
			sla(registers.h);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::sla_l:
			sla(registers.l);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::sla_hl:
		{
			uint8_t val = mmu.read_byte(registers.hl);
			sla(val);
			mmu.write_byte(registers.hl, val);
			pc += 2;
			cycles = 16;
			break;
		}
		case Prefix_CB::sla_a:
			sla(registers.a);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::sra_b:
			sra(registers.b);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::sra_c:
			sra(registers.c);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::sra_d:
			sra(registers.d);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::sra_e:
			sra(registers.e);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::sra_h:
			sra(registers.h);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::sra_l:
			sra(registers.l);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::sra_hl:
		{
			uint8_t val = mmu.read_byte(registers.hl);
			sra(val);
			mmu.write_byte(registers.hl, val);
			pc += 2;
			cycles = 16;
			break;
		}
		case Prefix_CB::sra_a:
			sra(registers.a);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::swap_b:
			swap(registers.b);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::swap_c:
			swap(registers.c);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::swap_d:
			swap(registers.d);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::swap_e:
			swap(registers.e);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::swap_h:
			swap(registers.h);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::swap_l:
			swap(registers.l);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::swap_hl:
		{
			uint8_t val = mmu.read_byte(registers.hl);
			swap(val);
			mmu.write_byte(registers.hl, val);
			pc += 2;
			cycles = 16;
			break;
		}
		case Prefix_CB::swap_a:
			swap(registers.a);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::srl_b:
			srl(registers.b);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::srl_c:
			srl(registers.c);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::srl_d:
			srl(registers.d);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::srl_e:
			srl(registers.e);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::srl_h:
			srl(registers.h);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::srl_l:
			srl(registers.l);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::srl_hl:
		{
			uint8_t val = mmu.read_byte(registers.hl);
			srl(val);
			mmu.write_byte(registers.hl, val);
			pc += 2;
			cycles = 16;
			break;
		}
		case Prefix_CB::srl_a:
			srl(registers.a);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_0_b:
			flags.z = !bit_check(registers.b, 0);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_0_c:
			flags.z = !bit_check(registers.c, 0);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_0_d:
			flags.z = !bit_check(registers.d, 0);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_0_e:
			flags.z = !bit_check(registers.e, 0);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_0_h:
			flags.z = !bit_check(registers.h, 0);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_0_l:
			flags.z = !bit_check(registers.l, 0);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_0_hl:
			flags.z = !bit_check(mmu.read_byte(registers.hl), 0);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_0_a:
			flags.z = !bit_check(registers.a, 0);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_1_b:
			flags.z = !bit_check(registers.b, 1);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_1_c:
			flags.z = !bit_check(registers.c, 1);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_1_d:
			flags.z = !bit_check(registers.d, 1);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_1_e:
			flags.z = !bit_check(registers.e, 1);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_1_h:
			flags.z = !bit_check(registers.h, 1);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_1_l:
			flags.z = !bit_check(registers.l, 1);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_1_hl:
			flags.z = !bit_check(mmu.read_byte(registers.hl), 1);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_1_a:
			flags.z = !bit_check(registers.a, 1);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_2_b:
			flags.z = !bit_check(registers.b, 2);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_2_c:
			flags.z = !bit_check(registers.c, 2);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_2_d:
			flags.z = !bit_check(registers.d, 2);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_2_e:
			flags.z = !bit_check(registers.e, 2);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_2_h:
			flags.z = !bit_check(registers.h, 2);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_2_l:
			flags.z = !bit_check(registers.l, 2);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_2_hl:
			flags.z = !bit_check(mmu.read_byte(registers.hl), 2);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_2_a:
			flags.z = !bit_check(registers.a, 2);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_3_b:
			flags.z = !bit_check(registers.b, 3);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_3_c:
			flags.z = !bit_check(registers.c, 3);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_3_d:
			flags.z = !bit_check(registers.d, 3);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_3_e:
			flags.z = !bit_check(registers.e, 3);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_3_h:
			flags.z = !bit_check(registers.h, 3);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_3_l:
			flags.z = !bit_check(registers.l, 3);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_3_hl:
			flags.z = !bit_check(mmu.read_byte(registers.hl), 3);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_3_a:
			flags.z = !bit_check(registers.a, 3);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_4_b:
			flags.z = !bit_check(registers.b, 4);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_4_c:
			flags.z = !bit_check(registers.c, 4);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_4_d:
			flags.z = !bit_check(registers.d, 4);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_4_e:
			flags.z = !bit_check(registers.e, 4);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_4_h:
			flags.z = !bit_check(registers.h, 4);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_4_l:
			flags.z = !bit_check(registers.l, 4);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_4_hl:
			flags.z = !bit_check(mmu.read_byte(registers.hl), 4);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_4_a:
			flags.z = !bit_check(registers.a, 4);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_5_b:
			flags.z = !bit_check(registers.b, 5);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_5_c:
			flags.z = !bit_check(registers.c, 5);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_5_d:
			flags.z = !bit_check(registers.d, 5);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_5_e:
			flags.z = !bit_check(registers.e, 5);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_5_h:
			flags.z = !bit_check(registers.h, 5);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_5_l:
			flags.z = !bit_check(registers.l, 5);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_5_hl:
			flags.z = !bit_check(mmu.read_byte(registers.hl), 5);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_5_a:
			flags.z = !bit_check(registers.a, 5);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_6_b:
			flags.z = !bit_check(registers.b, 6);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_6_c:
			flags.z = !bit_check(registers.c, 6);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_6_d:
			flags.z = !bit_check(registers.d, 6);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_6_e:
			flags.z = !bit_check(registers.e, 6);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_6_h:
			flags.z = !bit_check(registers.h, 6);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_6_l:
			flags.z = !bit_check(registers.l, 6);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_6_hl:
			flags.z = !bit_check(mmu.read_byte(registers.hl), 6);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_6_a:
			flags.z = !bit_check(registers.a, 6);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_7_b:
			flags.z = !bit_check(registers.b, 7);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_7_c:
			flags.z = !bit_check(registers.c, 7);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_7_d:
			flags.z = !bit_check(registers.d, 7);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_7_e:
			flags.z = !bit_check(registers.e, 7);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_7_h:
			flags.z = !bit_check(registers.h, 7);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_7_l:
			flags.z = !bit_check(registers.l, 7);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_7_hl:
			flags.z = !bit_check(mmu.read_byte(registers.hl), 7);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::bit_7_a:
			flags.z = !bit_check(registers.a, 7);
			flags.n = false;
			flags.h = true;
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_0_b:
			bit_clear(registers.b, 0);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_0_c:
			bit_clear(registers.c, 0);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_0_d:
			bit_clear(registers.d, 0);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_0_e:
			bit_clear(registers.e, 0);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_0_h:
			bit_clear(registers.h, 0);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_0_l:
			bit_clear(registers.l, 0);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_0_hl:
		{
			uint8_t val = mmu.read_byte(registers.hl);
			bit_clear(val, 0);
			mmu.write_byte(registers.hl, val);
			pc += 2;
			cycles = 16;
			break;
		}
		case Prefix_CB::res_0_a:
			bit_clear(registers.a, 0);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_1_b:
			bit_clear(registers.b, 1);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_1_c:
			bit_clear(registers.c, 1);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_1_d:
			bit_clear(registers.d, 1);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_1_e:
			bit_clear(registers.e, 1);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_1_h:
			bit_clear(registers.h, 1);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_1_l:
			bit_clear(registers.l, 1);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_1_hl:
		{
			uint8_t val = mmu.read_byte(registers.hl);
			bit_clear(val, 1);
			mmu.write_byte(registers.hl, val);
			pc += 2;
			cycles = 16;
			break;
		}
		case Prefix_CB::res_1_a:
			bit_clear(registers.a, 1);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_2_b:
			bit_clear(registers.b, 2);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_2_c:
			bit_clear(registers.c, 2);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_2_d:
			bit_clear(registers.d, 2);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_2_e:
			bit_clear(registers.e, 2);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_2_h:
			bit_clear(registers.h, 2);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_2_l:
			bit_clear(registers.l, 2);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_2_hl:
		{
			uint8_t val = mmu.read_byte(registers.hl);
			bit_clear(val, 2);
			mmu.write_byte(registers.hl, val);
			pc += 2;
			cycles = 16;
			break;
		}
		case Prefix_CB::res_2_a:
			bit_clear(registers.a, 2);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_3_b:
			bit_clear(registers.b, 3);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_3_c:
			bit_clear(registers.c, 3);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_3_d:
			bit_clear(registers.d, 3);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_3_e:
			bit_clear(registers.e, 3);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_3_h:
			bit_clear(registers.h, 3);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_3_l:
			bit_clear(registers.l, 3);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_3_hl:
		{
			uint8_t val = mmu.read_byte(registers.hl);
			bit_clear(val, 3);
			mmu.write_byte(registers.hl, val);
			pc += 2;
			cycles = 16;
			break;
		}
		case Prefix_CB::res_3_a:
			bit_clear(registers.a, 3);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_4_b:
			bit_clear(registers.b, 4);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_4_c:
			bit_clear(registers.c, 4);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_4_d:
			bit_clear(registers.d, 4);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_4_e:
			bit_clear(registers.e, 4);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_4_h:
			bit_clear(registers.h, 4);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_4_l:
			bit_clear(registers.l, 4);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_4_hl:
		{
			uint8_t val = mmu.read_byte(registers.hl);
			bit_clear(val, 4);
			mmu.write_byte(registers.hl, val);
			pc += 2;
			cycles = 16;
			break;
		}
		case Prefix_CB::res_4_a:
			bit_clear(registers.a, 4);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_5_b:
			bit_clear(registers.b, 5);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_5_c:
			bit_clear(registers.c, 5);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_5_d:
			bit_clear(registers.d, 5);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_5_e:
			bit_clear(registers.e, 5);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_5_h:
			bit_clear(registers.h, 5);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_5_l:
			bit_clear(registers.l, 5);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_5_hl:
		{
			uint8_t val = mmu.read_byte(registers.hl);
			bit_clear(val, 5);
			mmu.write_byte(registers.hl, val);
			pc += 2;
			cycles = 16;
			break;
		}
		case Prefix_CB::res_5_a:
			bit_clear(registers.a, 5);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_6_b:
			bit_clear(registers.b, 6);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_6_c:
			bit_clear(registers.c, 6);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_6_d:
			bit_clear(registers.d, 6);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_6_e:
			bit_clear(registers.e, 6);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_6_h:
			bit_clear(registers.h, 6);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_6_l:
			bit_clear(registers.l, 6);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_6_hl:
		{
			uint8_t val = mmu.read_byte(registers.hl);
			bit_clear(val, 6);
			mmu.write_byte(registers.hl, val);
			pc += 2;
			cycles = 16;
			break;
		}
		case Prefix_CB::res_6_a:
			bit_clear(registers.a, 6);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_7_b:
			bit_clear(registers.b, 7);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_7_c:
			bit_clear(registers.c, 7);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_7_d:
			bit_clear(registers.d, 7);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_7_e:
			bit_clear(registers.e, 7);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_7_h:
			bit_clear(registers.h, 7);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_7_l:
			bit_clear(registers.l, 7);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::res_7_hl:
		{
			uint8_t val = mmu.read_byte(registers.hl);
			bit_clear(val, 7);
			mmu.write_byte(registers.hl, val);
			pc += 2;
			cycles = 16;
			break;
		}
		case Prefix_CB::res_7_a:
			bit_clear(registers.a, 7);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_0_b:
			bit_set(registers.b, 0);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_0_c:
			bit_set(registers.c, 0);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_0_d:
			bit_set(registers.d, 0);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_0_e:
			bit_set(registers.e, 0);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_0_h:
			bit_set(registers.h, 0);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_0_l:
			bit_set(registers.l, 0);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_0_hl:
		{
			uint8_t val = mmu.read_byte(registers.hl);
			bit_set(val, 0);
			mmu.write_byte(registers.hl, val);
			pc += 2;
			cycles = 16;
			break;
		}
		case Prefix_CB::set_0_a:
			bit_set(registers.a, 0);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_1_b:
			bit_set(registers.b, 1);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_1_c:
			bit_set(registers.c, 1);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_1_d:
			bit_set(registers.d, 1);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_1_e:
			bit_set(registers.e, 1);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_1_h:
			bit_set(registers.h, 1);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_1_l:
			bit_set(registers.l, 1);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_1_hl:
		{
			uint8_t val = mmu.read_byte(registers.hl);
			bit_set(val, 1);
			mmu.write_byte(registers.hl, val);
			pc += 2;
			cycles = 16;
			break;
		}
		case Prefix_CB::set_1_a:
			bit_set(registers.a, 1);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_2_b:
			bit_set(registers.b, 2);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_2_c:
			bit_set(registers.c, 2);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_2_d:
			bit_set(registers.d, 2);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_2_e:
			bit_set(registers.e, 2);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_2_h:
			bit_set(registers.h, 2);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_2_l:
			bit_set(registers.l, 2);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_2_hl:
		{
			uint8_t val = mmu.read_byte(registers.hl);
			bit_set(val, 2);
			mmu.write_byte(registers.hl, val);
			pc += 2;
			cycles = 16;
			break;
		}
		case Prefix_CB::set_2_a:
			bit_set(registers.a, 2);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_3_b:
			bit_set(registers.b, 3);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_3_c:
			bit_set(registers.c, 3);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_3_d:
			bit_set(registers.d, 3);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_3_e:
			bit_set(registers.e, 3);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_3_h:
			bit_set(registers.h, 3);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_3_l:
			bit_set(registers.l, 3);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_3_hl:
		{
			uint8_t val = mmu.read_byte(registers.hl);
			bit_set(val, 3);
			mmu.write_byte(registers.hl, val);
			pc += 2;
			cycles = 16;
			break;
		}
		case Prefix_CB::set_3_a:
			bit_set(registers.a, 3);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_4_b:
			bit_set(registers.b, 4);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_4_c:
			bit_set(registers.c, 4);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_4_d:
			bit_set(registers.d, 4);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_4_e:
			bit_set(registers.e, 4);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_4_h:
			bit_set(registers.h, 4);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_4_l:
			bit_set(registers.l, 4);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_4_hl:
		{
			uint8_t val = mmu.read_byte(registers.hl);
			bit_set(val, 4);
			mmu.write_byte(registers.hl, val);
			pc += 2;
			cycles = 16;
			break;
		}
		case Prefix_CB::set_4_a:
			bit_set(registers.a, 4);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_5_b:
			bit_set(registers.b, 5);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_5_c:
			bit_set(registers.c, 5);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_5_d:
			bit_set(registers.d, 5);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_5_e:
			bit_set(registers.e, 5);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_5_h:
			bit_set(registers.h, 5);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_5_l:
			bit_set(registers.l, 5);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_5_hl:
		{
			uint8_t val = mmu.read_byte(registers.hl);
			bit_set(val, 5);
			mmu.write_byte(registers.hl, val);
			pc += 2;
			cycles = 16;
			break;
		}
		case Prefix_CB::set_5_a:
			bit_set(registers.a, 5);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_6_b:
			bit_set(registers.b, 6);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_6_c:
			bit_set(registers.c, 6);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_6_d:
			bit_set(registers.d, 6);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_6_e:
			bit_set(registers.e, 6);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_6_h:
			bit_set(registers.h, 6);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_6_l:
			bit_set(registers.l, 6);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_6_hl:
		{
			uint8_t val = mmu.read_byte(registers.hl);
			bit_set(val, 6);
			mmu.write_byte(registers.hl, val);
			pc += 2;
			cycles = 16;
			break;
		}
		case Prefix_CB::set_6_a:
			bit_set(registers.a, 6);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_7_b:
			bit_set(registers.b, 7);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_7_c:
			bit_set(registers.c, 7);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_7_d:
			bit_set(registers.d, 7);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_7_e:
			bit_set(registers.e, 7);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_7_h:
			bit_set(registers.h, 7);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_7_l:
			bit_set(registers.l, 7);
			pc += 2;
			cycles = 8;
			break;
		case Prefix_CB::set_7_hl:
		{
			uint8_t val = mmu.read_byte(registers.hl);
			bit_set(val, 7);
			mmu.write_byte(registers.hl, val);
			pc += 2;
			cycles = 16;
			break;
		}
		case Prefix_CB::set_7_a:
			bit_set(registers.a, 7);
			pc += 2;
			cycles = 8;
			break;
		default:
			//printf("Unknown opcode: 0x%0.2X 0x%0.2X\n", opcode, opcode2);
			spdlog::get("stdout")->error("Unknown opcode 0x{0:02X} 0x{1:02X} at 0x{2:04X}", (uint8_t)opcode, (uint8_t)mmu.read_byte(pc + 1), pc);
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
	case Instruction::ld_bc_a:
		mmu.write_byte(registers.bc, registers.a);
		++pc;
		cycles = 8;
		break;
	case Instruction::ld_de_a:
		mmu.write_byte(registers.de, registers.a);
		++pc;
		cycles = 8;
		break;
	case Instruction::ld_hl_a:
		mmu.write_byte(registers.hl, registers.a);
		++pc;
		cycles = 8;
		break;
	case Instruction::ld_hl_b:
		mmu.write_byte(registers.hl, registers.b);
		++pc;
		cycles = 8;
		break;
	case Instruction::ld_hl_c:
		mmu.write_byte(registers.hl, registers.c);
		++pc;
		cycles = 8;
		break;
	case Instruction::ld_hl_d:
		mmu.write_byte(registers.hl, registers.d);
		++pc;
		cycles = 8;
		break;
	case Instruction::ld_hl_e:
		mmu.write_byte(registers.hl, registers.e);
		++pc;
		cycles = 8;
		break;
	case Instruction::ld_hl_h:
		mmu.write_byte(registers.hl, registers.h);
		++pc;
		cycles = 8;
		break;
	case Instruction::ld_hl_l:
		mmu.write_byte(registers.hl, registers.l);
		++pc;
		cycles = 8;
		break;
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
		mmu.write_byte(registers.hl, registers.a);
		++registers.hl;
		++pc;
		cycles = 8;
		break;
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
	case Instruction::ldd_a_hl:
		registers.a = mmu.read_byte(registers.hl--);
		++pc;
		cycles = 8;
		break;
		// 16 bit loads
	case Instruction::ld_a16_sp:
	{
		uint16_t address = (mmu.read_byte(pc + 2) << 8) + mmu.read_byte(pc + 1);
		mmu.write_byte(address, static_cast<uint8_t>(sp));  // LSB
		mmu.write_byte(address + 1, sp >> 8);				// MSB
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
		flags.z ? set_z() : reset_z();
		flags.n ? set_n() : reset_n();
		flags.h ? set_h() : reset_h();
		flags.c ? set_c() : reset_c();
		// reset unused bits 0 - 3
		bitmask_clear(registers.f, 0x0F);

		sp -= 1;
		mmu.write_byte(sp, registers.a);
		sp -= 1;
		mmu.write_byte(sp, registers.f);
		++pc;
		cycles = 16;
		break;
	case Instruction::push_bc:
		sp -= 1;
		mmu.write_byte(sp, registers.b);
		sp -= 1;
		mmu.write_byte(sp, registers.c);
		++pc;
		cycles = 16;
		break;
	case Instruction::push_de:
		sp -= 1;
		mmu.write_byte(sp, registers.d);
		sp -= 1;
		mmu.write_byte(sp, registers.e);
		++pc;
		cycles = 16;
		break;
	case Instruction::push_hl:
		sp -= 1;
		mmu.write_byte(sp, registers.h);
		sp -= 1;
		mmu.write_byte(sp, registers.l);
		++pc;
		cycles = 16;
		break;
	case Instruction::pop_af:
		registers.f = mmu.read_byte(sp);
		++sp;
		registers.a = mmu.read_byte(sp);
		++sp;
		// reset our flags
		bit_check(registers.f, 7) ? flags.z = true : flags.z = false;
		bit_check(registers.f, 6) ? flags.n = true : flags.n = false;
		bit_check(registers.f, 5) ? flags.h = true : flags.h = false;
		bit_check(registers.f, 4) ? flags.c = true : flags.c = false;

		++pc;
		cycles = 12;
		break;
	case Instruction::pop_bc:
		registers.c = mmu.read_byte(sp);
		++sp;
		registers.b = mmu.read_byte(sp);
		++sp;
		++pc;
		cycles = 12;
		break;
	case Instruction::pop_de:
		registers.e = mmu.read_byte(sp);
		++sp;
		registers.d = mmu.read_byte(sp);
		++sp;
		++pc;
		cycles = 12;
		break;
	case Instruction::pop_hl:
		registers.l = mmu.read_byte(sp++);
		registers.h = mmu.read_byte(sp++);
		++pc;
		cycles = 12;
		break;
	case Instruction::ld_hl_sp_R8:
	{
		// Add the signed value R8 to SP and store the result in HL.
		int8_t offset = mmu.read_byte(pc + 1);
		registers.hl = sp + offset;
		// carry and half carry are computed on the lower 8 bits
		// #TODO: verify carry!
		(sp ^ offset ^ registers.hl) & 0x10 ? flags.h = true : flags.h = false;
		(sp ^ offset ^ registers.hl) & 0x100 ? flags.c = true : flags.c = false;
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
	case Instruction::add_a_a:
	{
		uint16_t res = registers.a + registers.a;

		(res & 0xFF) == 0 ? flags.z = true : flags.z = false;
		res > UINT8_MAX ? flags.c = true : flags.c = false;
		(registers.a ^ registers.a ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = false;

		registers.a = static_cast<uint8_t>(res);
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::add_a_b:
	{
		uint16_t res = registers.a + registers.b;

		(res & 0xFF) == 0 ? flags.z = true : flags.z = false;
		res > UINT8_MAX ? flags.c = true : flags.c = false;
		(registers.a ^ registers.b ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = false;

		registers.a = static_cast<uint8_t>(res);
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::add_a_c:
	{
		uint16_t res = registers.a + registers.c;

		(res & 0xFF) == 0 ? flags.z = true : flags.z = false;
		res > UINT8_MAX ? flags.c = true : flags.c = false;
		(registers.a ^ registers.c ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = false;

		registers.a = static_cast<uint8_t>(res);
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::add_a_d:
	{
		uint16_t res = registers.a + registers.d;

		(res & 0xFF) == 0 ? flags.z = true : flags.z = false;
		res > UINT8_MAX ? flags.c = true : flags.c = false;
		(registers.a ^ registers.d ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = false;

		registers.a = static_cast<uint8_t>(res);
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::add_a_e:
	{
		uint16_t res = registers.a + registers.e;

		(res & 0xFF) == 0 ? flags.z = true : flags.z = false;
		res > UINT8_MAX ? flags.c = true : flags.c = false;
		(registers.a ^ registers.e ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = false;

		registers.a = static_cast<uint8_t>(res);
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::add_a_h:
	{
		uint16_t res = registers.a + registers.h;

		(res & 0xFF) == 0 ? flags.z = true : flags.z = false;
		res > UINT8_MAX ? flags.c = true : flags.c = false;
		(registers.a ^ registers.h ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = false;

		registers.a = static_cast<uint8_t>(res);
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::add_a_l:
	{
		uint16_t res = registers.a + registers.l;

		(res & 0xFF) == 0 ? flags.z = true : flags.z = false;
		res > UINT8_MAX ? flags.c = true : flags.c = false;
		(registers.a ^ registers.l ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = false;

		registers.a = static_cast<uint8_t>(res);
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::add_a_hl:
	{
		uint8_t value = mmu.read_byte(registers.hl);
		uint16_t res = registers.a + value;

		(res & 0xFF) == 0 ? flags.z = true : flags.z = false;
		res > UINT8_MAX ? flags.c = true : flags.c = false;
		(registers.a ^ value ^ res) & 0x10 ? flags.h = true : flags.h = false;
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

		// test only the lower 8 bits, otherwise we'll get a wrong result if
		// there's a carry. This applies to every instruction we used a 16bit
		// container for the 8 bit result
		(res & 0xFF) == 0 ? flags.z = true : flags.z = false;
		res > UINT8_MAX ? flags.c = true : flags.c = false;
		(registers.a ^ value ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = false;

		registers.a = static_cast<uint8_t>(res);
		pc += 2;
		cycles = 8;
		break;
	}
	case Instruction::add_sp_r8:
	{
		int8_t value = mmu.read_byte(pc + 1);
		uint16_t res = sp + value;

		//(res & 0xFF) == 0 ? flags.z = true : flags.z = false;
		flags.z = false;
		(sp ^ value ^ res) & 0x100 ? flags.c = true : flags.c = false; // bit 7
		(sp ^ value ^ res) & 0x10 ? flags.h = true : flags.h = false; // bit 3
		flags.n = false;

		sp = res;
		pc += 2;
		cycles = 16;
		break;
	}
	case Instruction::adc_a_a:
	{
		uint8_t carry;
		flags.c == true ? carry = 1U : carry = 0U;
		uint16_t res = registers.a + registers.a + flags.c;

		(res & 0xFF) == 0 ? flags.z = true : flags.z = false;
		res > UINT8_MAX ? flags.c = true : flags.c = false;
		// #TODO: Verify
		(registers.a ^ registers.a ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = false;

		registers.a = static_cast<uint8_t>(res);
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::adc_a_b:
	{
		uint8_t carry;
		flags.c == true ? carry = 1U : carry = 0U;
		uint16_t res = registers.a + registers.b + carry;

		(res & 0xFF) == 0 ? flags.z = true : flags.z = false;
		res > UINT8_MAX ? flags.c = true : flags.c = false;
		// #TODO: Verify
		(registers.a ^ registers.b ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = false;

		registers.a = static_cast<uint8_t>(res);
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::adc_a_c:
	{
		uint8_t carry;
		flags.c == true ? carry = 1U : carry = 0U;
		uint16_t res = registers.a + registers.c + carry;

		(res & 0xFF) == 0 ? flags.z = true : flags.z = false;
		res > UINT8_MAX ? flags.c = true : flags.c = false;
		// #TODO: Verify
		(registers.a ^ registers.c ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = false;

		registers.a = static_cast<uint8_t>(res);
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::adc_a_d:
	{
		uint8_t carry;
		flags.c == true ? carry = 1U : carry = 0U;
		uint16_t res = registers.a + registers.d + carry;

		(res & 0xFF) == 0 ? flags.z = true : flags.z = false;
		res > UINT8_MAX ? flags.c = true : flags.c = false;
		// #TODO: Verify
		(registers.a ^ registers.d ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = false;

		registers.a = static_cast<uint8_t>(res);
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::adc_a_e:
	{
		uint8_t carry;
		flags.c == true ? carry = 1U : carry = 0U;
		uint16_t res = registers.a + registers.e + carry;

		(res & 0xFF) == 0 ? flags.z = true : flags.z = false;
		res > UINT8_MAX ? flags.c = true : flags.c = false;
		// #TODO: Verify
		(registers.a ^ registers.e ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = false;

		registers.a = static_cast<uint8_t>(res);
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::adc_a_h:
	{
		uint8_t carry;
		flags.c == true ? carry = 1U : carry = 0U;
		uint16_t res = registers.a + registers.h + carry;

		(res & 0xFF) == 0 ? flags.z = true : flags.z = false;
		res > UINT8_MAX ? flags.c = true : flags.c = false;
		// #TODO: Verify
		(registers.a ^ registers.h ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = false;

		registers.a = static_cast<uint8_t>(res);
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::adc_a_hl:
	{
		uint8_t value = mmu.read_byte(registers.hl);
		uint8_t carry;
		flags.c == true ? carry = 1U : carry = 0U;
		uint16_t res = registers.a + value + carry;

		(res & 0xFF) == 0 ? flags.z = true : flags.z = false;
		res > UINT8_MAX ? flags.c = true : flags.c = false;
		// #TODO: Verify
		(registers.a ^ value ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = false;

		registers.a = static_cast<uint8_t>(res);
		++pc;
		cycles = 8;
		break;
	}
	case Instruction::adc_a_l:
	{
		uint8_t carry;
		flags.c == true ? carry = 1U : carry = 0U;
		uint16_t res = registers.a + registers.l + carry;

		(res & 0xFF) == 0 ? flags.z = true : flags.z = false;
		res > UINT8_MAX ? flags.c = true : flags.c = false;
		// #TODO: Verify
		(registers.a ^ registers.l ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = false;

		registers.a = static_cast<uint8_t>(res);
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::adc_a_d8:
	{
		uint8_t value = mmu.read_byte(pc + 1);
		uint8_t carry;
		flags.c == true ? carry = 1U : carry = 0U;
		uint16_t res = registers.a + value + carry;

		(res & 0xFF) == 0 ? flags.z = true : flags.z = false;
		res > UINT8_MAX ? flags.c = true : flags.c = false;
		(registers.a ^ value ^ res) & 0x10 ? flags.h = true : flags.h = false;
		// #TODO: fix/verify half carry flag
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
		uint8_t res = registers.a - value - carry;

		res == 0 ? flags.z = true : flags.z = false;
		value + carry > registers.a ? flags.c = true : flags.c = false;
		(registers.a ^ value ^ res) & 0x10 ? flags.h = true : flags.h = false;

		flags.n = true;

		registers.a = res;
		pc += 2;
		cycles = 8;
		break;
	}
	case Instruction::sbc_a_a:
	{
		uint8_t carry;
		flags.c == true ? carry = 1U : carry = 0U;
		uint8_t res = registers.a - registers.a - carry;

		res == 0 ? flags.z = true : flags.z = false;
		registers.a + carry > registers.a ? flags.c = true : flags.c = false;
		(registers.a ^ registers.a ^ res) & 0x10 ? flags.h = true : flags.h = false;

		flags.n = true;

		registers.a = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::sbc_a_b:
	{
		uint8_t carry;
		flags.c == true ? carry = 1U : carry = 0U;
		uint8_t res = registers.a - registers.b - carry;

		res == 0 ? flags.z = true : flags.z = false;
		registers.b + carry > registers.a ? flags.c = true : flags.c = false;
		(registers.a ^ registers.b ^ res) & 0x10 ? flags.h = true : flags.h = false;

		flags.n = true;

		registers.a = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::sbc_a_c:
	{
		uint8_t carry;
		flags.c == true ? carry = 1U : carry = 0U;
		uint8_t res = registers.a - registers.c - carry;

		res == 0 ? flags.z = true : flags.z = false;
		registers.c + carry > registers.a ? flags.c = true : flags.c = false;
		(registers.a ^ registers.c ^ res) & 0x10 ? flags.h = true : flags.h = false;

		flags.n = true;

		registers.a = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::sbc_a_d:
	{
		uint8_t carry;
		flags.c == true ? carry = 1U : carry = 0U;
		uint8_t res = registers.a - registers.d - carry;

		res == 0 ? flags.z = true : flags.z = false;
		registers.d + carry > registers.a ? flags.c = true : flags.c = false;
		(registers.a ^ registers.d ^ res) & 0x10 ? flags.h = true : flags.h = false;

		flags.n = true;

		registers.a = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::sbc_a_e:
	{
		uint8_t carry;
		flags.c == true ? carry = 1U : carry = 0U;
		uint8_t res = registers.a - registers.e - carry;

		res == 0 ? flags.z = true : flags.z = false;
		registers.e + carry > registers.a ? flags.c = true : flags.c = false;
		(registers.a ^ registers.e ^ res) & 0x10 ? flags.h = true : flags.h = false;

		flags.n = true;

		registers.a = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::sbc_a_h:
	{
		uint8_t carry;
		flags.c == true ? carry = 1U : carry = 0U;
		uint8_t res = registers.a - registers.h - carry;

		res == 0 ? flags.z = true : flags.z = false;
		registers.h + carry > registers.a ? flags.c = true : flags.c = false;
		(registers.a ^ registers.h ^ res) & 0x10 ? flags.h = true : flags.h = false;

		flags.n = true;

		registers.a = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::sbc_a_l:
	{
		uint8_t carry;
		flags.c == true ? carry = 1U : carry = 0U;
		uint8_t res = registers.a - registers.l - carry;

		res == 0 ? flags.z = true : flags.z = false;
		registers.l + carry > registers.a ? flags.c = true : flags.c = false;
		(registers.a ^ registers.l ^ res) & 0x10 ? flags.h = true : flags.h = false;

		flags.n = true;

		registers.a = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::sbc_a_hl:
	{
		uint8_t value = mmu.read_byte(registers.hl);
		uint8_t carry;
		flags.c == true ? carry = 1U : carry = 0U;
		uint8_t res = registers.a - value - carry;

		res == 0 ? flags.z = true : flags.z = false;
		value + carry > registers.a ? flags.c = true : flags.c = false;
		(registers.a ^ value ^ res) & 0x10 ? flags.h = true : flags.h = false;

		flags.n = true;

		registers.a = res;
		pc += 2;
		cycles = 8;
		break;
	}
	case Instruction::and_d8:
		registers.a &= mmu.read_byte(pc + 1);

		registers.a == 0 ? flags.z = true : flags.z = false;
		flags.n = false;
		flags.c = false;
		flags.h = true;
		pc += 2;
		cycles = 8;
		break;
	case Instruction::and_a:
		registers.a &= registers.a;

		registers.a == 0 ? flags.z = true : flags.z = false;
		flags.n = false;
		flags.c = false;
		flags.h = true;
		++pc;
		cycles = 4;
		break;
	case Instruction::and_b:
		registers.a &= registers.b;

		registers.a == 0 ? flags.z = true : flags.z = false;
		flags.n = false;
		flags.c = false;
		flags.h = true;
		++pc;
		cycles = 4;
		break;
	case Instruction::and_c:
		registers.a &= registers.c;

		registers.a == 0 ? flags.z = true : flags.z = false;
		flags.n = false;
		flags.c = false;
		flags.h = true;
		++pc;
		cycles = 4;
		break;
	case Instruction::and_d:
		registers.a &= registers.d;

		registers.a == 0 ? flags.z = true : flags.z = false;
		flags.n = false;
		flags.c = false;
		flags.h = true;
		++pc;
		cycles = 4;
		break;
	case Instruction::and_e:
		registers.a &= registers.e;

		registers.a == 0 ? flags.z = true : flags.z = false;
		flags.n = false;
		flags.c = false;
		flags.h = true;
		++pc;
		cycles = 4;
		break;
	case Instruction::and_h:
		registers.a &= registers.h;

		registers.a == 0 ? flags.z = true : flags.z = false;
		flags.n = false;
		flags.c = false;
		flags.h = true;
		++pc;
		cycles = 4;
		break;
	case Instruction::and_l:
		registers.a &= registers.l;

		registers.a == 0 ? flags.z = true : flags.z = false;
		flags.n = false;
		flags.c = false;
		flags.h = true;
		++pc;
		cycles = 4;
		break;
	case Instruction::and_hl:
		registers.a &= mmu.read_byte(registers.hl);

		registers.a == 0 ? flags.z = true : flags.z = false;
		flags.n = false;
		flags.c = false;
		flags.h = true;
		pc += 2;
		cycles = 8;
		break;

	case Instruction::sub_d8:
	{
		uint8_t val = mmu.read_byte(pc + 1);
		uint8_t res = registers.a - val;

		res == 0 ? flags.z = true : flags.z = false;
		val > registers.a ? flags.c = true : flags.c = false;
		(registers.a ^ val ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = true;

		registers.a = res;
		pc += 2;
		cycles = 8;
		break;
	}
	case Instruction::sub_a:
	{
		uint8_t res = registers.a - registers.a;

		res == 0 ? flags.z = true : flags.z = false;
		flags.c = false;
		flags.h = false;
		//registers.a > registers.a ? flags.c = true : flags.c = false;
		//(registers.a ^ registers.a ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = true;

		registers.a = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::sub_b:
	{
		uint8_t res = registers.a - registers.b;

		res == 0 ? flags.z = true : flags.z = false;
		registers.b > registers.a ? flags.c = true : flags.c = false;
		(registers.a ^ registers.b ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = true;

		registers.a = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::sub_c:
	{
		uint8_t res = registers.a - registers.c;

		res == 0 ? flags.z = true : flags.z = false;
		registers.c > registers.a ? flags.c = true : flags.c = false;
		(registers.a ^ registers.c ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = true;

		registers.a = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::sub_d:
	{
		uint8_t res = registers.a - registers.d;

		res == 0 ? flags.z = true : flags.z = false;
		registers.d > registers.a ? flags.c = true : flags.c = false;
		(registers.a ^ registers.d ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = true;

		registers.a = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::sub_e:
	{
		uint8_t res = registers.a - registers.e;

		res == 0 ? flags.z = true : flags.z = false;
		registers.e > registers.a ? flags.c = true : flags.c = false;
		(registers.a ^ registers.e ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = true;

		registers.a = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::sub_h:
	{
		uint8_t res = registers.a - registers.h;

		res == 0 ? flags.z = true : flags.z = false;
		registers.h > registers.a ? flags.c = true : flags.c = false;
		(registers.a ^ registers.h ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = true;

		registers.a = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::sub_l:
	{
		uint8_t res = registers.a - registers.l;

		res == 0 ? flags.z = true : flags.z = false;
		registers.l > registers.a ? flags.c = true : flags.c = false;
		(registers.a ^ registers.l ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = true;

		registers.a = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::sub_hl:
	{
		uint8_t val = mmu.read_byte(registers.hl);
		uint8_t res = registers.a - val;

		res == 0 ? flags.z = true : flags.z = false;
		val > registers.a ? flags.c = true : flags.c = false;
		(registers.a ^ val ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = true;

		registers.a = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::xor_a:
		registers.a ^= registers.a;
		registers.a == 0 ? flags.z = true : flags.z = false;
		flags.c = false;
		flags.h = false;
		flags.n = false;
		++pc;
		cycles = 4;
		break;
	case Instruction::xor_b:
		registers.a ^= registers.b;
		registers.a == 0 ? flags.z = true : flags.z = false;
		flags.c = false;
		flags.h = false;
		flags.n = false;
		++pc;
		cycles = 4;
		break;
	case Instruction::xor_c:
	{
		uint8_t res = registers.a ^ registers.c;
		res == 0 ? flags.z = true : flags.z = false;
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
		res == 0 ? flags.z = true : flags.z = false;
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
		res == 0 ? flags.z = true : flags.z = false;
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
		res == 0 ? flags.z = true : flags.z = false;
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
		res == 0 ? flags.z = true : flags.z = false;
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
		registers.a == 0 ? flags.z = true : flags.z = false;
		flags.c = false;
		flags.h = false;
		flags.n = false;

		++pc;
		cycles = 4;
		break;
	case Instruction::xor_d8:
		registers.a ^= mmu.read_byte(pc + 1);
		registers.a == 0 ? flags.z = true : flags.z = false;
		flags.c = false;
		flags.h = false;
		flags.n = false;

		pc += 2;
		cycles = 8;
		break;
	case Instruction::or_a:
		registers.a |= registers.a;
		registers.a == 0 ? flags.z = true : flags.z = false;
		flags.c = false;
		flags.h = false;
		flags.n = false;
		++pc;
		cycles = 4;
		break;
	case Instruction::or_b:
		registers.a |= registers.b;
		registers.a == 0 ? flags.z = true : flags.z = false;
		flags.c = false;
		flags.h = false;
		flags.n = false;
		++pc;
		cycles = 4;
		break;
	case Instruction::or_c:
		registers.a |= registers.c;
		registers.a == 0 ? flags.z = true : flags.z = false;
		flags.c = false;
		flags.h = false;
		flags.n = false;
		++pc;
		cycles = 4;
		break;
	case Instruction::or_d:
		registers.a |= registers.d;
		registers.a == 0 ? flags.z = true : flags.z = false;
		flags.c = false;
		flags.h = false;
		flags.n = false;
		++pc;
		cycles = 4;
		break;
	case Instruction::or_e:
		registers.a |= registers.e;
		registers.a == 0 ? flags.z = true : flags.z = false;
		flags.c = false;
		flags.h = false;
		flags.n = false;
		++pc;
		cycles = 4;
		break;
	case Instruction::or_h:
		registers.a |= registers.h;
		registers.a == 0 ? flags.z = true : flags.z = false;
		flags.c = false;
		flags.h = false;
		flags.n = false;
		++pc;
		cycles = 4;
		break;
	case Instruction::or_l:
		registers.a |= registers.l;
		registers.a == 0 ? flags.z = true : flags.z = false;
		flags.c = false;
		flags.h = false;
		flags.n = false;
		++pc;
		cycles = 4;
		break;
	case Instruction::or_hl:
		registers.a |= mmu.read_byte(registers.hl);
		registers.a == 0 ? flags.z = true : flags.z = false;
		flags.c = false;
		flags.h = false;
		flags.n = false;
		++pc;
		cycles = 8;
		break;
	case Instruction::or_d8:
		registers.a |= mmu.read_byte(pc + 1);
		registers.a == 0 ? flags.z = true : flags.z = false;
		flags.c = false;
		flags.h = false;
		flags.n = false;
		pc += 2;
		cycles = 8;
		break;
	case Instruction::inc_a:
	{
		uint8_t res = registers.a + 1;

		res == 0 ? flags.z = true : flags.z = false;
		(registers.a ^ 1 ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = false;

		registers.a = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::inc_b:
	{
		uint8_t res = registers.b + 1;

		res == 0 ? flags.z = true : flags.z = false;
		(registers.b ^ 1 ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = false;

		registers.b = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::inc_c:
	{
		uint8_t res = registers.c + 1;

		res == 0 ? flags.z = true : flags.z = false;
		(registers.c ^ 1 ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = false;

		registers.c = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::inc_d:
	{
		uint8_t res = registers.d + 1;

		res == 0 ? flags.z = true : flags.z = false;
		(registers.d ^ 1 ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = false;

		registers.d = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::inc_e:
	{
		uint8_t res = registers.e + 1;

		res == 0 ? flags.z = true : flags.z = false;
		(registers.e ^ 1 ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = false;

		registers.e = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::inc_h:
	{
		uint8_t res = registers.h + 1;

		res == 0 ? flags.z = true : flags.z = false;
		(registers.h ^ 1 ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = false;

		registers.h = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::inc_l:
	{
		uint8_t res = registers.l + 1;

		res == 0 ? flags.z = true : flags.z = false;
		(registers.l ^ 1 ^ res) & 0x10 ? flags.h = true : flags.h = false;
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

		res == 0 ? flags.z = true : flags.z = false;
		(val ^ 1 ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = false;

		++pc;
		cycles = 12;
		break;
	}
	case Instruction::dec_a:
	{
		uint8_t res = registers.a - 1;

		res == 0 ? flags.z = true : flags.z = false;
		(registers.a ^ 1 ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = true;

		registers.a = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::dec_b:
	{
		uint8_t res = registers.b - 1;

		res == 0 ? flags.z = true : flags.z = false;
		(registers.b ^ 1 ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = true;

		registers.b = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::dec_c:
	{
		uint8_t res = registers.c - 1;

		res == 0 ? flags.z = true : flags.z = false;
		(registers.c ^ 1 ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = true;

		registers.c = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::dec_d:
	{
		uint8_t res = registers.d - 1;

		res == 0 ? flags.z = true : flags.z = false;
		(registers.d ^ 1 ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = true;

		registers.d = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::dec_e:
	{
		uint8_t res = registers.e - 1;

		res == 0 ? flags.z = true : flags.z = false;
		(registers.e ^ 1 ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = true;

		registers.e = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::dec_h:
	{
		uint8_t res = registers.h - 1;

		res == 0 ? flags.z = true : flags.z = false;
		(registers.h ^ 1 ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = true;

		registers.h = res;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::dec_l:
	{
		uint8_t res = registers.l - 1;

		res == 0 ? flags.z = true : flags.z = false;
		(registers.l ^ 1 ^ res) & 0x10 ? flags.h = true : flags.h = false;
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
		res == 0 ? flags.z = true : flags.z = false;
		(val ^ 1 ^ res) & 0x10 ? flags.h = true : flags.h = false;
		flags.n = true;

		++pc;
		cycles = 12;
		break;
	}
	case Instruction::cp_a:
	{
		uint8_t res = registers.a - registers.a;

		res == 0 ? flags.z = true : flags.z = false;
		(registers.a ^ registers.a ^ res) & 0x10 ? flags.h = true : flags.h = false;
		registers.a > registers.a ? flags.c = true : flags.c = false;
		flags.n = true;

		++pc;
		cycles = 4;
		break;
	}
	case Instruction::cp_b:
	{
		uint8_t res = registers.a - registers.b;

		res == 0 ? flags.z = true : flags.z = false;
		(registers.a ^ registers.b ^ res) & 0x10 ? flags.h = true : flags.h = false;
		registers.b > registers.a ? flags.c = true : flags.c = false;
		flags.n = true;

		++pc;
		cycles = 4;
		break;
	}
	case Instruction::cp_c:
	{
		uint8_t res = registers.a - registers.c;

		res == 0 ? flags.z = true : flags.z = false;
		(registers.a ^ registers.c ^ res) & 0x10 ? flags.h = true : flags.h = false;
		registers.c > registers.a ? flags.c = true : flags.c = false;
		flags.n = true;

		++pc;
		cycles = 4;
		break;
	}
	case Instruction::cp_d:
	{
		uint8_t res = registers.a - registers.d;

		res == 0 ? flags.z = true : flags.z = false;
		(registers.a ^ registers.d ^ res) & 0x10 ? flags.h = true : flags.h = false;
		registers.d > registers.a ? flags.c = true : flags.c = false;
		flags.n = true;

		++pc;
		cycles = 4;
		break;
	}
	case Instruction::cp_e:
	{
		uint8_t res = registers.a - registers.e;

		res == 0 ? flags.z = true : flags.z = false;
		(registers.a ^ registers.e ^ res) & 0x10 ? flags.h = true : flags.h = false;
		registers.e > registers.a ? flags.c = true : flags.c = false;
		flags.n = true;

		++pc;
		cycles = 4;
		break;
	}
	case Instruction::cp_h:
	{
		uint8_t res = registers.a - registers.h;

		res == 0 ? flags.z = true : flags.z = false;
		(registers.a ^ registers.h ^ res) & 0x10 ? flags.h = true : flags.h = false;
		registers.h > registers.a ? flags.c = true : flags.c = false;
		flags.n = true;

		++pc;
		cycles = 4;
		break;
	}
	case Instruction::cp_l:
	{
		uint8_t res = registers.a - registers.l;

		res == 0 ? flags.z = true : flags.z = false;
		(registers.a ^ registers.l ^ res) & 0x10 ? flags.h = true : flags.h = false;
		registers.l > registers.a ? flags.c = true : flags.c = false;
		flags.n = true;

		++pc;
		cycles = 4;
		break;
	}
	case Instruction::cp_d8:
	{
		uint8_t val = mmu.read_byte(pc + 1);
		uint8_t res = registers.a - val;

		res == 0 ? flags.z = true : flags.z = false;
		(registers.a ^ val ^ res) & 0x10 ? flags.h = true : flags.h = false;
		val > registers.a ? flags.c = true : flags.c = false;
		flags.n = true;

		pc += 2;
		cycles = 8;
		break;
	}
	case Instruction::cp_hl:
	{
		uint8_t val = mmu.read_byte(registers.hl);
		uint8_t res = registers.a - val;

		res == 0 ? flags.z = true : flags.z = false;
		flags.n = true;
		(registers.a ^ val ^ res) & 0x10 ? flags.h = true : flags.h = false;
		val > registers.a ? flags.c = true : flags.c = false;

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
	case Instruction::inc_sp:
		++sp;
		++pc;
		cycles = 8;
		break;
	case Instruction::dec_bc:
		--registers.bc;
		++pc;
		cycles = 8;
		break;
	case Instruction::dec_de:
		--registers.de;
		++pc;
		cycles = 8;
		break;
	case Instruction::dec_hl:
		--registers.hl;
		++pc;
		cycles = 8;
		break;
	case Instruction::dec_sp:
		--sp;
		++pc;
		cycles = 8;
		break;
	case Instruction::add_hl_bc:
	{
		uint32_t res = registers.hl + registers.bc;

		res > UINT16_MAX ? flags.c = true : flags.c = false;
		(registers.hl ^ registers.bc ^ res) & 0x1000 ? flags.h = true : flags.h = false;
		flags.n = false;

		registers.hl = static_cast<uint16_t>(res);
		++pc;
		cycles = 8;
		break;
	}
	case Instruction::add_hl_de:
	{
		uint32_t res = registers.hl + registers.de;

		res > UINT16_MAX ? flags.c = true : flags.c = false;
		(registers.hl ^ registers.de ^ res) & 0x1000 ? flags.h = true : flags.h = false;
		flags.n = false;

		registers.hl = static_cast<uint16_t>(res);
		++pc;
		cycles = 8;
		break;
	}
	case Instruction::add_hl_hl:
	{
		uint32_t res = registers.hl + registers.hl;

		res > UINT16_MAX ? flags.c = true : flags.c = false;
		(registers.hl ^ registers.hl ^ res) & 0x1000 ? flags.h = true : flags.h = false;
		flags.n = false;

		registers.hl = static_cast<uint16_t>(res);
		++pc;
		cycles = 8;
		break;
	}
	case Instruction::add_hl_sp:
	{
		uint32_t res = registers.hl + sp;

		res > UINT16_MAX ? flags.c = true : flags.c = false;
		(registers.hl ^ sp ^ res) & 0x1000 ? flags.h = true : flags.h = false;
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
	case Instruction::jp_c_a16:
		if (!flags.c) {
			pc += 3;
			cycles = 12;
		}
		else {
			pc = (mmu.read_byte(pc + 2) << 8) + mmu.read_byte(pc + 1);
			cycles = 16;
		}
		break;
	case Instruction::jp_nc_a16:
		if (flags.c) {
			pc += 3;
			cycles = 12;
		}
		else {
			pc = (mmu.read_byte(pc + 2) << 8) + mmu.read_byte(pc + 1);
			cycles = 16;
		}
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
		rl(registers.a);
		// this instruction has a hardware bug
		flags.z = false;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::rra:
	{
		rr(registers.a);
		// hardware bug
		flags.z = false;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::rrca:
	{
		uint8_t bit = bit_check(registers.a, 0);
		registers.a >>= 1;
		registers.a ^= (-bit ^ registers.a) & (1U << 7);
		//flags.c = bit;
		bit == 0 ? flags.c = false : flags.c = true;
		// unsure whether Z flag should be modified here
		flags.z = false;
		flags.n = false;
		flags.h = false;
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
		// unsure whether Z flag should be modified here
		flags.z = false;
		flags.n = false;
		flags.h = false;
		++pc;
		cycles = 4;
		break;
	}
	// Calls
	case Instruction::call_a16:
		mmu.write_byte(--sp, (pc + 3) >> 8);
		mmu.write_byte(--sp, pc + 3);
		pc = (mmu.read_byte(pc + 2) << 8) + mmu.read_byte(pc + 1);
		cycles = 12;
		break;
	case Instruction::call_nz_a16:
		if (!flags.z) {
			uint16_t address = (mmu.read_byte(pc + 2) << 8) + mmu.read_byte(pc + 1);
			mmu.write_byte(--sp, (pc + 3) >> 8);
			mmu.write_byte(--sp, pc + 3);
			cycles = 24;
			pc = address;
		}
		else {
			pc += 3;
			cycles = 12;
		}
		break;
	case Instruction::call_z_a16:
		if (flags.z) {
			uint16_t address = (mmu.read_byte(pc + 2) << 8) + mmu.read_byte(pc + 1);
			mmu.write_byte(--sp, (pc + 3) >> 8);
			mmu.write_byte(--sp, pc + 3);
			cycles = 24;
			pc = address;
		}
		else {
			pc += 3;
			cycles = 12;
		}
		break;
	case Instruction::call_nc_a16:
		if (!flags.c) {
			uint16_t address = (mmu.read_byte(pc + 2) << 8) + mmu.read_byte(pc + 1);
			mmu.write_byte(--sp, (pc + 3) >> 8);
			mmu.write_byte(--sp, pc + 3);
			cycles = 24;
			pc = address;
		}
		else {
			pc += 3;
			cycles = 12;
		}
		break;
	case Instruction::call_c_a16:
		if (flags.c) {
			uint16_t address = (mmu.read_byte(pc + 2) << 8) + mmu.read_byte(pc + 1);
			mmu.write_byte(--sp, (pc + 3) >> 8);
			mmu.write_byte(--sp, pc + 3);
			cycles = 24;
			pc = address;
		}
		else {
			pc += 3;
			cycles = 12;
		}
		break;
	// Restarts
	case Instruction::rst_00h:
		mmu.write_byte(--sp, (pc + 1) >> 8);
		mmu.write_byte(--sp, pc + 1);
		pc = 0x0000;
		cycles = 16;
		break;
	case Instruction::rst_08h:
		mmu.write_byte(--sp, (pc + 1) >> 8);
		mmu.write_byte(--sp, pc + 1);
		pc = 0x0008;
		cycles = 16;
		break;
	case Instruction::rst_10h:
		mmu.write_byte(--sp, (pc + 1) >> 8);
		mmu.write_byte(--sp, pc + 1);
		pc = 0x0010;
		cycles = 16;
		break;
	case Instruction::rst_18h:
		mmu.write_byte(--sp, (pc + 1) >> 8);
		mmu.write_byte(--sp, pc + 1);
		pc = 0x0018;
		cycles = 16;
		break;
	case Instruction::rst_20h:
		mmu.write_byte(--sp, (pc + 1) >> 8);
		mmu.write_byte(--sp, pc + 1);
		pc = 0x0020;
		cycles = 16;
		break;
	case Instruction::rst_28h:
		mmu.write_byte(--sp, (pc + 1) >> 8);
		mmu.write_byte(--sp, pc + 1);
		pc = 0x0028;
		cycles = 16;
		break;
	case Instruction::rst_30h:
		mmu.write_byte(--sp, (pc + 1) >> 8);
		mmu.write_byte(--sp, pc + 1);
		pc = 0x0030;
		cycles = 16;
		break;
	case Instruction::rst_38h:
		mmu.write_byte(--sp, (pc + 1) >> 8);
		mmu.write_byte(--sp, pc + 1);
		pc = 0x0038;
		cycles = 16;
		break;
	// Returns
	case Instruction::ret:
	{
		pc = (mmu.read_byte(sp + 1) << 8) + mmu.read_byte(sp);
		sp += 2;
		cycles = 8;
		break;
	}
	case Instruction::reti:
	{
		pc = (mmu.read_byte(sp + 1) << 8) + mmu.read_byte(sp);
		ime = true;
		sp += 2;
		cycles = 16;
		break;
	}
	case Instruction::ret_z:
	{
		if (flags.z) {
			pc = (mmu.read_byte(sp + 1) << 8) + mmu.read_byte(sp);
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
			pc = (mmu.read_byte(sp + 1) << 8) + mmu.read_byte(sp);
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
			pc = (mmu.read_byte(sp + 1) << 8) + mmu.read_byte(sp);
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
			pc = (mmu.read_byte(sp + 1) << 8) + mmu.read_byte(sp);
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
	case Instruction::daa:
	{
		//  Decimal adjust register A to get a correct BCD representation after an arithmetic instruction.
		// add 6 or sub 6 if flag.n
		if ((registers.a & 0x0F) > 0x09)
			flags.n ? registers.a -= 0x06 : registers.a += 0x06;
		// #TODO: carry stuff
		//throw;
		++pc;
		cycles = 4;
		break;
	}
	case Instruction::ld_a_at_c:
		registers.a = mmu.read_byte(0xFF00 + registers.c);
		++pc;
		cycles = 8;
		break;
	case Instruction::cpl:
		registers.a = ~registers.a;
		flags.n = true;
		flags.h = true;
		++pc;
		break;
	case Instruction::scf:
		flags.c = true;
		flags.n = false;
		flags.h = false;
		++pc;
		cycles = 4;
		break;
	case Instruction::ccf:
		flags.c = !flags.c;
		flags.n = false;
		flags.h = false;
		++pc;
		cycles = 4;
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
		//if (!ime) ++pc; //act as a nop if interrupts are disabled
		cycles = 4;
		break;
	default:
		spdlog::get("stdout")->error("Unknown opcode 0x{0:02x} at PC 0x{1:04x}", (uint8_t)opcode, pc);
		halted = true;
		//printf("Unknown opcode 0x%0.2X at PC 0x%0.4X\n", opcode, pc);
		//std::cerr << "Unknown Opcode: " << std::hex << opcode << std::dec << std::endl;
		break;
	}

	return (uint8_t)opcode;
}

inline void CPU::rlc(uint8_t &reg)
{
	uint8_t bit = bit_check(reg, 7);
	reg <<= 1;
	// put the bit we shifted out back in the 0th position
	reg ^= (-bit ^ reg) & (1U << 0);
	// carry flag holds old bit 7
	flags.c = bit;
	// update Z flag
	reg == 0 ? flags.z = true : flags.z = false;
	// reset subtract flag
	flags.n = false;
	// reset half carry flag
	flags.h = false;
}

inline void CPU::rrc(uint8_t &reg)
{
	uint8_t bit = bit_check(reg, 0);
	reg >>= 1;
	// return the old 0th back to bit 7, and put our new bit in the carry
	bit == 1U ? bit_set(reg, 7) : bit_clear(reg, 7);
	//reg == 0 ? flags.z = true : flags.z = false;
	flags.z = !reg;
	flags.c = bit;
	flags.h = false;
	flags.n = false;
}

/*     -----------------------v
     ^-- CY <-- [7 <-- 0] <---
*/
inline void CPU::rl(uint8_t &reg)
{
	uint8_t bit = bit_check(reg, 7);
	reg <<= 1;
	uint8_t carry;
	flags.c == true ? carry = 0x01 : carry = 0x00;
	// put the previous carry back in the 0th position
	reg ^= (-carry ^ reg) & (1U << 0);
	// carry flag holds old bit 7
	bit == 1U ? flags.c = true : flags.c = false;
	// update Z flag
	reg == 0 ? flags.z = true : flags.z = false;
	// reset subtract flag
	flags.n = false;
	// reset half carry flag
	flags.h = false;
}

inline void CPU::srl(uint8_t &reg)
{
	// shift right into carry
	uint8_t bit = bit_check(reg, 0);
	reg >>= 1;
	bit == 0 ? flags.c = false : flags.c = true;
	reg == 0 ? flags.z = true : flags.z = false;
	flags.h = false;
	flags.n = false;
}

/*	  *	v------------------------
** RR * --> CY --> [7 --> 0] ---^
*/
inline void CPU::rr(uint8_t &reg)
{
	uint8_t bit = bit_check(reg, 0);
	reg >>= 1;
	// return the old carry back to bit 7, and put our new bit in the carry
	flags.c ? bit_set(reg, 7) : bit_clear(reg, 7);
	reg == 0 ? flags.z = true : flags.z = false;
	// update carry with the new bit
	bit == 0 ? flags.c = false : flags.c = true;
	flags.h = false;
	flags.n = false;
}

inline void CPU::sla(uint8_t &reg)
{
	flags.c = bit_check(reg, 7);
	reg <<= 1;
	reg == 0 ? flags.z = true : flags.z = false;
	flags.h = false;
	flags.n = false;
}

inline void CPU::sra(uint8_t &reg)
{
	uint8_t bit = bit_check(reg, 7);
	flags.c = bit_check(reg, 0);
	reg >>= 1;
	reg ^= (-bit ^ reg) & (1U << 7);
	reg == 0 ? flags.z = true : flags.z = false;
	flags.h = false;
	flags.n = false;
}

inline void CPU::swap(uint8_t &reg)
{
	uint8_t temp = reg;
	reg <<= 4;
	temp >>= 4;
	reg &= 0xF0;
	temp &= 0x0F;
	reg |= temp;
	reg == 0 ? flags.z = true : flags.z = false;
	flags.n = false;
	flags.h = false;
	flags.c = false;
}