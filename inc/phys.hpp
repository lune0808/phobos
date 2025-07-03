#pragma once
#include <glm/glm.hpp>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include "entity.hpp"

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

struct wall_mesh
{
	std::vector<glm::vec2> boundary; // [i;i+1modN] edges
	enum { bit = 3 };
};

struct ray_circle_intersection
{
	float time;
	bool has_intersection;
};

ray_circle_intersection collision_test(circle const &c, ray const &r);
bool collision_test(circle const &c, triangle const &t);
bool collision_test(ray const &r1, ray const &r2);
bool collision_test(circle const &c, wall_mesh const &m);

struct transform2d
{
	glm::mat3x2 repr;

	glm::vec2 &pos();
	glm::vec2 &x();
	glm::vec2 &y();
};

struct phys
{
	template <typename ...Colliders>
	struct mask {
		enum { value = ((1zu << Colliders::bit) | ...) };
	};

	template <typename ...Colliders>
	static constexpr size_t mask_v = mask<Colliders...>::value;

	template <typename T>
	struct collider : T
	{
		std::uint32_t mask;
	};

	// possible better representation
	// large array of (lhs.bit, rhs.bit)
	// sorted so that each bucket always
	// calls into the same one test
	// instead of storing the masks

	std::unordered_map<entity, collider<circle>> circle_;
	std::unordered_map<entity, collider<triangle>> triangle_;
	std::unordered_map<entity, collider<ray>> ray_;
	std::unordered_map<entity, collider<wall_mesh>> wall_mesh_;
	std::unordered_map<entity, glm::vec2> speed;

	void collider_circle(entity e, std::uint32_t collide_mask);
	void collider_triangle(entity e, std::uint32_t collide_mask);
	void collider_ray(entity e, std::uint32_t collide_mask);
	void collider_wall_mesh(entity e, wall_mesh const &m);
	void add_speed(entity e, glm::vec2 initial);
	glm::vec2 *get_speed(entity e);
	void update_colliders();

	std::unordered_set<entity> colliding;

	int init();
	void fini();
	void update(float now, float dt);
};

} // phobos

