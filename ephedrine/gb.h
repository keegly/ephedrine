#pragma once

class Gameboy
{
	public:
		Gameboy();
		void step();
		void reset();
	private:
		//unsigned short opcode;
		unsigned short pc;
		unsigned short sp;

		// Flag register
		unsigned char F;

};