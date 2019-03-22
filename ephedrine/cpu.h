#ifndef CPU_H
#define CPU_H

#include "mmu.h"
#include "bit_utility.h"
#include "spdlog\fmt\ostr.h"
#include <iomanip>

struct Flags {
	bool z;		// zero - bit 7
	bool n;		// subtraction - bit 6
	bool h;		// half carry - bit 5
	bool c;		// carry - bit 4
};

struct Registers
{
	union {
		uint16_t af;
		struct {
			uint8_t f;
			uint8_t a;
		};
	};
	union {
		uint16_t bc;
		struct {
			uint8_t c;
			uint8_t b;
		};
	};
	union {
		uint16_t de;
		struct {
			uint8_t e;
			uint8_t d;
		};
	};
	union {
		uint16_t hl;
		struct {
			uint8_t l;
			uint8_t h;
		};
	};
};

class CPU {
public:
	// fetch decode execute
	// talk to mmu_ for memory_ access
	// returns previous opcode
	CPU(std::shared_ptr<MMU> m);
	uint8_t step();
	void handle_interrupts();
	int cycles;
	void nop();
	void print();
	bool halted = false;
	// for ui
	const Registers GetRegisters() const {
		return this->registers_;
	}
	const Flags GetFlags() const {
		return this->flags_;
	}
	const CPU* GetState() const {
		return this;
	}
	uint16_t GetPC() const {
		return this->pc_;
	}
	uint16_t GetSP() const {
		return this->sp_;
	}
private:
	Registers registers_;
	Flags flags_;
	enum class Instruction : uint8_t {
		nop = 0x00,
		ld_bc_d16 = 0X01,
		ld_bc_a = 0x02,
		inc_bc = 0x03,
		inc_b = 0x04,
		dec_b = 0x05,
		ld_b_d8 = 0x06,
		rlca = 0x07,
		ld_a16_sp = 0x08,
		add_hl_bc = 0x09,
		ld_a_bc = 0x0A,
		dec_bc = 0x0B,
		inc_c = 0x0C,
		dec_c = 0x0D,
		ld_c_d8 = 0x0E,
		rrca = 0x0F,
		stop = 0x10,
		ld_de_d16 = 0x11,
		ld_de_a = 0x12,
		inc_de = 0x13,
		inc_d = 0x14,
		dec_d = 0x15,
		ld_d_d8 = 0x16,
		rla = 0x17,
		jr_r8 = 0x18,
		add_hl_de = 0x19,
		ld_a_de = 0x1A,
		dec_de = 0x1B,
		inc_e = 0x1C,
		dec_e = 0x1D,
		ld_e_d8 = 0x1E,
		rra = 0x1F,
		jr_nz_r8 = 0x20,
		ld_hl_d16 = 0x21,
		ldi_hl_a = 0x22,
		inc_hl = 0x23,
		inc_h = 0x24,
		dec_h = 0x25,
		ld_h_d8 = 0x26,
		daa = 0x27,
		jr_z_r8 = 0x28,
		add_hl_hl = 0x29,
		ldi_a_hl = 0x2A,
		dec_hl = 0x2B,
		inc_l = 0x2C,
		dec_l = 0x2D,
		ld_l_d8 = 0x2E,
		cpl = 0x2F,
		jr_nc_r8 = 0x30,
		ld_sp_d16 = 0x31,
		ldd_hl_a = 0x32,
		inc_sp = 0x33,
		inc_at_hl = 0x34, // INC_(HL)
		dec_at_hl = 0x35, // DEC_(HL)
		ld_hl_d8 = 0x36,
		scf = 0x37,
		jr_c_r8 = 0x38,
		add_hl_sp = 0x39,
		ldd_a_hl = 0x3A,
		dec_sp = 0x3B,
		inc_a = 0x3C,
		dec_a = 0x3D,
		ld_a_d8 = 0x3E,
		ccf = 0x3F,
		ld_b_b = 0x40,
		ld_b_c = 0x41,
		ld_b_d = 0x42,
		ld_b_e = 0x43,
		ld_b_h = 0x44,
		ld_b_l = 0x45,
		ld_b_hl = 0x46,
		ld_b_a = 0x47,
		ld_c_b = 0x48,
		ld_c_c = 0x49,
		ld_c_d = 0x4A,
		ld_c_e = 0x4B,
		ld_c_h = 0x4C,
		ld_c_l = 0x4D,
		ld_c_hl = 0x4E,
		ld_c_a = 0x4F,
		ld_d_b = 0x50,
		ld_d_c = 0x51,
		ld_d_d = 0x52,
		ld_d_e = 0x53,
		ld_d_h = 0x54,
		ld_d_l = 0x55,
		ld_d_hl = 0x56,
		ld_d_a = 0x57,
		ld_e_b = 0x58,
		ld_e_c = 0x59,
		ld_e_d = 0x5A,
		ld_e_e = 0x5B,
		ld_e_h = 0x5C,
		ld_e_l = 0x5D,
		ld_e_hl = 0x5E,
		ld_e_a = 0x5F,
		ld_h_b = 0x60,
		ld_h_c = 0x61,
		ld_h_d = 0x62,
		ld_h_e = 0x63,
		ld_h_h = 0x64,
		ld_h_l = 0x65,
		ld_h_hl = 0x66,
		ld_h_a = 0x67,
		ld_l_b = 0x68,
		ld_l_c = 0x69,
		ld_l_d = 0x6A,
		ld_l_e = 0x6B,
		ld_l_h = 0x6C,
		ld_l_l = 0x6D,
		ld_l_hl = 0x6E,
		ld_l_a = 0x6F,
		ld_hl_b = 0x70,
		ld_hl_c = 0x71,
		ld_hl_d = 0x72,
		ld_hl_e = 0x73,
		ld_hl_h = 0x74,
		ld_hl_l = 0x75,
		halt = 0x76,
		ld_hl_a = 0x77,
		ld_a_b = 0x78,
		ld_a_c = 0x79,
		ld_a_d = 0x7A,
		ld_a_e = 0x7B,
		ld_a_h = 0x7C,
		ld_a_l = 0x7D,
		ld_a_at_hl = 0x7E,
		ld_a_a = 0x7F,
		add_a_b = 0x80,
		add_a_c = 0x81,
		add_a_d = 0x82,
		add_a_e = 0x83,
		add_a_h = 0x84,
		add_a_l = 0x85,
		add_a_hl = 0x86,
		add_a_a = 0x87,
		adc_a_b = 0x88,
		adc_a_c = 0x89,
		adc_a_d = 0x8A,
		adc_a_e = 0x8B,
		adc_a_h = 0x8C,
		adc_a_l = 0x8D,
		adc_a_hl = 0x8E,
		adc_a_a = 0x8F,
		sub_b = 0x90,
		sub_c = 0x91,
		sub_d = 0x92,
		sub_e = 0x93,
		sub_h = 0x94,
		sub_l = 0x95,
		sub_hl = 0x96,
		sub_a = 0x97,
		sbc_a_b = 0x98,
		sbc_a_c = 0x99,
		sbc_a_d = 0x9A,
		sbc_a_e = 0x9B,
		sbc_a_h = 0x9C,
		sbc_a_l = 0x9D,
		sbc_a_hl = 0x9E,
		sbc_a_a = 0x9F,
		and_b = 0xA0,
		and_c = 0xA1,
		and_d = 0xA2,
		and_e = 0xA3,
		and_h = 0xA4,
		and_l = 0xA5,
		and_hl = 0xA6,
		and_a = 0xA7,
		xor_b = 0xA8,
		xor_c = 0xA9,
		xor_d = 0xAA,
		xor_e = 0xAB,
		xor_h = 0xAC,
		xor_l = 0xAD,
		xor_hl = 0xAE,
		xor_a = 0xAF,
		or_b = 0xB0,
		or_c = 0xB1,
		or_d = 0xB2,
		or_e = 0xB3,
		or_h = 0xB4,
		or_l = 0xB5,
		or_hl = 0xB6,
		or_a = 0xB7,
		cp_b = 0xB8,
		cp_c = 0xB9,
		cp_d = 0xBA,
		cp_e = 0xBB,
		cp_h = 0xBC,
		cp_l = 0xBD,
		cp_hl = 0xBE,
		cp_a = 0xBF,
		ret_nz = 0xC0,
		pop_bc = 0xC1,
		jp_nz_a16 = 0xC2,
		jp_a16 = 0xC3,
		call_nz_a16 = 0xC4,
		push_bc = 0xC5,
		add_a_d8 = 0xC6,
		rst_00h = 0xC7,
		ret_z = 0xC8,
		ret = 0xC9,
		jp_z_a16 = 0xCA,
		prefix_cb = 0xCB,
		call_z_a16 = 0xCC,
		call_a16 = 0xCD,
		adc_a_d8 = 0xCE,
		rst_08h = 0xCF,
		ret_nc = 0xD0,
		pop_de = 0xD1,
		jp_nc_a16 = 0xD2,
		call_nc_a16 = 0xD4,
		push_de = 0xD5,
		sub_d8 = 0xD6,
		rst_10h = 0xD7,
		ret_c = 0xD8,
		reti = 0xD9,
		jp_c_a16 = 0xDA,
		call_c_a16 = 0xDC,
		sbc_a_d8 = 0xDE,
		rst_18h = 0xDF,
		ldh_a8_a = 0xE0,
		pop_hl = 0xE1,
		ld_at_c_a = 0xE2, // LD_(C)_A
		push_hl = 0xE5,
		and_d8 = 0xE6,
		rst_20h = 0xE7,
		add_sp_r8 = 0xE8,
		jp_hl = 0xE9,
		ld_a16_a = 0xEA,
		xor_d8 = 0xEE,
		rst_28h = 0xEF,
		ldh_a_a8 = 0xF0,
		pop_af = 0xF1,
		ld_a_at_c = 0xF2, // LD_A_(C)
		di = 0xF3,
		push_af = 0xF5,
		or_d8 = 0xF6,
		rst_30h = 0xF7,
		ld_hl_sp_R8 = 0xF8,
		ld_sp_hl = 0xF9,
		ld_a_a16 = 0xFA,
		ei = 0xFB,
		cp_d8 = 0xFE,
		rst_38h = 0xFF
	};
	enum class Prefix_CB : uint8_t {
		rlc_b = 0x00,
		rlc_c = 0x01,
		rlc_d = 0x02,
		rlc_e = 0x03,
		rlc_h = 0x04,
		rlc_l = 0x05,
		rlc_hl = 0x06,
		rlc_a = 0x07,
		rrc_b = 0x08,
		rrc_c = 0x09,
		rrc_d = 0x0A,
		rrc_e = 0x0B,
		rrc_h = 0x0C,
		rrc_l = 0x0D,
		rrc_hl = 0x0E,
		rrc_a = 0x0F,
		rl_b = 0x10,
		rl_c = 0x11,
		rl_d = 0x12,
		rl_e = 0x13,
		rl_h = 0x14,
		rl_l = 0x15,
		rl_hl = 0x16,
		rl_a = 0x17,
		rr_b = 0x18,
		rr_c = 0x19,
		rr_d = 0x1A,
		rr_e = 0x1B,
		rr_h = 0x1C,
		rr_l = 0x1D,
		rr_hl = 0x1E,
		rr_a = 0x1F,
		sla_b = 0x20,
		sla_c = 0x21,
		sla_d = 0x22,
		sla_e = 0x23,
		sla_h = 0x24,
		sla_l = 0x25,
		sla_hl = 0x26,
		sla_a = 0x27,
		sra_b = 0x28,
		sra_c = 0x29,
		sra_d = 0x2A,
		sra_e = 0x2B,
		sra_h = 0x2C,
		sra_l = 0x2D,
		sra_hl = 0x2E,
		sra_a = 0x2F,
		swap_b = 0x30,
		swap_c = 0x31,
		swap_d = 0x32,
		swap_e = 0x33,
		swap_h = 0x34,
		swap_l = 0x35,
		swap_hl = 0x36,
		swap_a = 0x37,
		srl_b = 0x38,
		srl_c = 0x39,
		srl_d = 0x3A,
		srl_e = 0x3B,
		srl_h = 0x3C,
		srl_l = 0x3D,
		srl_hl = 0x3E,
		srl_a = 0x3F,
		bit_0_b = 0x40,
		bit_0_c = 0x41,
		bit_0_d = 0x42,
		bit_0_e = 0x43,
		bit_0_h = 0x44,
		bit_0_l = 0x45,
		bit_0_hl = 0x46,
		bit_0_a = 0x47,
		bit_1_b = 0x48,
		bit_1_c = 0x49,
		bit_1_d = 0x4A,
		bit_1_e = 0x4B,
		bit_1_h = 0x4C,
		bit_1_l = 0x4D,
		bit_1_hl = 0x4E,
		bit_1_a = 0x4F,
		bit_2_b = 0x50,
		bit_2_c = 0x51,
		bit_2_d = 0x52,
		bit_2_e = 0x53,
		bit_2_h = 0x54,
		bit_2_l = 0x55,
		bit_2_hl = 0x56,
		bit_2_a = 0x57,
		bit_3_b = 0x58,
		bit_3_c = 0x59,
		bit_3_d = 0x5A,
		bit_3_e = 0x5B,
		bit_3_h = 0x5C,
		bit_3_l = 0x5D,
		bit_3_hl = 0x5E,
		bit_3_a = 0x5F,
		bit_4_b = 0x60,
		bit_4_c = 0x61,
		bit_4_d = 0x62,
		bit_4_e = 0x63,
		bit_4_h = 0x64,
		bit_4_l = 0x65,
		bit_4_hl = 0x66,
		bit_4_a = 0x67,
		bit_5_b = 0x68,
		bit_5_c = 0x69,
		bit_5_d = 0x6A,
		bit_5_e = 0x6B,
		bit_5_h = 0x6C,
		bit_5_l = 0x6D,
		bit_5_hl = 0x6E,
		bit_5_a = 0x6F,
		bit_6_b = 0x70,
		bit_6_c = 0x71,
		bit_6_d = 0x72,
		bit_6_e = 0x73,
		bit_6_h = 0x74,
		bit_6_l = 0x75,
		bit_6_hl = 0x76,
		bit_6_a = 0x77,
		bit_7_b = 0x78,
		bit_7_c = 0x79,
		bit_7_d = 0x7A,
		bit_7_e = 0x7B,
		bit_7_h = 0x7C,
		bit_7_l = 0x7D,
		bit_7_hl = 0x7E,
		bit_7_a = 0x7F,
		res_0_b = 0x80,
		res_0_c = 0x81,
		res_0_d = 0x82,
		res_0_e = 0x83,
		res_0_h = 0x84,
		res_0_l = 0x85,
		res_0_hl = 0x86,
		res_0_a = 0x87,
		res_1_b = 0x88,
		res_1_c = 0x89,
		res_1_d = 0x8A,
		res_1_e = 0x8B,
		res_1_h = 0x8C,
		res_1_l = 0x8D,
		res_1_hl = 0x8E,
		res_1_a = 0x8F,
		res_2_b = 0x90,
		res_2_c = 0x91,
		res_2_d = 0x92,
		res_2_e = 0x93,
		res_2_h = 0x94,
		res_2_l = 0x95,
		res_2_hl = 0x96,
		res_2_a = 0x97,
		res_3_b = 0x98,
		res_3_c = 0x99,
		res_3_d = 0x9A,
		res_3_e = 0x9B,
		res_3_h = 0x9C,
		res_3_l = 0x9D,
		res_3_hl = 0x9E,
		res_3_a = 0x9F,
		res_4_b = 0xA0,
		res_4_c = 0xA1,
		res_4_d = 0xA2,
		res_4_e = 0xA3,
		res_4_h = 0xA4,
		res_4_l = 0xA5,
		res_4_hl = 0xA6,
		res_4_a = 0xA7,
		res_5_b = 0xA8,
		res_5_c = 0xA9,
		res_5_d = 0xAA,
		res_5_e = 0xAB,
		res_5_h = 0xAC,
		res_5_l = 0xAD,
		res_5_hl = 0xAE,
		res_5_a = 0xAF,
		res_6_b = 0xB0,
		res_6_c = 0xB1,
		res_6_d = 0xB2,
		res_6_e = 0xB3,
		res_6_h = 0xB4,
		res_6_l = 0xB5,
		res_6_hl = 0xB6,
		res_6_a = 0xB7,
		res_7_b = 0xB8,
		res_7_c = 0xB9,
		res_7_d = 0xBA,
		res_7_e = 0xBB,
		res_7_h = 0xBC,
		res_7_l = 0xBD,
		res_7_hl = 0xBE,
		res_7_a = 0xBF,
		set_0_b = 0xC0,
		set_0_c = 0xC1,
		set_0_d = 0xC2,
		set_0_e = 0xC3,
		set_0_h = 0xC4,
		set_0_l = 0xC5,
		set_0_hl = 0xC6,
		set_0_a = 0xC7,
		set_1_b = 0xC8,
		set_1_c = 0xC9,
		set_1_d = 0xCA,
		set_1_e = 0xCB,
		set_1_h = 0xCC,
		set_1_l = 0xCD,
		set_1_hl = 0xCE,
		set_1_a = 0xCF,
		set_2_b = 0xD0,
		set_2_c = 0xD1,
		set_2_d = 0xD2,
		set_2_e = 0xD3,
		set_2_h = 0xD4,
		set_2_l = 0xD5,
		set_2_hl = 0xD6,
		set_2_a = 0xD7,
		set_3_b = 0xD8,
		set_3_c = 0xD9,
		set_3_d = 0xDA,
		set_3_e = 0xDB,
		set_3_h = 0xDC,
		set_3_l = 0xDD,
		set_3_hl = 0xDE,
		set_3_a = 0xDF,
		set_4_b = 0xE0,
		set_4_c = 0xE1,
		set_4_d = 0xE2,
		set_4_e = 0xE3,
		set_4_h = 0xE4,
		set_4_l = 0xE5,
		set_4_hl = 0xE6,
		set_4_a = 0xE7,
		set_5_b = 0xE8,
		set_5_c = 0xE9,
		set_5_d = 0xEA,
		set_5_e = 0xEB,
		set_5_h = 0xEC,
		set_5_l = 0xED,
		set_5_hl = 0xEE,
		set_5_a = 0xEF,
		set_6_b = 0xF0,
		set_6_c = 0xF1,
		set_6_d = 0xF2,
		set_6_e = 0xF3,
		set_6_h = 0xF4,
		set_6_l = 0xF5,
		set_6_hl = 0xF6,
		set_6_a = 0xF7,
		set_7_b = 0xF8,
		set_7_c = 0xF9,
		set_7_d = 0xFA,
		set_7_e = 0xFB,
		set_7_h = 0xFC,
		set_7_l = 0xFD,
		set_7_hl = 0xFE,
		set_7_a = 0xFF
	};
	uint16_t sp_;
	uint16_t pc_;
	bool ime_;
//	MMU &mmu_;
	std::shared_ptr<MMU> mmu_;

