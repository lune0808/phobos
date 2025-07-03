#include "tick.hpp"
#include "phys.hpp"
#include <cassert>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

void tick::expire_in(render::entity e, expire_in_t data)
{
	auto [_, inserted] = expiring_.emplace(e, data);
	assert(inserted);
}

void tick::follow(render::entity e, follow_t data)
{
	auto [_, inserted] = following_.emplace(e, data);
	assert(inserted);
}

void tick::collide_test(render::entity e, collide_test_t data)
{
	auto [_, inserted] = colliding_.emplace(e, data);
	assert(inserted);
}

void tick::spin(render::entity e, spin_t data)
{
	auto [_, inserted] = spinning_.emplace(e, data);
	assert(inserted);
}

void tick::update(float dt)
{
	for (auto &[e, data] : expiring_) {
		if ((data.remaining -= dt) <= 0.0f) {
			rdr.despawn(e);
		}
	}
	for (const auto e : rdr.despawning) {
		expiring_.erase(e);
		following_.erase(e);
		spinning_.erase(e);
		colliding_.erase(e);
	}
	for (auto &[e, data] : following_) {
		const auto cur = rdr.access(e);
		const auto tgt = rdr.access(data.target);
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
		const auto cur = rdr.access(e);
		cur->x() = data.state;
		cur->y() = state_next;
		data.state = state_next;
	}
	for (auto &[e, data] : colliding_) {
		const auto cur = rdr.access(e);
		const auto tgt = rdr.access(data.target);
		const circle tgt_hitbox{ tgt->pos(), tgt->x().x / 2.0f };
		const triangle cur_hitbox{ cur->pos(), cur->x(), cur->y() };
		const auto colliding = collision_test(tgt_hitbox, cur_hitbox);
		if (!colliding)
			continue;
		rdr.colliding.emplace(e);
		rdr.colliding.emplace(data.target);
	}
}
