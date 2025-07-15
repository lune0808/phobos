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

bool tick::live(entity e)
{
	return expiring_.find(e) != expiring_.end();
}

void tick::follow(entity e, follow_t data)
{
	auto [_, inserted] = following_.emplace(e, data);
	assert(inserted);
}

int tick::init()
{
	return 0;
}

void tick::fini()
{
}

void tick::clear()
{
	expiring_.clear();
	following_.clear();
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
	}
	for (auto &[e, data] : following_) {
		auto cur = system.tfms.referential(e);
		auto tgt = system.tfms.world(data.target);
		const auto delta = tgt.pos() - cur->pos();
		if (glm::length2(delta) < 1e-3)
			continue;
		const auto dir = glm::normalize(delta);
		const auto speed = 0.03f;
		cur->pos() += dt * speed * dir;
	}
}

} // phobos
