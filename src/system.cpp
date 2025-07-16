#include "system.hpp"

namespace phobos {
// single definition
global_systems system;
extern std::unordered_map<entity, std::uint32_t> g_entity_mapping[static_cast<size_t>(system_id::NUM)];

int init()
{
	system_id failure_point = system_id::NUM;
#define X(name) if (system.name.init() != 0) { \
			failure_point = system_id::name; \
			goto end; \
		}
	PHOBOS_SYSTEMS(X)
#undef X
end:
	return failure_point != system_id::NUM? -static_cast<int>(failure_point)-1: 0;
}

void fini()
{
#define X(name) system.name.fini();
	PHOBOS_SYSTEMS(X)
#undef X
}

void update(float now, float dt)
{
#define X(name) system.name.update(now, dt);
	PHOBOS_SYSTEMS(X)
#undef X
	update(); // entity index could be a system too
}

std::uint32_t index(entity e, system_id sys)
{
	auto addr = g_entity_mapping[static_cast<size_t>(sys)].find(e);
	assert(addr != g_entity_mapping[static_cast<size_t>(sys)].end());
	return addr->second;
}

void reindex(entity e, system_id sys, std::uint32_t idx)
{
	auto addr = g_entity_mapping[static_cast<size_t>(sys)].find(e);
	assert(addr != g_entity_mapping[static_cast<size_t>(sys)].end());
	addr->second = idx;
}

void add_component(entity e, system_id sys)
{
	auto [addr, ins] = g_entity_mapping[static_cast<size_t>(sys)].emplace(e, 0);
	assert(ins);
}

void del_component(entity e, system_id sys)
{
	auto addr = g_entity_mapping[static_cast<size_t>(sys)].find(e);
	assert(addr != g_entity_mapping[static_cast<size_t>(sys)].end());
	g_entity_mapping[static_cast<size_t>(sys)].erase(addr);
}

bool has_component(entity e, system_id sys)
{
	return g_entity_mapping[static_cast<size_t>(sys)].contains(e);
}

} // phobos
