#include <iostream>
#include <array>
#include <GLFW/glfw3.h>

#include "gb.h"
#include "instructions.h"

void error_callback(int error, const char* description) {
	std::cerr << "Error: " << description << std::endl;
}
int main(int argc, char** argv) {
	Gameboy gb = {};
	FILE *cartridge;
	long size;
	uint8_t *buffer;
	size_t result;
	//fopen_s(&cartridge, "../06-ld r,r.gb", "rb");
	fopen_s(&cartridge, "../cpu_instrs.gb", "rb");
	fseek(cartridge, 0, SEEK_END);
	size = ftell(cartridge);
	rewind(cartridge);

	buffer = (uint8_t*)malloc(sizeof(uint8_t)*size);

	result = fread(buffer, 1, size, cartridge);
	//gb.load(buffer, size);
	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		return -1;
	}

	GLFWwindow* window = glfwCreateWindow(320, 240, "GB Emu", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	//glfwSwapInterval(1);

	//GLubyte pixels[8192]{ 0x00 };
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 320, 240, 0, GL_RED, GL_UNSIGNED_BYTE, &pixels);

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);
		
		//for (int i = 0; i < 0x2000; ++i) {
		//	if (i % 2)
		//		pixels[i] = 0xFF; //gb.memory[i + 0x8000];
		//}
		//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 320, 240, GL_RED, GL_UNSIGNED_BYTE, &pixels);
		
		//glfwSwapBuffers(window);
		//glfwPollEvents();
		if (gb.cycles > 0) {
			--gb.cycles;
			continue; // still not done executing previous instruction
		}
		gb.step();

		// en/disable interrupts after instruction AFTER the DI/EI instruction :S
		if (gb.get_prev_opcode() == 0xF3) {
			// DI
			gb.disable_interrupt();
		}
		if (gb.get_prev_opcode() == 0xFA) {
			// EI
			gb.enable_interrupt();
		}
	}

	glfwTerminate();
	return 0;
}