#pragma once
#include "entity.hpp"
#include <unordered_map>

namespace phobos {

struct enemy {
	enum class type_t {
		dumb0,
		NUM
	};

	enum class state_t {
		idle,
		move,
		combat_idle,
		combat_attack,
		combat_attack_cooldown,
		NUM
	};

	struct enemy_t {
		state_t state;
		float elapsed;
	};

	std::unordered_map<entity, enemy_t> enemy_[static_cast<size_t>(type_t::NUM)];
	entity player;

	void make_enemy(entity e, type_t type);
	void make_player(entity e);

	int init();
	void fini();
	void update(float now, float dt);
	void clear();
};

} // phobos

