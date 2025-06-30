#include "window.hpp"
#include "shader.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <print>


int main()
{
	using namespace std::literals;
	window win{"Gaming\0"sv};
	if (win.error()) return 1;

	int width;
	int height;
	int channels;
	stbi_set_flip_vertically_on_load(true);
	auto pimg = stbi_load("res/basic.png", &width, &height, &channels, 0);
	if (!pimg || channels != 4) return 1;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pimg);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(pimg);

	shader_pipeline shader{
		"#version 410 core\n"
		"layout(location=0) in vec2 attr_pos;\n"
		"layout(location=1) in vec2 attr_uv;\n"
		"out vec2 vert_uv;\n"
		"void main() {\n"
			"vert_uv = attr_uv;\n"
			"gl_Position = vec4(attr_pos, 0.0, 1.0);\n"
		"}\n\0"sv,
		"#version 410 core\n"
		"in vec2 vert_uv;\n"
		"out vec4 frag_color;\n"
		"uniform sampler2D unif_color;\n"
		"void main() {\n"
			"frag_color = texture(unif_color, vert_uv);\n"
		"}\n\0"sv,
	};
	if (!shader.ok()) return 1;
	shader.bind();

	float vdata[] = {
		-0.5f, -0.5f, 0.0f, 0.0f,
		+0.5f, -0.5f, 1.0f, 0.0f,
		+0.5f, +0.5f, 1.0f, 1.0f,
		+0.5f, +0.5f, 1.0f, 1.0f,
		-0.5f, +0.5f, 0.0f, 1.0f,
		-0.5f, -0.5f, 0.0f, 0.0f,
	};
	GLuint glbufs[2];
	glGenVertexArrays(1, &glbufs[0]);
	glBindVertexArray(glbufs[0]);
	glGenBuffers(1, &glbufs[1]);
	glBindBuffer(GL_ARRAY_BUFFER, glbufs[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof vdata, vdata, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), nullptr);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	while (win.live()) {
		shader.bind();
		glUniform1i(glGetUniformLocation(shader.id, "unif_color"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex);
		glBindVertexArray(glbufs[0]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		win.draw();
	}

	glDeleteBuffers(1, &glbufs[1]);
	glDeleteVertexArrays(1, &glbufs[0]);
	shader.fini();
}


