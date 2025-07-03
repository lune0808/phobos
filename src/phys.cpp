#include "phys.hpp"
#include "render.hpp"
#include <glm/gtx/norm.hpp>

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

glm::vec2 &transform2d::pos()
{
	return *reinterpret_cast<glm::vec2*>(&repr[2]);
}

glm::vec2 &transform2d::x()
{
	return *reinterpret_cast<glm::vec2*>(&repr[0]);
}

glm::vec2 &transform2d::y()
{
	return *reinterpret_cast<glm::vec2*>(&repr[1]);
}


void phys::collider_circle(std::uint32_t e, std::uint32_t collide_mask)
{
	// dynamically updated
	auto [at, inserted] = circle_.emplace(e, collider<circle>{ ::circle{}, collide_mask });
	assert(inserted);
}

void phys::collider_triangle(std::uint32_t e, std::uint32_t collide_mask)
{
	auto [at, inserted] = triangle_.emplace(e, collider<triangle>{ ::triangle{}, collide_mask });
	assert(inserted);
}

void phys::update_colliders(render &rdr)
{
	for (const auto e : rdr.despawning) {
		circle_.erase(e);
		triangle_.erase(e);
	}
	for (auto &[e, col] : circle_) {
		const auto tfm = rdr.access(e);
		col.origin = tfm->pos();
		col.radius = tfm->x().x * 0.5f;
	}
	for (auto &[e, col] : triangle_) {
		const auto tfm = rdr.access(e);
		col.origin = tfm->pos();
		col.u = tfm->x();
		col.v = tfm->y();
	}
}

void phys::sim(render &rdr, float dt)
{
	update_colliders(rdr);
	for (auto &[e1, col1] : circle_) {
		if (col1.mask & (1zu<<::triangle::bit)) {
			for (auto &[e2, col2] : triangle_) {
				if (collision_test(col1, col2)) {
					rdr.colliding.emplace(e1);
					rdr.colliding.emplace(e2);
				}
			}
		}
	}
	for (auto &[e1, col1] : triangle_) {
		if (col1.mask & (1zu<<::circle::bit)) {
			for (auto &[e2, col2] : circle_) {
				if (collision_test(col2, col1)) {
					rdr.colliding.emplace(e1);
					rdr.colliding.emplace(e2);
				}
			}
		}
	}
}
