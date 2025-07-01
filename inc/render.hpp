#pragma once
#include <cstdint>
#include <unordered_map>
#include <glad/gl.h>
#include "shader.hpp"
#include "texture.hpp"
#include "window.hpp"


class render
{
public:
	enum object {
		player,
		enemy,
		NUM
	};

private:
	enum { ENT_SHIFT = 2 };
	static_assert(NUM < (1<<ENT_SHIFT), "increase shift amount");

	using entity = std::uint32_t;

	struct per_entity
	{
		float x;
		float y;
		float w;
		float h;
	};

	struct per_draw
	{
		GLuint va;
		shader_pipeline shader;
		texture tex;
	};

	per_draw ctx[NUM];
	std::unordered_map<entity, per_entity> data;
	size_t ok;
	entity nexte;

public:
	render();
	~render();

	entity spawn(object type, per_entity settings);
	void despawn(entity e);
	per_entity *access(entity e);

	void draw(window const &to);
};

