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

static void spawn_slash(glm::vec2 dir, entity en, enemy::enemy_t &data)
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
	const auto cone = spawn();
	system.render.drawable(cone, render::attack_cone);
	system.tfms.transformable(cone, {{swing, swing_tail, zero}, hand});
	const auto cone_speed = spawn();
	system.tfms.transformable(cone_speed, {{{0.0f, windspeed}, {-windspeed, 0.0f}, zero}, 0});
	system.deriv.deriv_from(cone, cone_speed);
	system.phys.collider_triangle(cone);
	const auto trail = spawn();
	system.tfms.transformable(trail, {});
	system.render.trailable(trail, cone);

	data.managed[enemy::enemy_t::slash_hand] = hand;
	data.managed[enemy::enemy_t::slash_cone] = cone;
	data.managed[enemy::enemy_t::slash_speed] = cone_speed;
	data.managed[enemy::enemy_t::slash_trail] = trail;
}

struct transition_t
{
	enemy::state_t next;
	std::uint32_t consumed;
};

// FIXME: find how a transition combat idle -> combat attack occurs
static transition_t transition(enemy::state_t s, enemy::event_t e)
{
	static const transition_t tbl[static_cast<size_t>(enemy::event_t::NUM)][static_cast<size_t>(enemy::state_t::NUM)] = {
		{
			{ enemy::state_t::just_spawned          , 1 },
			{ enemy::state_t::idle                  , 1 },
			{ enemy::state_t::move                  , 1 },
			{ enemy::state_t::combat_idle           , 1 },
			{ enemy::state_t::combat_attack_windup  , 1 },
			{ enemy::state_t::combat_attack         , 1 },
			{ enemy::state_t::combat_attack_cooldown, 1 },
		},
		{
			{ enemy::state_t::just_spawned          , 1 },
			{ enemy::state_t::combat_idle           , 2 },
			{ enemy::state_t::combat_idle           , 2 },
			{ enemy::state_t::combat_attack_windup  , 2 },
			{ enemy::state_t::combat_attack_windup  , 2 },
			{ enemy::state_t::combat_attack         , 2 },
			{ enemy::state_t::combat_attack_cooldown, 2 },
		},
		{
			{ enemy::state_t::just_spawned          , 1 },
			{ enemy::state_t::move                  , 1 },
			{ enemy::state_t::move                  , 1 },
			{ enemy::state_t::move                  , 1 },
			{ enemy::state_t::combat_attack_windup  , 1 },
			{ enemy::state_t::combat_attack         , 1 },
			{ enemy::state_t::combat_attack_cooldown, 1 },
		},
		{
			{ enemy::state_t::idle                  , 1 },
			{ enemy::state_t::idle                  , 1 },
			{ enemy::state_t::move                  , 1 },
			{ enemy::state_t::combat_idle           , 1 },
			{ enemy::state_t::combat_attack         , 1 },
			{ enemy::state_t::combat_attack_cooldown, 1 },
			{ enemy::state_t::combat_idle           , 1 },
		},
	};
	return tbl[static_cast<size_t>(e)][static_cast<size_t>(s)];
}

void enemy::update(float, float dt)
{
	std::vector<std::pair<std::uint32_t, std::uint32_t>> event;
	auto pl_pos = system.tfms.world(player).pos();
	for (size_t cidx = 0; cidx < system.dispatch.events.size(); ++cidx) {
		const auto event = system.dispatch.events[cidx];
		const auto fsm_idx = index(event.listen, system_id::enemy) >> type_shift;
		auto &fsm = enemy_[static_cast<size_t>(type_t::dumb0)][fsm_idx];
		const auto state = fsm.state;
		auto nstate = state;
		for (size_t ev = 0; ev < static_cast<size_t>(event_t::NUM); ) {
			if (event.payload & (1u << ev)) {
				const auto trans = transition(nstate, static_cast<event_t>(ev));
				nstate = trans.next;
				ev += trans.consumed;
			} else {
				++ev;
			}
		}
		if (state != nstate) switch (nstate) {
		case state_t::combat_attack_windup:
			{
			assert(event.listen == fsm.id);
			const auto en_pos = system.tfms.world(event.listen).pos();
			const auto diff = pl_pos - en_pos;
			spawn_slash(glm::normalize(diff), event.listen, fsm);
			system.tick.wait(event.listen, 0.2f);
			}
			break;
		case state_t::combat_attack:
			{
			const auto speed = 7.0f;
			const auto tfm = system.tfms.referential(fsm.managed[enemy_t::slash_speed]);
			assert(tfm);
			tfm->x().y = speed;
			tfm->y().x = -speed;
			system.tick.wait(event.listen, 0.8f);
			}
			break;
		case state_t::combat_attack_cooldown:
			system.tick.wait(event.listen, 0.5f);
			despawn(fsm.managed[enemy_t::slash_hand]);
			despawn(fsm.managed[enemy_t::slash_cone]);
			despawn(fsm.managed[enemy_t::slash_speed]);
			despawn(fsm.managed[enemy_t::slash_trail]);
			fsm.managed[enemy_t::slash_hand] = 0;
			fsm.managed[enemy_t::slash_cone] = 0;
			fsm.managed[enemy_t::slash_speed] = 0;
			fsm.managed[enemy_t::slash_trail] = 0;
			break;
		default:
			break;
		}

		fsm.state = nstate;
	}

	for (const auto &e : enemy_[static_cast<size_t>(type_t::dumb0)]) {
		if (e.state == state_t::move) {
			const auto en_pos = system.tfms.world(e.id).pos();
			const auto diff = pl_pos - en_pos;
			const float speed = 1.5f;
			system.tfms.referential(e.id)->pos() += dt * speed * glm::normalize(diff);
		}
	}
}

