#pragma once
#include "c++lib.hpp"
#include "entity.hpp"

namespace phobos {

struct enemy {
	enum class type_t {
		dumb0,
		NUM
	};

	enum : std::uint32_t { type_shift = 1, type_mask = (1<<type_shift) - 1 };
	static_assert(type_mask >= static_cast<std::uint32_t>(type_t::NUM)-1, "increase type_shift");

	enum class state_t {
		idle,
		move,
		combat_idle,
		combat_attack_windup,
		combat_attack,
		combat_attack_cooldown,
		NUM
	};

	struct enemy_t {
		entity id;
		state_t state;
		entity slash_speed;
		float elapsed;
	};

	std::vector<enemy_t> enemy_[static_cast<size_t>(type_t::NUM)];
	entity player;

	void make_enemy(entity e, type_t type);
	void make_player(entity e);

	int init();
	void fini();
	void update(float now, float dt);
	void remove(entity e);
};

} // phobos

