#include "c++lib.hpp"
#include "system.hpp"

namespace phobos {

int hp::init()
{
	return 0;
}

void hp::fini() {}

void hp::update(float, float)
{
	for (const auto &[e, other] : system.phys.colliding) {
		if (!has_component(e, system_id::hp))
			continue;
		const auto other_type = system.phys.collider_type(other);
		if (other_type != triangle::bit)
			continue;
		auto &hp = living_[index(e, system_id::hp)];
		if (has_component(hp.cooldown, system_id::tick))
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

void hp::remove(entity e)
{
	const std::uint32_t idx = index(e, system_id::hp);
	const std::uint32_t swapped_idx = living_.size()-1;
	living_[idx] = living_[swapped_idx];
	id[idx] = id[swapped_idx];
	del_component(e, system_id::hp);
	reindex(id[idx], system_id::hp, idx);
	living_.pop_back();
	id.pop_back();
}

void hp::damageable(entity e, float initial)
{
	living_.emplace_back(initial, initial, 0);
	id.emplace_back(e);
	add_component(e, system_id::hp);
	reindex(e, system_id::hp, living_.size()-1);
}

} // phobos

