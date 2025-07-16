#pragma once
#include "c++lib.hpp"
#include <glm/glm.hpp>
#include "entity.hpp"
#include "transform.hpp"

namespace phobos {

struct circle
{
	glm::vec2 origin;
	float radius;
	enum { bit = 0 };
};

struct ray
{
	glm::vec2 origin;
	glm::vec2 swept;
	enum { bit = 1 };
};

struct triangle
{
	glm::vec2 origin;
	glm::vec2 u;
	glm::vec2 v;
	enum { bit = 2 };
};

struct wall_mesh : std::vector<glm::vec2> // [i;i+1modN] edges
{
	enum { bit = 3 };
};

bool collision_test(circle const &c, ray const &r);
bool collision_test(circle const &c, triangle const &t);
bool collision_test(ray const &r1, ray const &r2);
bool collision_test(circle const &c, wall_mesh const &m);

struct deriv
{
	int init();
	void fini();
	void update(float now, float dt);
	void remove(entity e);

	std::vector<std::pair<entity, entity>> deriv_;

	void deriv_from(entity x, entity xprime);
};

struct phys
{
	template <typename T>
	struct collider : T
	{
		entity id;
	};

	// possible better representation
	// large array of (lhs.bit, rhs.bit)
	// sorted so that each bucket always
	// calls into the same one test
	// instead of storing the masks

	// TODO: somehow make that an array
	std::vector<collider<circle>> circle_;
	std::vector<collider<triangle>> triangle_;
	std::vector<collider<ray>> ray_;
	std::vector<collider<wall_mesh>> wall_mesh_;

	enum : std::uint32_t { type_shift = 2, type_mask = (1<<type_shift) - 1 };
	static_assert(type_mask >= static_cast<std::uint32_t>(collider<wall_mesh>::bit), "increase type_shift");

	void collider_circle(entity e);
	void collider_triangle(entity e);
	void collider_ray(entity e);
	void collider_wall_mesh(entity e, wall_mesh const &m);
	std::uint32_t collider_type(entity e);
	void update_colliders();

	struct collision_data
	{
		entity main;
		entity other;
	};

	std::vector<collision_data> colliding;

	int init();
	void fini();
	void update(float now, float dt);
	void remove(entity e);
};

} // phobos

