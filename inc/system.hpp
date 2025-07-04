#pragma once
#include "entity.hpp"
#include "render.hpp"
#include "tick.hpp"
#include "phys.hpp"
#include "transform.hpp"
#include "health.hpp"

namespace phobos {

#define PHOBOS_SYSTEMS(X) \
	X(tick) /* tick decides if entities are live for this frame so it is always first */ \
	X(tfms) \
	X(phys) \
	X(hp) \
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
void clear();

} // phobos


