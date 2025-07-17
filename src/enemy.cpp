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

enum class dumb0_event {
	player_far,
	player_visible,
	player_close,
	NUM
};

#if 0
struct transition_t {
	enemy::state_t next;
	float wait;
};

static transition_t transition(enemy::state_t cur, dumb0_event evt)
{
	static const transition_t tbl[dumb0_event::NUM][enemy::state_t::NUM] =
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
	return tbl[evt][cur];
}
#endif

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
	system.phys.collider_triangle(cone);
	const auto trail = spawn();
	system.tfms.transformable(trail, {});
	system.tick.expire_in(trail, {lifetime});
	system.render.trailable(trail, cone);

	return cone_speed;
}

struct transition_t
{
	enemy::state_t next;
	std::uint32_t consumed;
};

static transition_t transition(enemy::state_t s, enemy::event_t e)
{
	static const transition_t tbl[static_cast<size_t>(enemy::event_t::NUM)][static_cast<size_t>(enemy::state_t::NUM)] = {
		{
			{ enemy::state_t::idle       , 1 },
			{ enemy::state_t::move       , 1 },
			{ enemy::state_t::combat_idle, 1 },
			{ enemy::state_t::combat_idle, 1 },
			{ enemy::state_t::combat_idle, 1 },
			{ enemy::state_t::combat_idle, 1 },
		},
		{
			{ enemy::state_t::combat_idle, 2 },
			{ enemy::state_t::combat_idle, 2 },
			{ enemy::state_t::combat_idle, 2 },
			{ enemy::state_t::combat_idle, 2 },
			{ enemy::state_t::combat_idle, 2 },
			{ enemy::state_t::combat_idle, 2 },
		},
		{
			{ enemy::state_t::move       , 1 },
			{ enemy::state_t::move       , 1 },
			{ enemy::state_t::move       , 1 },
			{ enemy::state_t::move       , 1 },
			{ enemy::state_t::move       , 1 },
			{ enemy::state_t::move       , 1 },
		},
		{
			{ enemy::state_t::idle       , 1 },
			{ enemy::state_t::move       , 1 },
			{ enemy::state_t::combat_idle, 1 },
			{ enemy::state_t::combat_idle, 1 },
			{ enemy::state_t::combat_idle, 1 },
			{ enemy::state_t::combat_idle, 1 },
		},
	};
	return tbl[static_cast<size_t>(e)][static_cast<size_t>(s)];
}

