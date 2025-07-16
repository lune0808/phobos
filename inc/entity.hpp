#pragma once
#include "c++lib.hpp"

namespace phobos {

using entity = std::uint32_t;

entity spawn();
void despawn(entity);
void update();

} // phobos

