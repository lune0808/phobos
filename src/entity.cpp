#include "entity.hpp"

namespace phobos {

static entity g_cur;
static std::vector<entity> g_on_hold;

entity spawn()
{
	return ++g_cur;
}

void despawn(entity e)
{
	g_on_hold.push_back(e);
}

void update()
{
	g_on_hold.clear();
}

std::vector<entity> const &on_hold()
{
	return g_on_hold;
}

} // phobos

