#include "system.hpp"
#include <cstring>
#include <cassert>
#include <print>
#include <string_view>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>


namespace phobos {

static constexpr size_t MAX_TRAILS = 16zu;

static GLuint describe_layout_f2f2(void *data, size_t size)
{
	GLuint va;
	glGenVertexArrays(1, &va);
	glBindVertexArray(va);
	GLuint vb;
	glGenBuffers(1, &vb);
	glBindBuffer(GL_ARRAY_BUFFER, vb);
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(0*sizeof(float)));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
	return va;
}

struct trail_buffers {
	GLuint va;
	GLuint wpos;
	GLuint ts;
};

static trail_buffers describe_layout_trail(size_t segment_count)
{
	std::vector<glm::vec2> uv;
	std::vector<GLuint> tri;
	uv.emplace_back(0.0f, 0.0f);
	uv.emplace_back(0.0f, 1.0f);
	for (size_t i = 1; i < segment_count; ++i) {
		const auto prog = static_cast<float>(i) / static_cast<float>(segment_count-1);
		uv.emplace_back(prog, 0.0f);
		uv.emplace_back(prog, 1.0f);
		const auto end = uv.size();
		// -1 -3
		// -2 -4
		// provoking vertex is the last of a triangle
		// and we want to take the "newest" one so
		// that old trails do not show
		tri.emplace_back(end-2);
		tri.emplace_back(end-4);
		tri.emplace_back(end-3);
		tri.emplace_back(end-1);
		tri.emplace_back(end-2);
		tri.emplace_back(end-3);
	}

	trail_buffers buf;
	glGenVertexArrays(1, &buf.va);
	glBindVertexArray(buf.va);
	GLuint vb;
	glGenBuffers(1, &vb);
	glBindBuffer(GL_ARRAY_BUFFER, vb);
	glBufferData(GL_ARRAY_BUFFER, uv.size() * sizeof uv[0], uv.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &buf.wpos);
	glBindBuffer(GL_ARRAY_BUFFER, buf.wpos);
	glBufferData(GL_ARRAY_BUFFER, uv.size() * 2*sizeof(float), nullptr, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);
	glGenBuffers(1, &buf.ts);
	glBindBuffer(GL_ARRAY_BUFFER, buf.ts);
	glBufferData(GL_ARRAY_BUFFER, uv.size() * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(2);
	GLuint ib;
	glGenBuffers(1, &ib);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, tri.size() * sizeof tri[0], tri.data(), GL_STATIC_DRAW);
	glBindVertexArray(0);
	return buf;
}

int render::init()
{
	camera.pos.x = 0.0f;
	camera.pos.y = 0.0f;
	camera.dim.x = 800.0f;
	camera.dim.y = 600.0f;
	size_t ok = 0;
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
		"uniform float unif_red_shift;\n"
		"void main() {\n"
			"vec4 color = texture(unif_color, vert_uv);\n"
			"color.r += unif_red_shift;\n"
			"color.g -= unif_red_shift;\n"
			"color.b -= unif_red_shift;\n"
			"frag_color = color;\n"
		"}\n\0"sv,
	};
	glm::mat3 id(1.0f);
	shader.bind();
	glUniformMatrix3fv(glGetUniformLocation(shader.id, "unif_model"), 1, GL_FALSE, glm::value_ptr(id));
	glUniformMatrix3fv(glGetUniformLocation(shader.id, "unif_view" ), 1, GL_FALSE, glm::value_ptr(id));

	shader_pipeline trail_shader{
		"#version 410 core\n"
		"layout(location=0) in vec2 attr_wpos;\n"
		"layout(location=1) in vec2 attr_uv;\n"
		"layout(location=2) in float attr_timestamp;\n"
		"out vec2 vert_uv;\n"
		"flat out float vert_scale;\n"
		"uniform float now;\n"
		"uniform float max_dt;\n"
		"uniform mat3 unif_view;\n"
		"void main() {\n"
		"	vert_uv = attr_uv;\n"
		"	vert_scale = 1.0 - min(max_dt, now - attr_timestamp) / max_dt;\n"
		"	vec3 pos = unif_view * vec3(attr_wpos, 1.0);\n"
		"	gl_Position = vec4(pos.xy, 0.0, 1.0);\n"
		"}\n\0"sv,

		"#version 410 core\n"
		"in vec2 vert_uv;\n"
		"flat in float vert_scale;\n"
		"out vec4 frag_color;\n"
		"uniform sampler2D unif_color;\n"
		"void main() {\n"
		"	vec4 color = texture(unif_color, vert_uv);\n"
		"	frag_color = vert_scale * color;\n"
		"}\n\0"sv
	};
	if (!shader.ok()) goto fail;
	if (!trail_shader.ok()) goto fail;
	trail_shader.bind();
	glUniformMatrix3fv(glGetUniformLocation(trail_shader.id, "unif_model"), 1, GL_FALSE, glm::value_ptr(id));
	glUniformMatrix3fv(glGetUniformLocation(trail_shader.id, "unif_view" ), 1, GL_FALSE, glm::value_ptr(id));

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
		GLuint va = describe_layout_f2f2(vdata, sizeof vdata);
		ctx[player] = per_draw{ va, shader, tex, 6 };
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
		GLuint va = describe_layout_f2f2(vdata, sizeof vdata);
		ctx[enemy] = per_draw{ va, shader, tex, 6 };
		++ok;
	}

	{
		unsigned char fill[4] = { 0xf2, 0xde, 0xe3, 0xff };
		image img;
		img.base = fill;
		img.width = 1;
		img.height = 1;
		img.channels = 4;
		texture tex{img, shader, "unif_color\0"sv};

		float vdata[] = {
			0.0f, 0.0f, 0.0f, 0.0f,
			// (1-A)v+Au to spread the cone over a larger area (framerate dependent)
			6.0f,-5.0f, 1.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 1.0f,
		};
		GLuint va = describe_layout_f2f2(vdata, sizeof vdata);
		ctx[attack_cone] = per_draw{ va, shader, tex, 3 };
		++ok;
	}

	{
		unsigned char fill[4] = { 0xf2, 0xde, 0xe3, 0x80 };
		image img;
		img.base = fill;
		img.width = 1;
		img.height = 1;
		img.channels = 4;
		texture tex{img, trail_shader, "unif_color\0"sv};
		auto buf = describe_layout_trail(TRAIL_MAX_SEGMENTS);
		ctx[trail] = per_draw{ buf.va, trail_shader, tex, (TRAIL_MAX_SEGMENTS-1)*6 };
		trails.wpos = buf.wpos;
		trails.ts = buf.ts;
		++ok;
	}

	if (ok == NUM)
		return 0;
