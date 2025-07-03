#pragma once
#include <glm/glm.hpp>
#include <cstdint>
#include <vector>
#include <unordered_map>

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

struct ray_circle_intersection
{
	float time;
	bool has_intersection;
};

struct wall_mesh
{
	std::vector<glm::vec2> boundary; // [i;i+1modN] edges

	enum { bit = 3 };
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

struct render;

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

	std::unordered_map<std::uint32_t, collider<circle>> circle_;
	std::unordered_map<std::uint32_t, collider<triangle>> triangle_;
	std::unordered_map<std::uint32_t, collider<ray>> ray_;
	std::unordered_map<std::uint32_t, collider<wall_mesh>> wall_mesh_;
	std::unordered_map<std::uint32_t, glm::vec2> speed;

	void collider_circle(std::uint32_t e, std::uint32_t collide_mask);
	void collider_triangle(std::uint32_t e, std::uint32_t collide_mask);
	void collider_ray(std::uint32_t e, std::uint32_t collide_mask);
	void collider_wall_mesh(std::uint32_t e, wall_mesh const &m);
	void add_speed(std::uint32_t e, glm::vec2 initial);
	glm::vec2 *get_speed(std::uint32_t e);
	void update_colliders(render &rdr);
	void sim(render &rdr, float dt);
};

