#include "window.hpp"
#include "shader.hpp"


int main()
{
	using namespace std::literals;
	window win{"Gaming\0"sv};
	if (win.error()) return 1;

	shader_pipeline shader{
		"#version 410 core\n"
		"layout(location=0) in vec2 attr_pos;\n"
		"void main() {\n"
			"gl_Position = vec4(attr_pos, 0.0, 1.0);\n"
		"}\n\0"sv,
		"#version 410 core\n"
		"out vec4 frag_color;\n"
		"void main() {\n"
			"frag_color = vec4(1.0, 1.0, 1.0, 1.0);\n"
		"}\n\0"sv,
	};
	if (!shader.ok()) return 1;
	shader.bind();

	float vdata[] = {
		-0.5f, -0.5f,
		+0.5f, -0.5f,
		 0.0f, +0.5f,
	};
	GLuint glbufs[2];
	glGenVertexArrays(1, &glbufs[0]);
	glBindVertexArray(glbufs[0]);
	glGenBuffers(1, &glbufs[1]);
	glBindBuffer(GL_ARRAY_BUFFER, glbufs[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof vdata, vdata, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), nullptr);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	while (win.live()) {
		glBindVertexArray(glbufs[0]);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		win.draw();
	}

	glDeleteBuffers(1, &glbufs[1]);
	glDeleteVertexArrays(1, &glbufs[0]);
	shader.fini();
}


