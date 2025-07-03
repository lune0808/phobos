#pragma once
#include "entity.hpp"
#include "render.hpp"
#include "tick.hpp"
#include "phys.hpp"

namespace phobos {

#define PHOBOS_SYSTEMS(X) \
	X(tick) \
	X(phys) \
	X(render) \


enum class system_id {
	none,
#define X(name) name,
	PHOBOS_SYSTEMS(X)
#undef X
	NUM
};

extern struct global_systems
{
#define X(name) ::phobos::name name;
	PHOBOS_SYSTEMS(X)
#undef X
} system;

system_id init();
void fini();
void update(float now, float dt);

} // phobos


