#pragma once

class Gameboy
{
	public:
		Gameboy();
		void step();
		void reset();
		void load(uint8_t*, long);
		uint8_t memory[0xFFFF]{0x00};
	private:
		struct I {
			uint8_t opcode;
			std::string name;
			int cycles;
			int length;
		};
		enum class Register : uint8_t {
			a,
			f,
			b,
			c,
			d,
			e,
			h,
			l
		};
		enum class Instruction : uint8_t {
			nop				= 0x00,
			ld_bc_d16		= 0X01,
			ld_bc_a			= 0x02,
			inc_bc			= 0x03,
			inc_b			= 0x04,
			dec_b			= 0x05,
			ld_b_d8			= 0x06,
			rlca			= 0x07,
			ld_a16_sp		= 0x08,
			add_hl_bc		= 0x09,
			ld_a_bc			= 0x0A,
			dec_bc			= 0x0B,
			inc_c			= 0x0C,
			dec_c			= 0x0D,
			ld_c_d8			= 0x0E,
			rrca			= 0x0F,
			stop			= 0x10,
			ld_de_d16		= 0x11,
			ld_de_a			= 0x12,
			inc_de			= 0x13,
			inc_d			= 0x14,
			dec_d			= 0x15,
			ld_d_d8			= 0x16,
			rla				= 0x17,
			jr_r8			= 0x18,
			add_hl_de		= 0x19,
			ld_a_de			= 0x1A,
			dec_de			= 0x1B,
			inc_e			= 0x1C,
			dec_e			= 0x1D,
			ld_e_d8			= 0x1E,
			rra				= 0x1F,
			jr_nz_r8		= 0x20,
			ld_hl_d16		= 0x21,
			ldi_hl_a		= 0x22,
			inc_hl			= 0x23,
			inc_h			= 0x24,
			dec_h			= 0x25,
			ld_h_d8			= 0x26,
			daa				= 0x27,
			jr_z_r8			= 0x28,
			add_hl_hl		= 0x29,
			ldi_a_hl		= 0x2A,
			dec_hl			= 0x2B,
			inc_l			= 0x2C,
			dec_l			= 0x2D,
			ld_l_d8			= 0x2E,
			cpl				= 0x2F,
			jr_nc_r8		= 0x30,
			ld_sp_d16		= 0x31,
			ldd_hl_a		= 0x32,
			inc_sp			= 0x33,
			inc_at_hl		= 0x34, // INC_(HL)
			dec_at_hl		= 0x35, // DEC_(HL)
			ld_hl_d8		= 0x36,
			scf				= 0x37,
			jr_c_r8			= 0x38,
			add_hl_sp		= 0x39,
			ldd_a_hl		= 0x3A,
			dec_sp			= 0x3B,
			inc_a			= 0x3C,
			dec_a			= 0x3D,
			ld_a_d8			= 0x3E,
			ccf				= 0x3F,
			ld_b_b			= 0x40,
			ld_b_c			= 0x41,
			ld_b_d			= 0x42,
			ld_b_e			= 0x43,
			ld_b_h			= 0x44,
			ld_b_l			= 0x45,
			ld_b_hl			= 0x46,
			ld_b_a			= 0x47,
			ld_c_b			= 0x48,
			ld_c_c			= 0x49,
			ld_c_d			= 0x4A,
			ld_c_e			= 0x4B,
			ld_c_h			= 0x4C,
			ld_c_l			= 0x4D,
			ld_c_hl			= 0x4E,
			ld_c_a			= 0x4F,
			ld_d_b			= 0x50,
			ld_d_c			= 0x51,
			ld_d_d			= 0x52,
			ld_d_e			= 0x53,
			ld_d_h			= 0x54,
			ld_d_l			= 0x55,
			ld_d_hl			= 0x56,
			ld_d_a			= 0x57,
			ld_e_b			= 0x58,
			ld_e_c			= 0x59,
			ld_e_d			= 0x5A,
			ld_e_e			= 0x5B,
			ld_e_h			= 0x5C,
			ld_e_l			= 0x5D,
			ld_e_hl			= 0x5E,
			ld_e_a			= 0x5F,
			ld_h_b			= 0x60,
			ld_h_c			= 0x61,
			ld_h_d			= 0x62,
			ld_h_e			= 0x63,
			ld_h_h			= 0x64,
			ld_h_l			= 0x65,
			ld_h_hl			= 0x66,
			ld_h_a			= 0x67,
			ld_l_b			= 0x68,
			ld_l_c			= 0x69,
			ld_l_d			= 0x6A,
			ld_l_e			= 0x6B,
			ld_l_h			= 0x6C,
			ld_l_l			= 0x6D,
			ld_l_hl			= 0x6E,
			ld_l_a			= 0x6F,
			ld_hl_b			= 0x70,
			ld_hl_c			= 0x71,
			ld_hl_d			= 0x72,
			ld_hl_e			= 0x73,
			ld_hl_h			= 0x74,
			ld_hl_l			= 0x75,
			halt			= 0x76,
			ld_hl_a			= 0x77,
			ld_a_b			= 0x78,
			ld_a_c			= 0x79,
			ld_a_d			= 0x7A,
			ld_a_e			= 0x7B,
			ld_a_h			= 0x7C,
			ld_a_l			= 0x7D,
			ld_a_at_hl		= 0x7E,
			ld_a_a			= 0x7F,
			add_a_b			= 0x80,
			add_a_c			= 0x81,
			add_a_d			= 0x82,
			add_a_e			= 0x83,
			add_a_h			= 0x84,
			add_a_l			= 0x85,
			add_a_hl		= 0x86,
			add_a_a			= 0x87,
			adc_a_b			= 0x88,
			adc_a_c			= 0x89,
			adc_a_d			= 0x8A,
			adc_a_e			= 0x8B,
			adc_a_h			= 0x8C,
			adc_a_l			= 0x8D,
			adc_a_hl		= 0x8E,
			adc_a_a			= 0x8F,
			sub_b			= 0x90,
			sub_c			= 0x91,
			sub_d			= 0x92,
			sub_e			= 0x93,
			sub_h			= 0x94,
			sub_l			= 0x95,
			sub_hl			= 0x96,
			sub_a			= 0x97,
			sbc_a_b			= 0x98,
			sbc_a_c			= 0x99,
			sbc_a_d			= 0x9A,
			sbc_a_e			= 0x9B,
			sbc_a_h			= 0x9C,
			sbc_a_l			= 0x9D,
			sbc_a_hl		= 0x9E,
			sbc_a_a			= 0x9F,
			and_b			= 0xA0,
			and_c			= 0xA1,
			and_d			= 0xA2,
			and_e			= 0xA3,
			and_h			= 0xA4,
			and_l			= 0xA5,
			and_hl			= 0xA6,
			and_a			= 0xA7,
			xor_b			= 0xA8,
			xor_c			= 0xA9,
			xor_d			= 0xAA,
			xor_e			= 0xAB,
			xor_h			= 0xAC,
			xor_l			= 0xAD,
			xor_hl			= 0xAE,
			xor_a			= 0xAF,
			or_b			= 0xB0,
			or_c			= 0xB1,
			or_d			= 0xB2,
			or_e			= 0xB3,
			or_h			= 0xB4,
			or_l			= 0xB5,
			or_hl			= 0xB6,
			or_a			= 0xB7,
			cp_b			= 0xB8,
			cp_c			= 0xB9,
			cp_d			= 0xBA,
			cp_e			= 0xBB,
			cp_h			= 0xBC,
			cp_l			= 0xBD,
			cp_hl			= 0xBE,
			cp_a			= 0xBF,
			ret_nz			= 0xC0,
			pop_bc			= 0xC1,
			jp_nz_a16		= 0xC2,
			jp_a16			= 0xC3,
			call_nz_a16		= 0xC4,
			push_bc			= 0xC5,
			add_a_d8		= 0xC6,
			rst_00h			= 0xC7,
			ret_z			= 0xC8,
			ret				= 0xC9,
			jp_z_a16		= 0xCA,
			prefix_cb		= 0xCB,
			call_z_a16		= 0xCC,
			call_a16		= 0xCD,
			adc_a_d8		= 0xCE,
			rst_08h			= 0xCF,
			ret_nc			= 0xD0,
			pop_de			= 0xD1,
			jp_nc_a16		= 0xD2,
			call_nc_a16		= 0xD4,
			push_de			= 0xD5,
			sub_d8			= 0xD6,
			rst_10h			= 0xD7,
			ret_c			= 0xD8,
			reti			= 0xD9,
			jp_c_a16		= 0xDA,
			call_c_a16		= 0xDC,
			sbc_a_d8		= 0xDE,
			rst_18h			= 0xDF,
			ldh_a8_a		= 0xE0,
			pop_hl			= 0xE1,
			ld_at_c_a		= 0xE2, // LD_(C)_A
			push_hl			= 0xE5,
			and_d8			= 0xE6,
			rst_20h			= 0xE7,
			add_sp_r8		= 0xE8,
			jp_hl			= 0xE9,
			ld_a16_a		= 0xEA,
			xor_d8			= 0xEE,
			rst_28h			= 0xEF,
			ldh_a_a8		= 0xF0,
			pop_af			= 0xF1,
			ld_a_at_c		= 0xF2, // LD_A_(C)
			di				= 0xF3,
			push_af			= 0xF5,
			or_d8			= 0xF6,
			rst_30h			= 0xF7,
			ld_hl_sp_R8		= 0xF8,
			ld_sp_hl		= 0xF9,
			ld_a_a16		= 0xFA,
			ei				= 0xFB,
			cp_d8			= 0xFE,
			rst_38h			= 0xFF
		};
		enum class Prefix_CB : uint8_t {
			rlc_b			= 0x00,
			rlc_c			= 0x01,
			rlc_d 			= 0x02,
			rlc_e			= 0x03,
			rlc_h			= 0x04,
			rlc_l			= 0x05,
			rlc_at_hl		= 0x06,
			rlc_a			= 0x07,
			rrc_b			= 0x08,
			rrc_c			= 0x09,
			rrc_d			= 0x0A,
			rrc_e			= 0x0B,
			rrc_h			= 0x0C,
			rrc_l			= 0x0D,
			rrc_at_hl		= 0x0E,
			rrc_a			= 0x0F,
			rl_b			= 0x10,
			rl_c			= 0x11,
			rl_d			= 0x12,
			rl_e			= 0x13,
			rl_h			= 0x14,
			rl_l			= 0x15,
			rl_at_hl		= 0x16,
			rl_a			= 0x17,
			rr_b			= 0x18,
			rr_c			= 0x19,
			rr_d			= 0x1A,
			rr_e			= 0x1B,
			rr_h			= 0x1C,
			rr_l			= 0x1D,
			rr_at_hl		= 0x1E,
			rr_a			= 0x1F,
			bit_7_h			= 0x7C,
			set_7_a			= 0xFF
		};
		// opcodes one byte only?
		Instruction opcode;
		uint16_t pc;
		uint16_t sp;
		// Boot ROM
		const unsigned char rom[256] = {
			0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB,
			0x21, 0x26, 0xFF, 0x0E, 0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3,
			0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0, 0x47, 0x11, 0x04, 0x01,
			0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B,
			0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22,
			0x23, 0x05, 0x20, 0xF9, 0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99,
			0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20, 0xF9, 0x2E, 0x0F, 0x18,
			0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04,
			0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20,
			0xF7, 0x1D, 0x20, 0xF2, 0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62,
			0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06, 0x7B, 0xE2, 0x0C, 0x3E,
			0x87, 0xE2, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20,
			0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17,
			0xC1, 0xCB, 0x11, 0x17, 0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9,
			0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83,
			0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
			0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63,
			0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E,
			0x3C, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C, 0x21, 0x04, 0x01, 0x11,
			0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x20, 0xFE, 0x23, 0x7D, 0xFE, 0x34, 0x20,
			0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x20, 0xFE,
			0x3E, 0x01, 0xE0, 0x50
		};

		// interrupt master enable
		bool ime;
		// Registers
		uint8_t registers[8]{};

		// Memory
		//std::array<uint8_t, 0xFFFF> memory;
		
};