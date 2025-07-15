#pragma once
#include "entity.hpp"
#include "render.hpp"
#include "tick.hpp"
#include "phys.hpp"
#include "transform.hpp"
#include "health.hpp"
#include "enemy.hpp"
#include "input.hpp"

namespace phobos {

#define PHOBOS_SYSTEMS(X) \
	X(input) \
	X(tick) /* tick decides if entities are live for this frame so it is always first */ \
	X(tfms) \
	X(enemy) \
	X(phys) \
	X(deriv) \
	X(hp) \
	X(render) \
	X(gl) \


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


