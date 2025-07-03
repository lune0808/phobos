#pragma once
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <glad/gl.h>
#include "shader.hpp"
#include "texture.hpp"
#include "window.hpp"
#include "phys.hpp"


class render
{
public:
	enum object {
		trail,
		enemy,
		attack_cone,
		player,
		NUM
	};

	enum { ENT_SHIFT = 2 };
	enum { ENT_MASK = (1<<ENT_SHIFT)-1 };
	static_assert(NUM <= (1<<ENT_SHIFT), "increase shift amount");

	using entity = std::uint32_t;

	struct per_entity : transform2d {};

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
	std::unordered_map<entity, per_entity> data[NUM];
	size_t ok;
	entity nexte;

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
	std::vector<entity> despawning;
	std::unordered_set<entity> colliding;
	struct {
		glm::vec2 pos;
		glm::vec2 dim;
	} camera;

	render(glm::vec2 campos, glm::vec2 camdim);
	~render();

	entity spawn(object type, per_entity const &settings);
	void despawn(entity e);
	per_entity *access(entity e);
	void add_trail(entity e, entity ref);

	void draw();
};

