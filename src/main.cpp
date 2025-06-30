#include "window.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>


int main()
{
	using namespace std::literals;
	// TODO: use a NUL-terminated string type
	window win{"Gaming\0"sv};
	if (win.error()) return 1;

	shader_pipeline shader{
		"#version 410 core\n"
		"layout(location=0) in vec2 attr_pos;\n"
		"layout(location=1) in vec2 attr_uv;\n"
		"out vec2 vert_uv;\n"
		"uniform mat2 unif_view;\n"
		"uniform mat2 unif_model;\n"
		"void main() {\n"
			"vert_uv = attr_uv;\n"
			"vec2 pos = unif_view * unif_model * attr_pos;\n"
			"gl_Position = vec4(pos, 0.0, 1.0);\n"
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

	glm::mat2 id(1.0f);
	glUniformMatrix2fv(glGetUniformLocation(shader.id, "unif_model"), 1, GL_FALSE, glm::value_ptr(id));
	glUniformMatrix2fv(glGetUniformLocation(shader.id, "unif_view" ), 1, GL_FALSE, glm::value_ptr(id));

	image basic{"res/basic.png\0"sv};
	texture tex{basic, shader, "unif_color\0"sv};
	basic.fini();

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
		const auto dims = win.dims();
		glm::mat2 view = { { 1.0f, 0.0f }, { 0.0f, float(dims.x) / float(dims.y) } };

		shader.bind();
		glUniformMatrix2fv(glGetUniformLocation(shader.id, "unif_view" ), 1, GL_FALSE, glm::value_ptr(view));
		tex.bind(0);
		glBindVertexArray(glbufs[0]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		win.draw();
	}

	glDeleteBuffers(1, &glbufs[1]);
	glDeleteVertexArrays(1, &glbufs[0]);
	shader.fini();
}


