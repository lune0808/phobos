#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string_view>


class window
{
	GLFWwindow *handle;

public:
	window(std::string_view title);
	~window();

	bool error() const;
	bool live() const;
	void draw() const;
	glm::ivec2 dims() const;
};

