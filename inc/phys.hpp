#pragma once
#include <glm/glm.hpp>

struct circle
{
	glm::vec2 origin;
	float radius;
};

struct ray
{
	glm::vec2 origin;
	glm::vec2 swept;
};

struct triangle
{
	glm::vec2 origin;
	glm::vec2 u;
	glm::vec2 v;
};

struct ray_circle_intersection
{
	float time;
	bool has_intersection;
};

ray_circle_intersection collision_test(circle const &c, ray const &r);
bool collision_test(circle const &c, triangle const &t);

