#include "system.hpp"
#include <glm/gtx/norm.hpp>

namespace phobos {

ray_circle_intersection collision_test(circle const &c, ray const &r)
{
	const auto diff = r.origin - c.origin;
	const auto swept2 = glm::length2(r.swept);
	const auto diff2 = glm::length2(diff);
	const auto dot = glm::dot(diff, r.swept);
	const auto delta_over_4 = dot * dot - swept2 * (diff2 - c.radius * c.radius);
	ray_circle_intersection result;
	if (delta_over_4 < 0) {
		result.has_intersection = false;
		return result;
	}
	const auto root = std::sqrt(delta_over_4);
	const auto time_lo = (-dot - root) / swept2;
	const auto time_hi = (-dot + root) / swept2;
	if (time_hi < 0.0f) {
		result.has_intersection = false;
		return result;
	} else if (time_lo > 1.0f) {
		result.has_intersection = false;
		return result;
	}
	result.has_intersection = true;
	if (time_hi <= 1.0f) {
		result.time = time_hi;
		return result;
	} else {
		result.time = time_lo;
		return result;
	}
}

bool collision_test(circle const &c, triangle const &t)
{
	const auto rebased = c.origin - t.origin;
	// triangle in circle
	if (glm::length2(rebased) <= c.radius * c.radius)
		return true;
	const auto side1 = glm::dot(t.u, rebased);
	const auto side2 = glm::dot(t.v, rebased);
	const auto side3 = glm::dot(t.v-t.u, c.origin - (t.origin+t.u));
	// circle in triangle
	if (side1 > 0.0f && side2 < 0.0f && side3 > 0.0f)
		return true;
	// triangle edge on circle boundary
	if (collision_test(c, ray{ t.origin, t.u }).has_intersection)
		return true;
	if (collision_test(c, ray{ t.origin, t.v }).has_intersection)
		return true;
	if (collision_test(c, ray{ t.origin+t.u, t.v-t.u }).has_intersection)
		return true;
	return false;
}

glm::vec2 &transform2d::pos() { return repr[2]; }
glm::vec2 &transform2d::  x() { return repr[0]; }
glm::vec2 &transform2d::  y() { return repr[1]; }

bool collision_test(ray const &r1, ray const &r2)
{
	const auto diff = r2.origin - r1.origin;
	static constexpr float epsilon = 1e-6;
	const glm::mat2 system{ r1.swept, r2.swept };
	const auto det = glm::determinant(system);
	// parallel lines. may be the same but whatever
	if (glm::abs(det) < epsilon)
		return false;
	const auto [t1, minus_t2] = glm::inverse(system) * diff;
	// if t1,t2 are in [0,1], collision at rN.O + tN * rN.S
	return (0.0f <= t1 && t1 <= 1.0f)
	    && (0.0f >= minus_t2 && minus_t2 >= -1.0f);
}

bool collision_test(circle const &c, wall_mesh const &m)
{
	const ray edge{
		m.boundary.back(),
		m.boundary[0]-m.boundary.back(),
	};
	if (collision_test(c, edge).has_intersection)
		return true;
	for (size_t i = 0; i < m.boundary.size()-1; ++i) {
		const ray edge{
			m.boundary[i],
			m.boundary[i+1]-m.boundary[i],
		};
		if (collision_test(c, edge).has_intersection) {
			return true;
		}
	}
	return false;
}

void phys::collider_circle(entity e, std::uint32_t collide_mask)
{
	// dynamically updated
	auto [at, inserted] = circle_.emplace(e, collider<circle>{ circle{}, collide_mask });
	assert(inserted);
}

void phys::collider_triangle(entity e, std::uint32_t collide_mask)
{
	auto [at, inserted] = triangle_.emplace(e, collider<triangle>{ triangle{}, collide_mask });
	assert(inserted);
}

void phys::collider_ray(entity e, std::uint32_t collide_mask)
{
	auto [at, inserted] = ray_.emplace(e, collider<ray>{ ray{}, collide_mask });
	assert(inserted);
}

void phys::collider_wall_mesh(entity e, wall_mesh const &m)
{
	auto [at, inserted] = wall_mesh_.emplace(e, collider<wall_mesh>{m, 0});
	assert(inserted);
}

void phys::add_speed(entity e, glm::vec2 initial)
{
	auto [at, inserted] = speed.emplace(e, initial);
	assert(inserted);
}

glm::vec2 *phys::get_speed(entity e)
{
	auto at = speed.find(e);
	return at != speed.end()? &at->second: nullptr;
}

int phys::init()
{
	return 0;
}

void phys::fini()
{
}

void phys::update_colliders()
{
	for (const auto e : on_hold()) {
		circle_.erase(e);
		triangle_.erase(e);
		ray_.erase(e);
		wall_mesh_.erase(e);
		speed.erase(e);
	}
	for (auto &[e, col] : circle_) {
		const auto tfm = system.tfms.get(e);
		col.origin = tfm->pos();
		col.radius = tfm->x().x * 0.5f;
	}
	for (auto &[e, col] : triangle_) {
		const auto tfm = system.tfms.get(e);
		col.origin = tfm->pos();
		col.u = tfm->x();
		col.v = tfm->y();
	}
	for (auto &[e, col] : ray_) {
		const auto tfm = system.tfms.get(e);
		col.origin = tfm->pos();
		col.swept = tfm->x();
	}
}

void phys::clear()
{
	circle_.clear();
	triangle_.clear();
	ray_.clear();
	wall_mesh_.clear();
	speed.clear();
	colliding.clear();
}

static void collision(std::unordered_map<entity, std::uint32_t> *map, entity e, std::uint32_t ibit)
{
	const auto mask = std::uint32_t{1u}<<ibit;
	auto [at, inserted] = map->emplace(e, mask);
	if (!inserted) {
		at->second |= mask;
	}
}

void phys::update(float, float dt)
{
	colliding.clear();
	update_colliders();
	for (auto &[e1, col1] : circle_) {
		if (col1.mask & (1zu<<triangle::bit)) {
			for (auto &[e2, col2] : triangle_) {
				if (collision_test(col1, col2)) {
					collision(&colliding, e1, triangle::bit);
					collision(&colliding, e2, circle::bit);
				}
			}
		}
		if (col1.mask & (1zu<<ray::bit)) {
			for (auto &[e2, col2] : ray_) {
				if (collision_test(col1, col2).has_intersection) {
					collision(&colliding, e1, ray::bit);
					collision(&colliding, e2, circle::bit);
				}
			}
		}
		if (col1.mask & (1zu<<wall_mesh::bit)) {
			for (auto &[e2, col2] : wall_mesh_) {
				if (collision_test(col1, col2)) {
					collision(&colliding, e1, wall_mesh::bit);
					collision(&colliding, e2, circle::bit);
				}
			}
		}
	}
	for (auto &[e1, col1] : triangle_) {
		if (col1.mask & (1zu<<circle::bit)) {
			for (auto &[e2, col2] : circle_) {
				if (collision_test(col2, col1)) {
					collision(&colliding, e1, circle::bit);
					collision(&colliding, e2, triangle::bit);
				}
			}
		}
		if (col1.mask & (1zu<<ray::bit)) {
			// approximate tests assuming small triangles
			for (auto &[e2, col2] : ray_) {
				if (collision_test(ray{col1.origin, col1.u}, col2)) {
					collision(&colliding, e1, ray::bit);
					collision(&colliding, e2, triangle::bit);
				}
			}
		}
	}
}

} // phobos
