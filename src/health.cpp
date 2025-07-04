#include "system.hpp"
#include <cassert>

namespace phobos {

int hp::init() { return 0; }
void hp::fini() {}
void hp::clear() { living_.clear(); }

void hp::update(float, float)
{
	for (const auto e : on_hold()) {
		living_.erase(e);
	}
	for (const auto &[e, col_data] : system.phys.colliding) {
		auto at = living_.find(e);
		if (at == living_.end())
			continue;
		if (!(col_data & (1u<<triangle::bit)))
			continue;
		auto &hp = at->second;
		if (system.tick.live(hp.cooldown))
			continue;
		// FIXME: this should come from the colliding entity
		hp.current -= 1.0f;
		if (hp.current > 0.0f) {
			hp.cooldown = spawn();
			system.tick.expire_in(hp.cooldown, {0.5f});
			continue;
		}
		despawn(e);
	}
}

void hp::damageable(entity e, float initial)
{
	auto [_, inserted] = living_.emplace(e, hp_t{initial, initial, 0});
	assert(inserted);
}

} // phobos

