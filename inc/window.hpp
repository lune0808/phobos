#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string_view>


class window
{
	GLFWwindow *handle;
	size_t iworld_zoom;

public:
	window() = default;
	void init(std::string_view title);
	void fini();

	bool error() const;
	bool live() const;
	void draw() const;
	glm::ivec2 dims() const;

	void world_zoom(bool closer);
	float get_world_zoom() const;

	auto get_handle() const { return this->handle; }
};

