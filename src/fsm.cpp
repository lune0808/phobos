#include "system.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>


namespace phobos {

int fsm::init()
{
	return 0;
}

void fsm::fini()
{
}

static void spawn_slash(glm::vec2 dir, entity en, fsm::enemy_dumb0 &data)
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

	data.slash_hand  = hand ;
	data.slash_cone  = cone ;
	data.slash_speed = cone_speed;
	data.slash_trail = trail;
}

static bool bit_test(std::uint32_t mask, std::uint32_t bit)
{
	return mask & (1u << bit);
}

static void transition(fsm::enemy_dumb0 &sm, std::uint32_t ev_mask, glm::vec2 pl_pos)
{
next:   switch (sm.state) {
	case fsm::just_spawned:
		if (bit_test(ev_mask, fsm::timeout)) {
			sm.state = fsm::idle;
			goto next;
		}
		break;
	case fsm::idle:
		if (bit_test(ev_mask, fsm::collide_fight_range)) {
			sm.state = fsm::combat_idle;
			goto next;
		} else if (bit_test(ev_mask, fsm::collide_sight_range)) {
			sm.state = fsm::move;
			goto next;
		}
		break;
	case fsm::move:
		if (bit_test(ev_mask, fsm::collide_fight_range)) {
			sm.state = fsm::combat_idle;
			goto next;
		}
		break;
	case fsm::combat_idle:
		if (!bit_test(ev_mask, fsm::collide_fight_range)) {
			sm.state = fsm::move;
			goto next;
		} else {
			sm.state = fsm::combat_attack_windup;
			const auto en_pos = system.tfms.world(sm.id).pos();
			const auto diff = pl_pos - en_pos;
			spawn_slash(glm::normalize(diff), sm.id, sm);
			system.tick.wait(sm.id, 0.2f);
		}
		break;
	case fsm::combat_attack_windup:
		if (bit_test(ev_mask, fsm::timeout)) {
			sm.state = fsm::combat_attack;
			const auto speed = 7.0f;
			const auto tfm = system.tfms.referential(sm.slash_speed);
			assert(tfm);
			tfm->x().y = speed;
			tfm->y().x = -speed;
			system.tick.wait(sm.id, 0.8f);
		}
		break;
	case fsm::combat_attack:
		if (bit_test(ev_mask, fsm::timeout)) {
			sm.state = fsm::combat_attack_cooldown;
			despawn(sm.slash_hand );
			sm.slash_hand  = 0;
			despawn(sm.slash_cone );
			sm.slash_cone  = 0;
			despawn(sm.slash_speed);
			sm.slash_speed = 0;
			despawn(sm.slash_trail);
			sm.slash_trail = 0;
			system.tick.wait(sm.id, 0.5f);
		}
		break;
	case fsm::combat_attack_cooldown:
		if (bit_test(ev_mask, fsm::timeout)) {
			sm.state = fsm::combat_idle;
			goto next;
		}
		break;
	default: assert(false);
	}
}

void fsm::update(float, float dt)
{
	const auto pl_pos = system.tfms.world(player).pos();
	for (size_t cidx = 0; cidx < system.dispatch.events.size(); ++cidx) {
		const auto event = system.dispatch.events[cidx];
		const auto fsm_idx = index(event.listen, system_id::fsm) >> type_shift;
		auto &fsm = fsm_enemy_dumb0[fsm_idx];
		transition(fsm, event.payload, pl_pos);
	}

	for (const auto &e : fsm_enemy_dumb0) {
		if (e.state == fsm::move) {
			const auto en_pos = system.tfms.world(e.id).pos();
			const auto diff = pl_pos - en_pos;
			const float speed = 1.5f;
			system.tfms.referential(e.id)->pos() += dt * speed * glm::normalize(diff);
		}
	}
}

void fsm::make_enemy_dumb0(entity e)
{
	const std::uint32_t type_idx = static_cast<std::uint32_t>(type::enemy_dumb0);
	const std::uint32_t idx = type_idx | fsm_enemy_dumb0.size() << type_shift;
	const auto fight_range = spawn();
	const auto sight_range = spawn();
	enemy_dumb0 repr{
		{ e, fsm::just_spawned },
		fight_range, sight_range,
		0, 0, 0, 0,
	};
	fsm_enemy_dumb0.emplace_back(repr);
	add_component(e, system_id::fsm);
	reindex(e, system_id::fsm, idx);
	system.dispatch_timeout.listen(e);
	system.tick.wait(e, 1.0f); // initial value
	system.dispatch.listen_collision(e, e, 0, 1u << fsm::collide_any);

	const auto range =
		+ 0.5f  // player radius
		+ 0.25f // enemy radius
		+ 0.6f  // enemy slash size
		- 0.1f  // margin
	;

	system.tfms.transformable(fight_range, range, glm::vec2{0.0f, 0.0f}, e);
	system.phys.collider_circle(fight_range);
	system.dispatch.listen_collision(e, fight_range, player, 1u << fsm::collide_fight_range);
	// system.render.drawable(fight_range, render::aggro);

	system.tfms.transformable(sight_range, 5.0f, glm::vec2{0.0f, 0.0f}, e);
	system.phys.collider_circle(sight_range);
	system.dispatch.listen_collision(e, sight_range, player, 1u << fsm::collide_sight_range);
	// system.render.drawable(sight_range, render::aggro);

}

void fsm::make_player(entity e)
{
	player = e;
}

void fsm::remove(entity e)
{
	const std::uint32_t idx = index(e, system_id::fsm);
	const std::uint32_t type_idx = idx & type_mask;
	const std::uint32_t swapped_idx = fsm_enemy_dumb0.size()-1;
	const std::uint32_t removed_idx = idx >> type_shift;
	fsm_enemy_dumb0[removed_idx] = fsm_enemy_dumb0[swapped_idx];
	reindex(fsm_enemy_dumb0[removed_idx].id, system_id::fsm, idx);
	del_component(e, system_id::fsm);
	fsm_enemy_dumb0.pop_back();
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
		const auto find_payload = 1u << fsm::timeout;
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