void enemy::update(float, float dt)
{
	std::vector<std::pair<std::uint32_t, std::uint32_t>> event;
	for (size_t cidx = 0; cidx < system.dispatch.events.size(); ++cidx) {
		const auto event = system.dispatch.events[cidx];
		const auto fsm_idx = index(event.listen, system_id::enemy) >> type_shift;
		auto &fsm = enemy_[static_cast<size_t>(type_t::dumb0)][fsm_idx];
		auto state = fsm.state;
		for (size_t ev = 0; ev < static_cast<size_t>(event_t::NUM); ) {
			if (event.payload & (1u << ev)) {
				const auto trans = transition(state, static_cast<event_t>(ev));
				state = trans.next;
				ev += trans.consumed;
			} else {
				++ev;
			}
		}
		fsm.state = state;
	}

	auto pl_pos = system.tfms.world(player).pos();
	for (const auto &e : enemy_[static_cast<size_t>(type_t::dumb0)]) {
		if (e.state == state_t::move) {
			const auto en_pos = system.tfms.world(e.id).pos();
			const auto diff = pl_pos - en_pos;
			const float speed = 1.5f;
			system.tfms.referential(e.id)->pos() += dt * speed * glm::normalize(diff);
		}
	}

#if 0
	auto pl_pos = system.tfms.world(player).pos();
	std::vector<dumb0_event> event;
	event.reserve(enemy_[type_t::dumb0].size()-1);
	for (size_t idx = 1; idx < enemy_[type_t::dumb0].size(); ++idx) {
		auto &e = enemy_[type_t::dumb0][idx];
		const auto en_pos = system.tfms.world(e.id).pos();
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
		event.emplace_back(evt);
	}
	for (size_t idx = 1; idx < enemy_[type_t::dumb0].size(); ++idx) {
		auto &e = enemy_[type_t::dumb0][idx];
		const auto trans = transition(e.state, event[idx-1]);
		const auto elapsed = e.elapsed += dt;
		if (elapsed > trans.wait) {
			e.state = trans.next;
			e.elapsed = 0.0f;
			if (trans.next == state_t::combat_attack_windup) {
				const auto en_pos = system.tfms.world(e.id).pos();
				const auto diff = pl_pos - en_pos;
				const auto s = spawn_slash(glm::normalize(diff), e.id);
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
			const auto en_pos = system.tfms.world(e.id).pos();
			const auto diff = pl_pos - en_pos;
			const float speed = 1.5f;
			system.tfms.referential(e.id)->pos() += dt * speed * glm::normalize(diff);
		}
	}
#endif
}

void enemy::make_enemy(entity e, type_t type)
{
	const std::uint32_t type_idx = static_cast<std::uint32_t>(type);
	const std::uint32_t idx = type_idx | enemy_[type_idx].size() << type_shift;
	enemy_[type_idx].emplace_back(e, state_t::idle, 0);
	add_component(e, system_id::enemy);
	reindex(e, system_id::enemy, idx);
	system.dispatch.listen_collision(e, e, 0, 1u << static_cast<size_t>(event_t::collide_any));

	const auto range =
		+ 0.5f  // player radius
		+ 0.25f // enemy radius
		+ 0.6f  // enemy slash size
		- 0.1f  // margin
	;
	const auto combat_range = spawn();
	system.tfms.transformable(combat_range, range, glm::vec2{0.0f, 0.0f}, e);
	system.phys.collider_circle(combat_range);
	system.dispatch.listen_collision(e, combat_range, player, 1u << static_cast<size_t>(event_t::collide_fight_range));
	// system.render.drawable(combat_range, render::aggro);

	const auto sight_range = spawn();
	system.tfms.transformable(sight_range, 3.0f, glm::vec2{0.0f, 0.0f}, e);
	system.phys.collider_circle(sight_range);
	system.dispatch.listen_collision(e, sight_range, player, 1u << static_cast<size_t>(event_t::collide_sight_range));
	system.render.drawable(sight_range, render::aggro);
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

int dispatch::init()
{
	return 0;
}

void dispatch::fini()
{
}

void dispatch::update(float, float)
{
	events.clear();
	// TODO: sort arrays for more efficient access
	//  O(N2*M) -> O(NlogN + MlogM + max(N,M))
	//  with N = # colliding and M = # listening
	//
	//  FIXME: collisions should be consumed once seen
	//  and wildcards should be tested last
	for (size_t i = 0; i < system.phys.colliding.size(); ++i) {
		const auto cur = system.phys.colliding[i];
		auto begin = std::cbegin(listening_collision);
		auto end   = std::cend  (listening_collision);
		const auto match = [=] (auto elem)
		{
			return elem.e == cur.main && (elem.with == cur.other || !elem.with);
		};
		auto find = std::find_if(begin, end, match);
		if (find == end)
			continue;

		auto col_b = std::begin(events);
		auto col_e = std::end  (events);
		auto col_i = std::find_if(col_b, col_e, [=] (auto elem) { return elem.listen == find->listen; });
		if (col_i == col_e) {
			events.emplace_back(find->listen, find->payload);
		} else {
			col_i->payload |= find->payload;
		}
	}
}

void dispatch::remove(entity e)
{
	const std::uint32_t idx = index(e, system_id::dispatch);
	const std::uint32_t swapped_idx = listening_collision.size()-1;
	listening_collision[idx] = listening_collision[swapped_idx];
	reindex(listening_collision[idx].e, system_id::dispatch, idx);
	del_component(e, system_id::dispatch);
	listening_collision.pop_back();
}

void dispatch::listen_collision(entity listen, entity e, entity with, std::uint32_t payload)
{
	listening_collision.emplace_back(listen, e, with, payload);
	add_component(e, system_id::dispatch);
	reindex(e, system_id::dispatch, listening_collision.size()-1);
}

} // phobos

