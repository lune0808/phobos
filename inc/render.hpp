#pragma once
#include <cstdint>
#include <unordered_map>
#include <glad/gl.h>
#include "shader.hpp"
#include "texture.hpp"
#include "window.hpp"
#include "phys.hpp"


class render
{
public:
	enum object {
		player,
		enemy,
		attack_cone,
		NUM
	};

	enum { ENT_SHIFT = 2 };
	enum { ENT_MASK = (1<<ENT_SHIFT)-1 };
	static_assert(NUM < (1<<ENT_SHIFT), "increase shift amount");

	using entity = std::uint32_t;

	struct per_entity : transform2d
	{
		struct flags_t {
			std::uint32_t colliding: 1;
		};

		flags_t &flags();
	};

private:
	struct per_draw
	{
		GLuint va;
		shader_pipeline shader;
		texture tex;
		GLuint tricount;
	};

	per_draw ctx[NUM];
	std::unordered_map<entity, per_entity> data[NUM];
	size_t ok;
	entity nexte;

public:
	std::vector<entity> despawning;
	struct {
		glm::vec2 pos;
		glm::vec2 dim;
	} camera;

	render(glm::vec2 campos, glm::vec2 camdim);
	~render();

	entity spawn(object type, per_entity const &settings);
	void despawn(entity e);
	per_entity *access(entity e);

	void draw();
};

