#include "c++lib.hpp"
#include "shader.hpp"


shader_stage::shader_stage(std::string_view src, GLenum kind)
{
	id = glCreateShader(kind);
	const char *ptr = src.data();
	glShaderSource(id, 1, &ptr, NULL);
	glCompileShader(id);

	int success;
	glGetShaderiv(id, GL_COMPILE_STATUS, &success);
	if (!success) {
		char buf[256];
		glGetShaderInfoLog(id, sizeof buf, NULL, buf);
		std::print("[GL] Compile shader {}: {}\n", kind, buf);
		id = 0;
	}
}

void shader_stage::fini()
{
	glDeleteShader(id);
}

shader_pipeline::shader_pipeline(std::string_view vsrc, std::string_view fsrc)
	: id(0)
{
	shader_stage v{vsrc, GL_VERTEX_SHADER};
	shader_stage f{fsrc, GL_FRAGMENT_SHADER};
	if (v.id == 0 || f.id == 0) return;
	id = glCreateProgram();
	glAttachShader(id, v.id);
	glAttachShader(id, f.id);
	glLinkProgram(id);

	int success;
	glGetProgramiv(id, GL_LINK_STATUS, &success);
	if (!success) {
		char buf[256];
		glGetProgramInfoLog(id, sizeof buf, NULL, buf);
		std::print("[GL] Link shader: {}\n", buf);
	}
}

void shader_pipeline::fini()
{
	glDeleteProgram(id);
}

bool shader_pipeline::ok() const
{
	return !!id;
}

void shader_pipeline::bind() const
{
	glUseProgram(id);
}

