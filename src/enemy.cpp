#include "system.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>


namespace phobos {

int enemy::init()
{
	for (size_t type = 0; type < static_cast<size_t>(type_t::NUM); ++type) {
		enemy_[type].emplace_back(0, state_t::idle, 0, 0.0f);
	}
	return 0;
}

void enemy::fini()
{
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
			{ enemy::state_t::idle, 0.1f },
			{ enemy::state_t::idle, 0.1f },
		},
		{
			{ enemy::state_t::move, 0.5f },
			{ enemy::state_t::move, 0.5f },
			{ enemy::state_t::move, 0.1f },
			{ enemy::state_t::combat_attack, 0.2f },
			{ enemy::state_t::combat_attack_cooldown, 0.8f },
			{ enemy::state_t::combat_idle, 0.5f },
		},
		{
			{ enemy::state_t::combat_idle, 0.1f },
			{ enemy::state_t::combat_idle, 0.1f },
			{ enemy::state_t::combat_attack_windup, 0.0f },
			{ enemy::state_t::combat_attack, 0.2f },
			{ enemy::state_t::combat_attack_cooldown, 0.8f },
			{ enemy::state_t::combat_idle, 0.5f },
		},
	};
	return tbl[static_cast<size_t>(evt)][static_cast<size_t>(cur)];
}

static entity spawn_slash(glm::vec2 dir, entity en)
{
	const auto at = 0.51f * dir;
	const auto angle = glm::radians(-90.0f);
	const auto windspeed = -1.5f;
	const auto cos = std::cos(angle);
	const auto sin = std::sin(angle);
	const auto swing = 0.6f/0.5f * glm::vec2{cos*dir.x - sin*dir.y, sin*dir.x + cos*dir.y};
	const auto delay = 20.0f;
	const auto cos2 = std::cos(glm::radians(delay));
	const auto sin2 = std::sin(glm::radians(delay));
	const auto swing_tail = glm::vec2{cos2*swing.x - sin2*swing.y, sin2*swing.x + cos2*swing.y};
	const auto speed = 7.0f;
	const auto lifetime = 0.2f + (glm::radians(+100.0f)-angle-windspeed*0.2f) / speed;

	const glm::vec2 zero{0.0f, 0.0f};
	const glm::vec2 x{1.0f, 0.0f};
	const glm::vec2 y{0.0f, 1.0f};

	const auto hand = spawn();
	system.tfms.transformable(hand, {{x, y, at}, en});
	system.tick.expire_in(hand, {lifetime});
	// TODO: find a better way to manage these objects' lifetime
	const auto cone = spawn();
	system.render.drawable(cone, render::attack_cone);
	system.tfms.transformable(cone, {{swing, swing_tail, zero}, hand});
	system.tick.expire_in(cone, {lifetime});
	const auto cone_speed = spawn();
	system.tfms.transformable(cone_speed, {{{0.0f, windspeed}, {-windspeed, 0.0f}, zero}, 0});
	system.deriv.deriv_from(cone, cone_speed);
	system.tick.expire_in(cone_speed, {lifetime});
	system.phys.collider_triangle(cone, phys::mask_v<circle>);
	const auto trail = spawn();
	system.tfms.transformable(trail, {});
	system.tick.expire_in(trail, {lifetime});
	system.render.trailable(trail, cone);

	return cone_speed;
}

void enemy::update(float, float dt)
{
	auto pl_pos = system.tfms.world(player).pos();
	for (size_t idx = 1; idx < enemy_[static_cast<size_t>(type_t::dumb0)].size(); ++idx) {
		auto &e = enemy_[static_cast<size_t>(type_t::dumb0)][idx];
		auto en_pos = system.tfms.world(e.id).pos();
		const auto diff = pl_pos - en_pos;
		const auto len2 = glm::length2(diff);
		const auto range =
			+ 0.5f  // player radius
			+ 0.25f // enemy radius
			+ 0.6f  // enemy slash size
			- 0.1f  // margin
		;
		const auto evt = (len2 < range*range) ? dumb0_event::player_close:
				 (len2 < 5.0f * 5.0f) ? dumb0_event::player_visible:
				 dumb0_event::player_far;
		const auto trans = transition(e.state, evt);
		e.elapsed += dt;
		if (e.elapsed > trans.wait) {
			e.state = trans.next;
			e.elapsed = 0.0f;
			if (trans.next == state_t::combat_attack_windup) {
				auto s = spawn_slash(glm::normalize(diff), e.id);
				e.slash_speed = s;
			} else if (trans.next == state_t::combat_attack) {
				const auto speed = 7.0f;
				const auto tfm = system.tfms.referential(e.slash_speed);
				assert(tfm);
				tfm->x().y = speed;
				tfm->y().x = -speed;
			}
		}
		if (e.state == enemy::state_t::move) {
			const float speed = 1.5f;
			system.tfms.referential(e.id)->pos() += dt * speed * glm::normalize(diff);
		}
	}
}

void enemy::make_enemy(entity e, type_t type)
{
	const std::uint32_t type_idx = static_cast<std::uint32_t>(type);
	const std::uint32_t idx = type_idx | enemy_[type_idx].size() << type_shift;
	enemy_[type_idx].emplace_back(e, state_t::idle, 0, 0.0f);
	add_component(e, system_id::enemy);
	reindex(e, system_id::enemy, idx);
}

void enemy::make_player(entity e)
{
	player = e;
}

void enemy::remove(entity e)
{
	const std::uint32_t idx = index(e, system_id::enemy);
	const std::uint32_t type_idx = idx & type_mask;
	const std::uint32_t swapped_idx = enemy_[type_idx].size()-1;
	const std::uint32_t removed_idx = idx >> type_shift;
	enemy_[type_idx][removed_idx] = enemy_[type_idx][swapped_idx];
	reindex(enemy_[type_idx][removed_idx].id, system_id::enemy, idx);
	del_component(e, system_id::enemy);
	enemy_[type_idx].pop_back();
}

} // phobos

