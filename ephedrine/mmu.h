#pragma once

namespace gb
{
	class MMU {
	public:
		MMU();
		uint8_t read_byte(uint8_t loc);
		void write_byte(uint8_t loc, uint8_t val);
		// total amount of 8kB memory banks we have
		int num_banks;
		std::array<uint8_t, 0u> memory;
	private:

	};
}