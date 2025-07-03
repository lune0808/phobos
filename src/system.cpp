#include "system.hpp"

namespace phobos {
// single definition
global_systems system;

system_id init()
{
	system_id failure_point = system_id::none;
#define X(name) if (system.name.init() != 0) { \
			failure_point = system_id::name; \
			goto end; \
		}
	PHOBOS_SYSTEMS(X)
#undef X
end:
	return failure_point;
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

void clear()
{
#define X(name) system.name.clear();
	PHOBOS_SYSTEMS(X)
#undef X
}

} // phobos
