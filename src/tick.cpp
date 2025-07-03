#include "system.hpp"
#include <cassert>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

namespace phobos {

void tick::expire_in(entity e, expire_in_t data)
{
	auto [_, inserted] = expiring_.emplace(e, data);
	assert(inserted);
}

void tick::follow(entity e, follow_t data)
{
	auto [_, inserted] = following_.emplace(e, data);
	assert(inserted);
}

void tick::spin(entity e, spin_t data)
{
	auto [_, inserted] = spinning_.emplace(e, data);
	assert(inserted);
}

int tick::init()
{
	return 0;
}

void tick::fini()
{
}

void tick::update(float, float dt)
{
	for (auto &[e, data] : expiring_) {
		if ((data.remaining -= dt) <= 0.0f) {
			despawn(e);
		}
	}
	for (const auto e : on_hold()) {
		expiring_.erase(e);
		following_.erase(e);
		spinning_.erase(e);
	}
	for (auto &[e, data] : following_) {
		const auto cur = system.tfms.get(e);
		const auto tgt = system.tfms.get(data.target);
		const auto delta = tgt->pos() - cur->pos();
		if (glm::length2(delta) < 1e-3)
			continue;
		const auto dir = glm::normalize(delta);
		const auto speed = 0.03f;
		cur->pos() += dt * speed * dir;
	}
	const auto speed = 15.0f;
	// Rcos(A+B)=(RcosA)cosB-(RsinA)sinB
	// Rsin(A+B)=(RcosA)sinB+(RsinA)cosB
	// this accumulates errors but it is fine because
	// entities don't spin for a long time
	const auto cos_dtheta = std::cos(speed * dt);
	const auto sin_dtheta = std::sin(speed * dt);
	for (auto &[e, data] : spinning_) {
		const auto cos_next = data.state.x * cos_dtheta
			            - data.state.y * sin_dtheta;
		const auto sin_next = data.state.x * sin_dtheta
			            + data.state.y * cos_dtheta;
		const auto state_next = glm::vec2{cos_next, sin_next};
		const auto cur = system.tfms.get(e);
		cur->x() = data.state;
		cur->y() = state_next;
		data.state = state_next;
	}
}

} // phobos
