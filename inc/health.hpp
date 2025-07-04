#pragma once
#include "entity.hpp"
#include <unordered_map>

namespace phobos {

struct hp_t {
	float current;
	float max;
	entity cooldown;
};

struct hp {
	int init();
	void fini();
	void clear();
	void update(float, float);

	std::unordered_map<entity, hp_t> living_;

	void damageable(entity e, float initial);
};

} // phobos

