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

	GLFWwindow* window = glfwCreateWindow(640, 480, "GB Emu", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	GLubyte pixels[8192]{ 0xFF };
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 64, 32, 0, GL_RED, GL_UNSIGNED_BYTE, &pixels);

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);
		
		//for (int i = 0; i < 0x2000; ++i) {
		//	if (i % 2)
		//		pixels[i] = 0xFF; //gb.memory[i + 0x8000];
		//}
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 64, 32, GL_RED, GL_UNSIGNED_BYTE, &pixels);
		
		glfwSwapBuffers(window);
		glfwPollEvents();
		gb.step();
	}

	glfwTerminate();
	return 0;
}