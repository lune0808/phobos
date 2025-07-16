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
	const std::uint32_t type_idx = collider<circle>::bit;
	circle_.emplace_back(collider<circle>{ circle{}, collide_mask, e });
	const std::uint32_t idx = type_idx | circle_.size()-1 << type_shift;
	add_component(e, system_id::phys);
	reindex(e, system_id::phys, idx);
}

void phys::collider_triangle(entity e, std::uint32_t collide_mask)
{
	const std::uint32_t type_idx = collider<triangle>::bit;
	triangle_.emplace_back(collider<triangle>{ triangle{}, collide_mask, e });
	const std::uint32_t idx = type_idx | triangle_.size()-1 << type_shift;
	add_component(e, system_id::phys);
	reindex(e, system_id::phys, idx);
}

void phys::collider_ray(entity e, std::uint32_t collide_mask)
{
	const std::uint32_t type_idx = collider<ray>::bit;
	ray_.emplace_back(collider<ray>{ ray{}, collide_mask, e });
	const std::uint32_t idx = type_idx | ray_.size()-1 << type_shift;
	add_component(e, system_id::phys);
	reindex(e, system_id::phys, idx);
}

void phys::collider_wall_mesh(entity e, wall_mesh const &m)
{
	const std::uint32_t type_idx = collider<wall_mesh>::bit;
	wall_mesh_.emplace_back(collider<wall_mesh>{ wall_mesh{}, 0, e });
	const std::uint32_t idx = type_idx | wall_mesh_.size()-1 << type_shift;
	add_component(e, system_id::phys);
	reindex(e, system_id::phys, idx);
}

void phys::remove(entity e)
{
	const std::uint32_t idx = index(e, system_id::phys);
	const std::uint32_t type_idx = idx & type_mask;
	const std::uint32_t removed_idx = idx >> type_shift;
	std::uint32_t swapped_idx;
	switch (type_idx) {
	case collider<circle>::bit:
		swapped_idx = circle_.size()-1;
		circle_[removed_idx] = circle_[swapped_idx];
		circle_.pop_back();
		reindex(circle_[removed_idx].id, system_id::phys, idx);
		break;
	case collider<triangle>::bit:
		swapped_idx = triangle_.size()-1;
		triangle_[removed_idx] = triangle_[swapped_idx];
		triangle_.pop_back();
		reindex(triangle_[removed_idx].id, system_id::phys, idx);
		break;
	case collider<ray>::bit:
		swapped_idx = ray_.size()-1;
		ray_[removed_idx] = ray_[swapped_idx];
		ray_.pop_back();
		reindex(ray_[removed_idx].id, system_id::phys, idx);
		break;
	case collider<wall_mesh>::bit:
		swapped_idx = wall_mesh_.size()-1;
		wall_mesh_[removed_idx] = wall_mesh_[swapped_idx];
		wall_mesh_.pop_back();
		reindex(wall_mesh_[removed_idx].id, system_id::phys, idx);
		break;
	}
	del_component(e, system_id::phys);

}

int phys::init()
{
	circle_.emplace_back(collider<circle>{});
	triangle_.emplace_back(collider<triangle>{});
	ray_.emplace_back(collider<ray>{});
	wall_mesh_.emplace_back(collider<wall_mesh>{});
	return 0;
}

void phys::fini()
{
}

void phys::update_colliders()
{
	for (auto &col : circle_) {
		auto tfm = system.tfms.world(col.id);
		col.origin = tfm.pos();
		col.radius = tfm.x().x * 0.5f;
	}
	for (auto &col : triangle_) {
		auto tfm = system.tfms.world(col.id);
		col.origin = tfm.pos();
		col.u = tfm.x();
		col.v = tfm.y();
	}
	for (auto &col : ray_) {
		auto tfm = system.tfms.world(col.id);
		col.origin = tfm.pos();
		col.swept = tfm.x();
	}
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
	for (auto &col1 : circle_) {
		if (col1.mask & (1zu<<triangle::bit)) {
			for (auto &col2 : triangle_) {
				if (collision_test(col1, col2)) {
					collision(&colliding, col1.id, triangle::bit);
					collision(&colliding, col2.id, circle::bit);
				}
			}
		}
		if (col1.mask & (1zu<<ray::bit)) {
			for (auto &col2 : ray_) {
				if (collision_test(col1, col2).has_intersection) {
					collision(&colliding, col1.id, ray::bit);
					collision(&colliding, col2.id, circle::bit);
				}
			}
		}
		if (col1.mask & (1zu<<wall_mesh::bit)) {
			for (auto &col2 : wall_mesh_) {
				if (collision_test(col1, col2)) {
					collision(&colliding, col1.id, wall_mesh::bit);
					collision(&colliding, col2.id, circle::bit);
				}
			}
		}
	}
	for (auto &col1 : triangle_) {
		if (col1.mask & (1zu<<circle::bit)) {
			for (auto &col2 : circle_) {
				if (collision_test(col2, col1)) {
					collision(&colliding, col1.id, circle::bit);
					collision(&colliding, col2.id, triangle::bit);
				}
			}
		}
		if (col1.mask & (1zu<<ray::bit)) {
			// approximate tests assuming small triangles
			for (auto &col2 : ray_) {
				if (collision_test(ray{col1.origin, col1.u}, col2)) {
					collision(&colliding, col1.id, ray::bit);
					collision(&colliding, col2.id, triangle::bit);
				}
			}
		}
	}
}

int deriv::init()
{
	deriv_.emplace_back(0, 0);
	return 0;
}

void deriv::fini()
{
}

void deriv::update(float, float dt)
{
	for (const auto [x, xprime] : deriv_) {
		auto &tx = *system.tfms.referential(x);
		// FIXME: not accurate for rotations?
		const auto &txprime = *system.tfms.referential(xprime);
		tx += dt * (txprime * tx);
	}
}

void deriv::deriv_from(entity x, entity xprime)
{
	deriv_.emplace_back(x, xprime);
	add_component(x, system_id::deriv);
	reindex(x, system_id::deriv, deriv_.size()-1);
}

void deriv::remove(entity e)
{
	const std::uint32_t idx = index(e, system_id::deriv);
	const std::uint32_t swapped_idx = deriv_.size()-1;
	deriv_[idx] = deriv_[swapped_idx];
	reindex(deriv_[idx].first, system_id::deriv, idx);
	deriv_.pop_back();
	del_component(e, system_id::deriv);
}

} // phobos
