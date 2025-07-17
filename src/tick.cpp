#include "c++lib.hpp"
#include "system.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

namespace phobos {

void tick::wait(entity e, float seconds)
{
	expiring_.emplace_back(seconds, e);
	add_component(e, system_id::tick);
	reindex(e, system_id::tick, expiring_.size()-1);
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
	timeout.clear();
	for (auto &data : expiring_) {
		if ((data.remaining -= dt) <= 0.0f) {
			timeout.emplace_back(data.id);
		}
	}
	for (auto e : timeout) {
		remove(e);
	}
}

void tick::remove(entity e)
{
	const std::uint32_t idx = index(e, system_id::tick);
	const std::uint32_t swapped_idx = expiring_.size()-1;
	expiring_[idx] = expiring_[swapped_idx];
	reindex(expiring_[idx].id, system_id::tick, idx);
	expiring_.pop_back();
	del_component(e, system_id::tick);
}

} // phobos
