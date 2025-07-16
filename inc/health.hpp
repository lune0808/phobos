#pragma once
#include "c++lib.hpp"
#include "entity.hpp"

namespace phobos {

struct hp_t {
	float current;
	float max;
	// TODO: should be part of enemy behavior
	entity cooldown;
};

struct hp {
	int init();
	void fini();
	void update(float, float);
	void remove(entity e);

	std::vector<hp_t> living_;
	std::vector<entity> id;

	void damageable(entity e, float initial);
};

} // phobos

