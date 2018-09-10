#include <iostream>
#include <GLFW/glfw3.h>

#include "gb.h"

void error_callback(int error, const char* description) {
	std::cerr << "Error: " << description << std::endl;
}

int main(int argc, char** argv) {
	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		// init fail
	}

	GLFWwindow* window = glfwCreateWindow(640, 480, "GBC Emu", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwTerminate();
	return 0;
}