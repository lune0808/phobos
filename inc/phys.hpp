#pragma once
#include <glm/glm.hpp>
#include <cstdint>

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

struct transform2d
{
	glm::mat3 repr;

	glm::vec2 &pos();
	glm::vec2 &x();
	glm::vec2 &y();

	using flags_t = std::uint32_t;
	flags_t &flags();
};

