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
		const auto cur = rdr.access(e);
		const auto tgt = rdr.access(data.target);
		const auto delta = tgt->pos() - cur->pos();
		if (glm::length2(delta) < 1e-3)
			continue;
		const auto dir = glm::normalize(delta);
		const auto speed = 0.03f;
		cur->pos() += dt * speed * dir;
		cur->flags().colliding = false;
	}
	for (auto &[e, data] : colliding_) {
		const auto cur = rdr.access(e);
		const auto tgt = rdr.access(data.target);
		const circle tgt_hitbox{ tgt->pos(), tgt->x().x / 2.0f };
		const triangle cur_hitbox{ cur->pos(), cur->x(), cur->y() };
		const auto colliding = collision_test(tgt_hitbox, cur_hitbox);
		cur->flags().colliding = colliding;
		tgt->flags().colliding = colliding;
	}
}
