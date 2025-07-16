#include "system.hpp"

namespace phobos {
// single definition
global_systems system;
extern std::unordered_map<entity, entity_desc> g_entity_mapping;

int init()
{
	spawn(); // 0 entity
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
	auto addr = g_entity_mapping.find(e);
	assert(addr != g_entity_mapping.end());
	return addr->second.index[static_cast<size_t>(sys)];
}

void reindex(entity e, system_id sys, std::uint32_t idx)
{
	auto addr = g_entity_mapping.find(e);
	assert(addr != g_entity_mapping.end());
	addr->second.index[static_cast<size_t>(sys)] = idx;
}

void add_component(entity e, system_id sys)
{
	auto addr = g_entity_mapping.find(e);
	assert(addr != g_entity_mapping.end());
	addr->second.mask |= 1ul << static_cast<std::uint32_t>(sys);
}

void del_component(entity e, system_id sys)
{
	auto addr = g_entity_mapping.find(e);
	assert(addr != g_entity_mapping.end());
	addr->second.mask &= ~(1ul << static_cast<std::uint32_t>(sys));
}

bool has_component(entity e, system_id sys)
{
	auto addr = g_entity_mapping.find(e);
	assert(addr != g_entity_mapping.end());
	return addr->second.mask & (1ul << static_cast<std::uint32_t>(sys));
}

} // phobos
