#include "render.hpp"
#include <cstdlib>
#include <print>
#include <assert.h>
#include <string_view>
#include <glm/gtc/type_ptr.hpp>


render::render()
	: ok(0), nexte(1)
{
	using namespace std::literals;
	shader_pipeline shader{
		"#version 410 core\n"
		"layout(location=0) in vec2 attr_pos;\n"
		"layout(location=1) in vec2 attr_uv;\n"
		"out vec2 vert_uv;\n"
		"uniform mat3 unif_view;\n"
		"uniform mat3 unif_model;\n"
		"void main() {\n"
			"vert_uv = attr_uv;\n"
			"vec3 pos = unif_view * unif_model * vec3(attr_pos, 1.0);\n"
			"gl_Position = vec4(pos.xy, 0.0, 1.0);\n"
		"}\n\0"sv,

		"#version 410 core\n"
		"in vec2 vert_uv;\n"
		"out vec4 frag_color;\n"
		"uniform sampler2D unif_color;\n"
		"void main() {\n"
			"frag_color = texture(unif_color, vert_uv);\n"
		"}\n\0"sv,
	};
	glm::mat3 id(1.0f);
	if (!shader.ok()) goto fail;
	shader.bind();
	glUniformMatrix3fv(glGetUniformLocation(shader.id, "unif_model"), 1, GL_FALSE, glm::value_ptr(id));
	glUniformMatrix3fv(glGetUniformLocation(shader.id, "unif_view" ), 1, GL_FALSE, glm::value_ptr(id));

	{
		image img{"res/basic.png\0"sv};
		if (!img.ok()) goto fail;
		texture tex{img, shader, "unif_color\0"sv};
		img.fini();

		float vdata[] = {
			-0.5f, -0.5f, 0.0f, 0.0f,
			+0.5f, -0.5f, 1.0f, 0.0f,
			+0.5f, +0.5f, 1.0f, 1.0f,
			+0.5f, +0.5f, 1.0f, 1.0f,
			-0.5f, +0.5f, 0.0f, 1.0f,
			-0.5f, -0.5f, 0.0f, 0.0f,
		};
		GLuint va;
		glGenVertexArrays(1, &va);
		glBindVertexArray(va);
		GLuint vb;
		glGenBuffers(1, &vb);
		glBindBuffer(GL_ARRAY_BUFFER, vb);
		glBufferData(GL_ARRAY_BUFFER, sizeof vdata, vdata, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), nullptr);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
		glEnableVertexAttribArray(1);
		glBindVertexArray(0);

		ctx[player] = per_draw{ va, shader, tex };
		++ok;
	}

	{
		image img{"res/enemy.png\0"sv};
		if (!img.ok()) goto fail;
		texture tex{img, shader, "unif_color\0"sv};
		img.fini();

		float vdata[] = {
			-0.5f, -0.5f, 0.0f, 0.0f,
			+0.5f, -0.5f, 1.0f, 0.0f,
			+0.5f, +0.5f, 1.0f, 1.0f,
			+0.5f, +0.5f, 1.0f, 1.0f,
			-0.5f, +0.5f, 0.0f, 1.0f,
			-0.5f, -0.5f, 0.0f, 0.0f,
		};
		GLuint va;
		glGenVertexArrays(1, &va);
		glBindVertexArray(va);
		GLuint vb;
		glGenBuffers(1, &vb);
		glBindBuffer(GL_ARRAY_BUFFER, vb);
		glBufferData(GL_ARRAY_BUFFER, sizeof vdata, vdata, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), nullptr);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
		glEnableVertexAttribArray(1);
		glBindVertexArray(0);

		ctx[enemy] = per_draw{ va, shader, tex };
		++ok;
	}

	if (ok == NUM)
		return;
fail:
	std::print("[GFX] Failed to load graphics assets\n");
	std::exit(1);
}

render::~render()
{
	for (size_t obj = 0; obj < std::size(ctx); ++obj) {
		glDeleteVertexArrays(1, &ctx[obj].va);
		ctx[obj].tex.fini();
		ctx[obj].shader.fini();
	}
}

render::entity render::spawn(object type, per_entity settings)
{
	const auto e = nexte++ << ENT_SHIFT | type;
	const auto [addr, inserted] = data.emplace(e, settings);
	assert(inserted);
	return e;
}

void render::despawn(entity e)
{
	auto addr = data.find(e);
	assert(addr != data.end());
	data.erase(addr);
}

render::per_entity *render::access(entity e)
{
	auto addr = data.find(e);
	assert(addr != data.end());
	return &addr->second;
}

void render::draw(window const &to)
{
	const auto dims = to.dims();
	const glm::mat3 view = { { 1.0f, 0.0f, 0.0f }, { 0.0f, float(dims.x) / float(dims.y), 0.0f }, { 0.0f, 0.0f, 1.0f } };
	for (const auto &[e, this_entity]: data) {
		const auto obj = e & (1<<ENT_SHIFT)-1;
		const auto &this_draw = ctx[obj];
		this_draw.shader.bind();
		this_draw.tex.bind(0);
		glBindVertexArray(this_draw.va);
		glUniformMatrix3fv(glGetUniformLocation(this_draw.shader.id, "unif_view"), 1, GL_FALSE, glm::value_ptr(view));
		const glm::mat3 model = {
			{ this_entity.w, 0.0f         , 0.0f },
			{ 0.0f         , this_entity.h, 0.0f },
			{ this_entity.x, this_entity.y, 1.0f },
		};
		glUniformMatrix3fv(glGetUniformLocation(this_draw.shader.id, "unif_model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
}
