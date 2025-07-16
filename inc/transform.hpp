#pragma once
#include "entity.hpp"
#include <vector>
#include <glm/glm.hpp>

namespace phobos {

struct transform : glm::mat3x2
{
	glm::vec2 &pos();
	glm::vec2 &  x();
	glm::vec2 &  y();
	friend transform operator*(transform l, transform r);

	entity parent;
	entity id;
};

struct tfms
{
	int init();
	void fini();
	void update(float, float);
	void remove(entity e);

	std::vector<transform> data;

	void transformable(entity e, transform tfm);
	transform *referential(entity e);
	transform world(entity e);
};

} // phobos