fail:
	std::print("[GFX] Failed to load graphics assets\n");
	return 1;
}

void render::fini()
{
	for (size_t obj = 0; obj < std::size(ctx); ++obj) {
		glDeleteVertexArrays(1, &ctx[obj].va);
		ctx[obj].tex.fini();
		ctx[obj].shader.fini();
	}
}

void render::drawable(entity e, object type)
{
	const auto [addr, inserted] = drawing_[type].emplace(e);
	assert(inserted);
}

void render::clear()
{
	for (size_t obj = 0; obj < NUM; ++obj) {
		drawing_[obj].clear();
	}
	trails.trailing_.clear();
}

void render::update(float now, float)
{
	for (const auto e : on_hold()) {
		for (size_t obj = 0; obj < NUM; ++obj) {
			drawing_[obj].erase(e);
		}
		trails.trailing_.erase(e);
	}
	for (auto &[e, data] : trails.trailing_) {
		const auto ref = system.tfms.get(e);
		data.buf[data.insert].base = ref->pos();
		data.buf[data.insert].offs = ref->pos() + ref->y();
		data.timestamp[data.insert] = glm::vec2{now, now};
		data.insert = (data.insert+1) % TRAIL_MAX_SEGMENTS;
	}
	const auto aspect_ratio = camera.dim.x / camera.dim.y;
	// translate THEN scale, so the scale is also applied to the offsets
	const glm::mat3 view{
		{ 1.0f        , 0.0f                       , 0.0f },
		{ 0.0f        , aspect_ratio               , 0.0f },
		{ camera.pos.x, camera.pos.y * aspect_ratio, 1.0f },
	};
	for (size_t obj = 0; obj < NUM; ++obj) {
		const auto &this_draw = ctx[obj];
		this_draw.shader.bind();
		this_draw.tex.bind(0);
		glBindVertexArray(this_draw.va);
		glUniformMatrix3fv(glGetUniformLocation(this_draw.shader.id, "unif_view"), 1, GL_FALSE, glm::value_ptr(view));
		if (obj != trail) for (const auto e: drawing_[obj]) {
			auto &this_entity = *system.tfms.get(e);
			const glm::mat3 model{
				glm::vec3{this_entity.  x(), 0.0f},
				glm::vec3{this_entity.  y(), 0.0f},
				glm::vec3{this_entity.pos(), 1.0f},
			};
			glUniformMatrix3fv(glGetUniformLocation(this_draw.shader.id, "unif_model"),
					1, GL_FALSE, glm::value_ptr(model));
			const auto red_shift = system.phys.colliding.contains(e)? 0.3f: 0.0f;
			glUniform1f(glGetUniformLocation(this_draw.shader.id, "unif_red_shift"), red_shift);
			glDrawArrays(GL_TRIANGLES, 0, this_draw.tricount);
		} else {
			glUniform1f(glGetUniformLocation(this_draw.shader.id, "now"), now);
			glUniform1f(glGetUniformLocation(this_draw.shader.id, "max_dt"), 0.3f);
			for (auto &[e, data] : trails.trailing_) {
				const auto to_end = (TRAIL_MAX_SEGMENTS-data.insert);
				const auto from_start = data.insert;
				glBindBuffer(GL_ARRAY_BUFFER, trails.wpos);
				glBufferSubData(GL_ARRAY_BUFFER, 0, to_end * sizeof data.buf[0], &data.buf[data.insert]);
				glBufferSubData(GL_ARRAY_BUFFER, to_end * sizeof data.buf[0], from_start * sizeof data.buf[0], &data.buf[0]          );
				glBindBuffer(GL_ARRAY_BUFFER, trails.ts);
				glBufferSubData(GL_ARRAY_BUFFER, 0, to_end * sizeof(float[2]), &data.timestamp[data.insert]);
				glBufferSubData(GL_ARRAY_BUFFER, to_end * sizeof(float[2]), from_start * sizeof(float[2]), &data.timestamp[0]          );
				glDrawElements(GL_TRIANGLES, this_draw.tricount, GL_UNSIGNED_INT, nullptr);
			}
		}
	}
}

void render::trailable(entity t, entity ref)
{
	// timestamp being 0.0 means effectively nothing is drawn
	auto [at, inserted] = trails.trailing_.emplace(ref, trail_t{});
	assert(inserted);
	auto trail = &at->second;
	trail->t = t;
}

} // phobos
