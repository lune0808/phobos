#include "window.hpp"
#include "system.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <string_view>
#include <print>

inline auto &ng = phobos::system;

phobos::transform tri_transform(glm::vec2 origin, glm::vec2 u, glm::vec2 v, phobos::entity parent = 0)
{
	return {{ u, v, origin }, parent};
}

phobos::transform quad_transform(glm::vec2 origin, glm::vec2 dims, phobos::entity parent = 0)
{
	return tri_transform(origin, {dims.x,0.0f}, {0.0f,dims.y}, parent);
}

phobos::entity cooldown(float seconds)
{
	const auto cd = phobos::spawn();
	ng.tick.expire_in(cd, {seconds});
	return cd;
}

phobos::entity spawn_slash(phobos::entity player)
{
	const auto cone = phobos::spawn();
	const auto trail = phobos::spawn();
	ng.render.drawable(cone, phobos::render::object::attack_cone);
	ng.render.drawable(trail, phobos::render::object::trail);
	ng.tfms.transformable(cone, quad_transform({0.55f,0.0f}, {1.0f,1.0f}, player));
	ng.tfms.transformable(trail, {});
	const float lifetime = 0.2f;
	ng.tick.expire_in(cone, {lifetime});
	ng.tick.expire_in(trail, {lifetime});
	ng.tick.spin(cone, {{0.2f,-0.4f}});
	ng.render.trailable(trail, cone);
	ng.phys.collider_triangle(cone, phobos::phys::mask_v<phobos::circle>);
	return cooldown(0.5f);
}

phobos::entity player_control(window const &win, phobos::entity attack, phobos::entity player, float dt)
{
	const auto handle = win.get_handle();
	glm::vec2 offset{ 0.0f, 0.0f };
	if (glfwGetKey(handle, GLFW_KEY_W) == GLFW_PRESS)
		offset.y += 1.0f;
	if (glfwGetKey(handle, GLFW_KEY_S) == GLFW_PRESS)
		offset.y -= 1.0f;
	if (glfwGetKey(handle, GLFW_KEY_D) == GLFW_PRESS)
		offset.x += 1.0f;
	if (glfwGetKey(handle, GLFW_KEY_A) == GLFW_PRESS)
		offset.x -= 1.0f;
	if (glm::length2(offset) > 0.5f) {
		const auto speed = 0.5f;
		offset = dt * speed * glm::normalize(offset);
		const auto tfm = ng.tfms.referential(player);
		tfm->pos() += offset;
		ng.render.camera.pos -= offset;
	}
	if (glfwGetKey(handle, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(handle, true);
	if (glfwGetKey(handle, GLFW_KEY_F) == GLFW_PRESS && !ng.tick.live(attack))
		attack = spawn_slash(player);
	return attack;
}

phobos::entity spawn_enemy(phobos::entity player, glm::vec2 pos)
{
	const auto enemy = phobos::spawn();
	ng.tfms.transformable(enemy, quad_transform(pos, {0.5f,0.5f}));
	ng.render.drawable(enemy, phobos::render::object::enemy);
	ng.phys.collider_circle(enemy, 0);
	ng.enemy.make_enemy(enemy, phobos::enemy::type_t::dumb0);
	// ng.hp.damageable(enemy, 3.0f);
	// const auto hp_bar = phobos::spawn();
	// ng.render.drawable(hp_bar, phobos::render::object::hp_bar);
	// ng.tfms.transformable(hp_bar, quad_transform({0.0f,1.0f}, {1.0f,0.2f}, enemy));
	return enemy;
}

int main()
{
	using namespace std::literals;
	// TODO: use a NUL-terminated string type
	window win{"Gaming\0"sv};
	if (win.error()) return 1;
	if (phobos::init() != phobos::system_id::none) return 1;
	const auto player = phobos::spawn();
	ng.render.drawable(player, phobos::render::object::player);
	ng.tfms.transformable(player, quad_transform({-0.3f,-0.1f}, {1.0f,1.0f}));
	ng.phys.collider_circle(player, 0);
	ng.enemy.make_player(player);
	phobos::entity attack = 0;
	const auto e1 = spawn_enemy(player, {+0.6f,+0.2f});
	const auto e2 = spawn_enemy(player, {+0.9f,-0.5f});

	std::print("\n");
	auto prev_time = glfwGetTime();
	while (win.live()) {
		const auto now = glfwGetTime();
		const auto dt = now - prev_time;
		prev_time = now;
		attack = player_control(win, attack, player, dt);
		std::print("\rframe time: {:#4.1f}ms fps: {:#3.1f}s-1         ", dt * 1e3, 1.0f / dt);
		phobos::update(now, dt);
		win.draw();
	}
	phobos::clear();
	phobos::fini();
	std::print("\n");
}


