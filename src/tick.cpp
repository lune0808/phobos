#include "c++lib.hpp"
#include "system.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

namespace phobos {

void tick::expire_in(entity e, expire_in_t data)
{
	data.id = e;
	expiring_.emplace_back(data);
	add_component(e, system_id::tick);
	reindex(e, system_id::tick, expiring_.size()-1);
}

int tick::init()
{
	expiring_.emplace_back(std::numeric_limits<float>::infinity(), 0);
	return 0;
}

void tick::fini()
{
}

void tick::update(float, float dt)
{
	for (auto &data : expiring_) {
		if ((data.remaining -= dt) <= 0.0f) {
			despawn(data.id);
		}
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
