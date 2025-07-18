#pragma once
#include "c++lib.hpp"
#include <glad/gl.h>
#include "shader.hpp"
#include "texture.hpp"
#include "window.hpp"
#include "entity.hpp"
#include "phys.hpp"
#include "transform.hpp"

namespace phobos {

struct full_wall_mesh {
	wall_mesh pos;
	std::vector<glm::vec2> uv;
	std::vector<GLuint> indices;
};

class render
{
public:
	enum object {
		// offline, build a triangle (inside) mesh describing
		// where you can walk, ordered so that vertices i,i+1modN
		// are always edges along the boundary so collision test
		// can just ignore the triangle table,
		// and to draw it you clear screen with the "outside"
		// texture then draw the "inside" as the wall_mesh
		wall_mesh,
		aggro,
		trail,
		enemy,
		attack_cone,
		player,
		// entity's parent has the hp value
		hp_bar,
		NUM
	};

	enum : std::uint32_t { type_shift = 3, type_mask = (1<<type_shift) - 1 };
	static_assert(type_mask >= static_cast<std::uint32_t>(object::NUM)-1, "increase type_shift");

private:
	struct per_draw
	{
		GLuint va;
		shader_pipeline shader;
		texture tex;
		GLuint tricount;
	};

	enum : size_t { TRAIL_MAX_SEGMENTS = 128zu };

	per_draw ctx[NUM];
	std::vector<entity> drawing_[NUM];
	GLuint attack_cone_mesh_vb;

	struct quad {
		glm::vec2 base;
		glm::vec2 offs;
	};

	struct trail_t {
		quad buf[TRAIL_MAX_SEGMENTS];
		glm::vec2 timestamp[TRAIL_MAX_SEGMENTS];
		size_t insert;
		entity ref;
	};

	struct {
		std::vector<trail_t> trailing_;
		GLuint wpos;
		GLuint ts;
	} trails;

public:
	glm::vec2 camera_pos;

	void drawable(entity e, object type);
	void trailable(entity e, entity ref);
	void wall(entity e, full_wall_mesh const &mesh);

	void update(float now, float dt);
	int init();
	void fini();
	void remove(entity e);
};

full_wall_mesh load_wall_mesh();

} // phobos

