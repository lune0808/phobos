#pragma once
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <glad/gl.h>
#include "shader.hpp"
#include "texture.hpp"
#include "window.hpp"
#include "entity.hpp"
#include "phys.hpp"
#include "transform.hpp"

namespace phobos {

class render
{
public:
	enum object {
		trail,
		enemy,
		attack_cone,
		player,
		hp_bar,
		// wall_mesh :
		// offline, build a triangle (inside) mesh describing
		// where you can walk, ordered so that vertices i,i+1modN
		// are always edges along the boundary so collision test
		// can just ignore the triangle table,
		// and to draw it you clear screen with the "outside"
		// texture then draw the "inside" as the wall_mesh
		NUM
	};

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
	std::unordered_set<entity> drawing_[NUM];
	GLuint attack_cone_mesh_vb;

	struct quad {
		glm::vec2 base;
		glm::vec2 offs;
	};

	struct trail_t {
		quad buf[TRAIL_MAX_SEGMENTS];
		glm::vec2 timestamp[TRAIL_MAX_SEGMENTS];
		size_t insert;
		entity t;
	};

	struct {
		std::unordered_map<entity, trail_t> trailing_;
		GLuint wpos;
		GLuint ts;
	} trails;

public:
	glm::vec2 camera_pos;

	void drawable(entity e, object type);
	void trailable(entity e, entity ref);

	void update(float now, float dt);
	void clear();
	int init();
	void fini();
};

} // phobos