void enemy::make_enemy(entity e, type_t type)
{
	const std::uint32_t type_idx = static_cast<std::uint32_t>(type);
	const std::uint32_t idx = type_idx | enemy_[type_idx].size() << type_shift;
	const auto combat_range = spawn();
	const auto sight_range = spawn();
	enemy_t repr{ e, state_t::just_spawned };
	repr.managed[enemy_t::fight_range] = combat_range;
	repr.managed[enemy_t::sight_range] = sight_range;
	enemy_[type_idx].emplace_back(repr);
	add_component(e, system_id::enemy);
	reindex(e, system_id::enemy, idx);
	system.dispatch_timeout.listen(e);
	system.tick.wait(e, 1.0f); // initial value
	system.dispatch.listen_collision(e, e, 0, 1u << static_cast<size_t>(event_t::collide_any));

	const auto range =
		+ 0.5f  // player radius
		+ 0.25f // enemy radius
		+ 0.6f  // enemy slash size
		- 0.1f  // margin
	;

	system.tfms.transformable(combat_range, range, glm::vec2{0.0f, 0.0f}, e);
	system.phys.collider_circle(combat_range);
	system.dispatch.listen_collision(e, combat_range, player, 1u << static_cast<size_t>(event_t::collide_fight_range));
	// system.render.drawable(combat_range, render::aggro);

	system.tfms.transformable(sight_range, 5.0f, glm::vec2{0.0f, 0.0f}, e);
	system.phys.collider_circle(sight_range);
	system.dispatch.listen_collision(e, sight_range, player, 1u << static_cast<size_t>(event_t::collide_sight_range));
	// system.render.drawable(sight_range, render::aggro);

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
		const auto begin = std::cbegin(listening_collision);
		const auto end   = std::cend  (listening_collision);
		const auto match = [=] (auto elem)
		{
			return elem.e == cur.main && (elem.with == cur.other || !elem.with);
		};
		const auto find = std::find_if(begin, end, match);
		if (find == end)
			continue;

		const auto col_b = std::begin(events);
		const auto col_e = std::end  (events);
		const auto col_i = std::find_if(col_b, col_e, [=] (auto elem) { return elem.listen == find->listen; });
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

int dispatch_timeout::init()
{
	return 0;
}

void dispatch_timeout::fini()
{
}

void dispatch_timeout::update(float, float)
{
	for (size_t i = 0; i < system.tick.timeout.size(); ++i) {
		const auto cur = system.tick.timeout[i];
		const auto begin = std::cbegin(listening_time);
		const auto end   = std::cend  (listening_time);
		const auto find = std::find_if(begin, end, [=] (auto elem) { return elem.listen == cur; });
		if (find == end)
			continue;

		const auto col_b = std::begin(system.dispatch.events);
		const auto col_e = std::end  (system.dispatch.events);
		const auto col_i = std::find_if(col_b, col_e, [=] (auto elem) { return elem.listen == find->listen; });
		const auto find_payload = 1u << static_cast<size_t>(enemy::event_t::timeout);
		if (col_i == col_e) {
			system.dispatch.events.emplace_back(find->listen, find_payload);
		} else {
			col_i->payload |= find_payload;
		}
	}
}

void dispatch_timeout::remove(entity e)
{
	const std::uint32_t idx = index(e, system_id::dispatch_timeout);
	const std::uint32_t swapped_idx = listening_time.size()-1;
	listening_time[idx] = listening_time[swapped_idx];
	reindex(listening_time[idx].listen, system_id::dispatch_timeout, idx);
	del_component(e, system_id::dispatch_timeout);
	listening_time.pop_back();
}

void dispatch_timeout::listen(entity listen)
{
	listening_time.emplace_back(listen);
	add_component(listen, system_id::dispatch_timeout);
	reindex(listen, system_id::dispatch_timeout, listening_time.size()-1);
}

} // phobos

