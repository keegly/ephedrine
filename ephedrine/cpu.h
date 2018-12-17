#pragma once

namespace gb
{
	class CPU {
	public:
		// fetch decode execute
		// talk to mmu for memory access
	private:
		struct flags {
			bool z;
			bool n;
			bool h;
			bool c;
		};
		struct registers
		{
			uint8_t a;
			// f
			uint8_t b;
			uint8_t c;
			uint8_t d;
			uint8_t e;
			uint8_t h;
			uint8_t l;
		};
		uint16_t sp;
		uint16_t pc;
	};
}