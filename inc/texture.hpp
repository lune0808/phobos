#pragma once
#include <string_view>
#include <glad/gl.h>
#include "shader.hpp"


struct image
{
	unsigned char *base;
	int width;
	int height;
	int channels;

	image(std::string_view path);
	void fini();

	bool ok() const;
};

struct texture
{
	GLuint handle;
	GLuint location;

	texture() = default;
	texture(image const &img, shader_pipeline const &shader, std::string_view name);
	void fini();

	void bind(GLuint unit) const;
};


