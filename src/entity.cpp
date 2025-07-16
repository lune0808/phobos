#include "entity.hpp"
#include "system.hpp"
#include <vector>
#include <unordered_map>

namespace phobos {

static entity g_cur;
std::unordered_map<entity, std::uint32_t> g_entity_mapping[static_cast<size_t>(system_id::NUM)];
static std::vector<entity> g_on_hold;

entity spawn()
{
	return g_cur++;
}

void despawn(entity e)
{
	assert(e);
	g_on_hold.push_back(e);
}

void update()
{
	for (const auto e : g_on_hold) {
#define X(name) if (has_component(e, system_id::name)) system.name.remove(e);
		PHOBOS_SYSTEMS(X)
#undef X
		for (size_t type = 0; type < std::size(g_entity_mapping); ++type) {
			g_entity_mapping[type].erase(e);
		}
	}
	g_on_hold.clear();
}

bool live(entity e)
{
	return e && g_entity_mapping[static_cast<size_t>(system_id::tick)].contains(e);
}

} // phobos

