#pragma once
#include "c++lib.hpp"
#include "entity.hpp"

namespace phobos {

// TODO: rename to fsm
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

	enum class event_t {
		collide_any,
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

struct dispatch
{
	int init();
	void fini();
	void update(float now, float dt);
	void remove(entity e);

	// TODO: maybe later on give the ability to listen
	// to collisions matching a certain tag mask

	// with = 0 means test for any collision
	void listen_collision(entity e, entity with, std::uint32_t payload);

	struct event_collision
	{
		entity e;
		std::uint32_t payload;
	};

	std::vector<event_collision> collision;

private:
	struct listening_collision_t {
		entity e;
		entity with;
		std::uint32_t payload;
	};

	std::vector<listening_collision_t> listening_collision;
};

} // phobos

