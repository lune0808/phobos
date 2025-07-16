#include "entity.hpp"
#include "system.hpp"
#include <vector>
#include <unordered_map>

namespace phobos {

static entity g_cur;
std::unordered_map<entity, entity_desc> g_entity_mapping;
static std::vector<entity> g_on_hold;

entity spawn()
{
	const auto e = g_cur++;
	g_entity_mapping.emplace(e, entity_desc{});
	return e;
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
		g_entity_mapping.erase(e);
	}
	g_on_hold.clear();
}

bool live(entity e)
{
	return e && g_entity_mapping.contains(e);
}

} // phobos

