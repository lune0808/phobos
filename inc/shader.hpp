#pragma once
#include <glad/gl.h>
#include <string_view>


struct shader_stage
{
	GLuint id;

	shader_stage(std::string_view src, GLenum kind);
	void fini();
};

struct shader_pipeline
{
	GLuint id;

	shader_pipeline() = default;
	shader_pipeline(std::string_view vsrc, std::string_view fsrc);
	void fini();

	bool ok() const;
	void bind() const;
};

