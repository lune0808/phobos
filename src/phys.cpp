#include "phys.hpp"
#define GLM_ENABLE_EXPERIMENTAL
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
	if (glm::length2(rebased) <= c.radius * c.radius)
		return true;
	const auto side1 = glm::dot(t.u, rebased);
	const auto side2 = glm::dot(t.v, rebased);
	const auto side3 = side2-side1; // glm::dot(t.v-t.u, rebased)
	if (side1 == side2 && side2 == side3)
		return true;
	if (collision_test(c, ray{ t.origin, t.u }).has_intersection)
		return true;
	if (collision_test(c, ray{ t.origin, t.v }).has_intersection)
		return true;
	if (collision_test(c, ray{ t.origin+t.u, t.v-t.u }).has_intersection)
		return true;
	return false;
}


