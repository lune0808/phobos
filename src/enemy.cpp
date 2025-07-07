#include "system.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>


namespace phobos {

int enemy::init()
{
	return 0;
}

void enemy::fini()
{
}

void enemy::clear()
{
	for (size_t type = 0; type < static_cast<size_t>(type_t::NUM); ++type) {
		enemy_[type].clear();
	}
}

struct transition_t {
	enemy::state_t next;
	float wait;
};

enum class dumb0_event {
	player_far,
	player_visible,
	player_close,
	NUM
};

static transition_t transition(enemy::state_t cur, dumb0_event evt)
{
	static const transition_t tbl[static_cast<size_t>(dumb0_event::NUM)][static_cast<size_t>(enemy::state_t::NUM)] =
	{
		{
			{ enemy::state_t::idle, 0.1f },
			{ enemy::state_t::idle, 0.1f },
			{ enemy::state_t::idle, 0.1f },
			{ enemy::state_t::idle, 0.1f },
		},
		{
			{ enemy::state_t::move, 0.5f },
			{ enemy::state_t::move, 0.5f },
			{ enemy::state_t::move, 0.0f },
			{ enemy::state_t::move, 0.2f },
		},
		{
			{ enemy::state_t::combat_idle, 0.1f },
			{ enemy::state_t::combat_idle, 0.1f },
			{ enemy::state_t::combat_attack, 0.4f },
			{ enemy::state_t::combat_idle, 0.2f },
		},
	};
	return tbl[static_cast<size_t>(evt)][static_cast<size_t>(cur)];
}

static void spawn_slash(glm::vec2 dir, entity en)
{
	static const float lifetime = 0.2f;
	const auto cone = spawn();
	system.render.drawable(cone, render::attack_cone);
	system.tfms.transformable(cone, {{{1.0f,0.0f},{0.0f,1.0f},0.5f*dir}, en});
	system.tick.expire_in(cone, {lifetime});
	system.tick.spin(cone, {0.6f / 0.5f * dir});
	system.phys.collider_triangle(cone, phys::mask_v<circle>);
	const auto trail = spawn();
	system.render.drawable(cone, render::trail);
	system.tfms.transformable(trail, {});
	system.tick.expire_in(trail, {lifetime});
	system.render.trailable(trail, cone);
}

void enemy::update(float, float dt)
{
	for (const auto e : on_hold()) {
		for (size_t type = 0; type < static_cast<size_t>(type_t::NUM); ++type) {
			enemy_[type].erase(e);
		}
	}
	auto pl_pos = system.tfms.world(player).pos();
	for (auto &[id, en] : enemy_[static_cast<size_t>(type_t::dumb0)]) {
		auto en_pos = system.tfms.world(id).pos();
		const auto diff = pl_pos - en_pos;
		const auto len2 = glm::length2(diff);
		const auto range =
			+ 0.5f  // player radius
			+ 0.25f // enemy radius
			+ 0.6f  // enemy slash size
			- 0.2f  // margin
		;
		const auto evt = (len2 < range*range) ? dumb0_event::player_close:
				 (len2 < 5.0f * 5.0f) ? dumb0_event::player_visible:
				 dumb0_event::player_far;
		const auto trans = transition(en.state, evt);
		en.elapsed += dt;
		if (en.elapsed > trans.wait) {
			en.state = trans.next;
			en.elapsed = 0.0f;
			if (trans.next == state_t::combat_attack) {
				spawn_slash(glm::normalize(diff), id);
			}
		}
		if (en.state == enemy::state_t::move) {
			const float speed = 0.1f;
			system.tfms.referential(id)->pos() += dt * speed * glm::normalize(diff);
		}
	}
}

void enemy::make_enemy(entity e, type_t type)
{
	auto [_, ins] = enemy_[static_cast<size_t>(type)].emplace(e, enemy_t{state_t::idle, 0.0f});
	assert(ins);
}

void enemy::make_player(entity e)
{
	player = e;
}

} // phobos