	bool halt_bug_occurred = false;

	// some CB opcode DRY?
	constexpr void rlc(uint8_t &reg);
	constexpr void rrc(uint8_t &reg);
	constexpr void rl(uint8_t &reg);
	constexpr void rr(uint8_t &reg);
	constexpr void sla(uint8_t &reg);
	constexpr void sra(uint8_t &reg);
	constexpr void srl(uint8_t &reg);
	constexpr void swap(uint8_t &reg);

	// Flag register bit twiddling
	constexpr void set_z() { bit_set(registers_.f, 7); };
	constexpr void reset_z() { bit_clear(registers_.f, 7); };
	constexpr void toggle_z() { bit_flip(registers_.f, 7); };

	constexpr void set_n() { bit_set(registers_.f, 6); };
	constexpr void reset_n() { bit_clear(registers_.f, 6); };
	constexpr void toggle_n() { bit_flip(registers_.f, 6); };

	constexpr void set_h() { bit_set(registers_.f, 5); };
	constexpr void reset_h() { bit_clear(registers_.f, 5); };
	constexpr void toggle_h() { bit_flip(registers_.f, 5); };

	constexpr void set_c() { bit_set(registers_.f, 4); };
	constexpr void reset_c() { bit_clear(registers_.f, 4); };
	constexpr void toggle_c() { bit_flip(registers_.f, 4); };
};

#endif // !CPU_H
