#pragma once
#include "entity.hpp"
#include <unordered_map>
#include <glm/glm.hpp>

namespace phobos {

struct transform : glm::mat3x2
{
	glm::vec2 &pos();
	glm::vec2 &  x();
	glm::vec2 &  y();
};

struct tfms
{
	int init();
	void fini();
	void update(float, float);

	std::unordered_map<entity, transform> data;

	void transformable(entity e, transform tfm);
	transform *get(entity e);
};

} // phobos

