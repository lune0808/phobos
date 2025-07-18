#pragma once
#include "c++lib.hpp"
#include "entity.hpp"
#include "vector.hpp"

namespace phobos {

struct fsm {
	enum type : std::uint32_t {
		enemy_dumb0,
		slash,
		type_num
	};

	enum : std::uint32_t { type_shift = 1, type_mask = (1<<type_shift) - 1 };
	static_assert(type_mask >= type_num-1, "increase type_shift");

	enum state_t : std::uint32_t {
		just_spawned,
		idle,
		move,
		combat_idle,
		combat_attack,
		combat_attack_cooldown,
		state_num
	};

	enum event_t : std::uint32_t {
		collide_any,
		collide_fight_range,
		collide_sight_range,
		timeout,
		die,
		event_num
	};

	struct state_machine
	{
		entity id;
		state_t state;
	};

	struct slash_t : state_machine
	{
		entity cone;
		entity speed;
		entity trail;
	};

	struct enemy_dumb0_t : state_machine
	{
		entity fight_range;
		entity sight_range;
		entity slash;
	};

	union {
		phobos::vector<enemy_dumb0_t> enemy_dumb0;
		phobos::vector<slash_t      > slash      ;
	} fsms[type_num];

	entity player;

	void make_enemy_dumb0(entity e);
	void make_slash(entity e, entity cone, entity speed, entity trail);
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
	void listen_collision(entity listen, entity e, entity with, std::uint32_t payload);

	struct event
	{
		entity listen;
		std::uint32_t payload;
	};

	std::vector<event> events;

private:
	struct listening_collision_t {
		entity listen;
		entity e;
		entity with;
		std::uint32_t payload;
	};

	std::vector<listening_collision_t> listening_collision;
};

struct dispatch_timeout
{
	int init();
	void fini();
	void update(float, float);
	void remove(entity);

	void listen(entity listen);

	struct listening_time_t {
		entity listen;
	};

	std::vector<listening_time_t> listening_time;
};

struct dispatch_death
{
	int init();
	void fini();
	void update(float, float);
	void remove(entity);

	void listen(entity listen, entity to);

	struct listening_death_t {
		entity listen;
		entity to;
	};

	std::vector<listening_death_t> listening_death;
};

} // phobos

