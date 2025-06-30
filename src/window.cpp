#include <print>
#include "window.hpp"

window::window(std::string_view title)
	: handle(nullptr)
{
	if (!glfwInit()) return;
	glfwSetErrorCallback([] (int err, const char *desc)
	{
		std::print("[GLFW] Error {}: {}\n", err, desc);
	});

	int width = 800;
	int height = 600;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	handle = glfwCreateWindow(width, height, title.data(), NULL, NULL);
	if (!handle) return;

	glfwMakeContextCurrent(handle);
	gladLoadGL(glfwGetProcAddress);
	glViewport(0, 0, width, height);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

window::~window()
{
	if (!handle) return;
	glfwDestroyWindow(handle);
	glfwTerminate();
}

bool window::error() const
{
	return !handle;
}

bool window::live() const
{
	return !glfwWindowShouldClose(handle);
}

void window::draw() const
{
	glfwSwapBuffers(handle);
	glfwPollEvents();
	glClear(GL_COLOR_BUFFER_BIT);
}

