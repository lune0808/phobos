#include "tick.hpp"
#include "phys.hpp"
#include <cassert>
#define GLM_ENABLE_EXPERIMENTAL
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
		colliding_.erase(e);
	}
	for (auto &[e, data] : following_) {
		const auto pos = rdr.access(e);
		const auto tgt = rdr.access(data.target);
		const auto delta = tgt->transform[2] - pos->transform[2];
		if (glm::length2(delta) < 1e-3)
			continue;
		const auto dir = glm::normalize(glm::vec2(delta.x, delta.y));
		const auto speed = 0.03f;
		pos->transform[2][0] += dt * speed * dir.x;
		pos->transform[2][1] += dt * speed * dir.y;
		pos->flags().colliding = false;
	}
	for (auto &[e, data] : colliding_) {
		const auto pos = rdr.access(e);
		const auto tgt = rdr.access(data.target);
		const circle tgt_hitbox{
			{ tgt->transform[2][0], tgt->transform[2][1] },
			tgt->transform[0][0] / 2.0f,
		};
		const triangle pos_hitbox{
			{ pos->transform[2][0], pos->transform[2][1] },
			{ pos->transform[0][0], pos->transform[0][1] },
			{ pos->transform[1][0], pos->transform[1][1] },
		};
		const auto colliding = collision_test(tgt_hitbox, pos_hitbox);
		pos->flags().colliding = colliding;
		tgt->flags().colliding = colliding;
	}
}
