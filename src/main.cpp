#include "gl/glew.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include "ExperimentController.h"

ExperimentController controller;

void resizeCB(GLFWwindow* window, int width, int height)
{
	controller.resize(width, height);
}

void keyboardCB(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	controller.keyInput(key, action, mods);
}

void mouseCB(GLFWwindow* window, int button, int action, int mods)
{
	controller.mouseButton(button, action, mods);
}

void cursorCB(GLFWwindow* window, double x, double y)
{
	controller.mouseMotion(x, y);
}

void scrollCB(GLFWwindow* window, double x, double y)
{
	controller.mouseScroll(x, y);
}

GLFWwindow* initGL(int width, int height, const char* title)
{
	if (!glfwInit())
		return NULL;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_RED_BITS, 8);
	glfwWindowHint(GLFW_GREEN_BITS, 8);
	glfwWindowHint(GLFW_BLUE_BITS, 8);
	glfwWindowHint(GLFW_ALPHA_BITS, 8);
	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	glfwWindowHint(GLFW_SAMPLES, 8);

	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (!window) {
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);

	glfwSwapInterval(0);

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		glfwTerminate();
		return NULL;
	}

	return window;
}

int main(int argc, char** argv)
{
	GLFWwindow* window = initGL(800, 600, "Leap Experiments");
	if (!window) {
		std::cout << "Couldn't initialize OpenGL" << std::endl;
		return 1;
	}

	srand(time(NULL));

	glfwSetFramebufferSizeCallback(window, resizeCB);
	glfwSetKeyCallback(window, keyboardCB);
	glfwSetMouseButtonCallback(window, mouseCB);
	glfwSetCursorPosCallback(window, cursorCB);
	glfwSetScrollCallback(window, scrollCB);

	int w, h;
	glfwGetWindowSize(window, &w, &h);
	controller.init();
	controller.resize(w, h);

	while (!glfwWindowShouldClose(window)) {
		controller.update();
		controller.draw();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();

    return 0;
}