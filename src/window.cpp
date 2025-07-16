#include "c++lib.hpp"
#include "window.hpp"

static constexpr float world_zoom_tbl[] = {
	0.05f, 0.07f, 0.09f, 0.12f, 0.15f, 0.18f, 0.22f, 0.26f, 0.30f, 0.35f, 0.40f, 0.45f, 0.50f, 0.66f, 0.72f, 0.79f, 0.85f, 0.93f, 1.00f
};

void window::world_zoom(bool closer)
{
	size_t nexti = closer? iworld_zoom+1: iworld_zoom-1;
	if (nexti < std::size(world_zoom_tbl)) {
		iworld_zoom = nexti;
	}
}

float window::get_world_zoom() const
{
	return world_zoom_tbl[iworld_zoom];
}

void window::init(std::string_view title)
{
	handle = nullptr;
	iworld_zoom = std::size(world_zoom_tbl)/2;
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

void window::fini()
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
	const auto dims = this->dims();
	glViewport(0, 0, dims.x, dims.y);
}

glm::ivec2 window::dims() const
{
	int width;
	int height;
	glfwGetFramebufferSize(handle, &width, &height);
	return { width, height };
}

