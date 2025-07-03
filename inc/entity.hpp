#pragma once
#include <cstdint>
#include <vector>

namespace phobos {

using entity = std::uint32_t;

entity spawn();
void despawn(entity);
void update();
std::vector<entity> const &on_hold();

} // phobos

