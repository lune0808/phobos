#include "window.hpp"
#include "render.hpp"
#include "phys.hpp"
#include "tick.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <string_view>


render::per_entity quad_transform(glm::vec2 origin, glm::vec2 dims)
{
	return {{{
		{ dims.x, 0.0f   },
		{ 0.0f  , dims.y },
		origin
	}}};
}

render::per_entity tri_transform(glm::vec2 origin, glm::vec2 u, glm::vec2 v)
{
	return {{{ u, v, origin }}};
}

render::entity spawn_slash(render &rdr, tick &tick, phys &phys, render::entity player, render::entity target)
{
	const auto pos = rdr.access(player)->pos() + glm::vec2{0.55f,0.0f};
	const auto cone = rdr.spawn(render::object::attack_cone, quad_transform(pos, {0.0f,0.0f}));
	const auto trail = rdr.spawn(render::object::trail, render::per_entity{});
	const float lifetime = 0.2f;
	tick.expire_in(cone, {lifetime});
	tick.expire_in(trail, {lifetime});
	tick.spin(cone, {{0.2f,-0.4f}});
	rdr.add_trail(trail, cone);
	phys.collider_triangle(cone, phys::mask_v<circle>);
	return cone;
}

render::entity player_control(window const &win, render &rdr, tick &tick, phys &phys,
		render::entity attack, render::entity target, render::entity player, float dt)
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
		const auto tfm = rdr.access(player);
		tfm->pos() += offset;
		rdr.camera.pos -= offset;
	}
	if (glfwGetKey(handle, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(handle, true);
	if (glfwGetKey(handle, GLFW_KEY_F) == GLFW_PRESS && !rdr.access(attack))
		attack = spawn_slash(rdr, tick, phys, player, target);
	return attack;
}

int main()
{
	using namespace std::literals;
	// TODO: use a NUL-terminated string type
	window win{"Gaming\0"sv};
	if (win.error()) return 1;
	render rdr{{0.0f,0.0f}, win.dims()};
	tick tick{rdr};
	phys phys;
	const auto player = rdr.spawn(render::object::player, quad_transform({-0.3f,-0.1f}, {1.0f,1.0f}));
	const auto enemy  = rdr.spawn(render::object::enemy , quad_transform({ 0.6f, 0.2f}, {0.5f,0.5f}));
	phys.collider_circle(player, 0);
	phys.collider_circle(enemy, 0);
	tick.follow(enemy, {player});
	render::entity attack = 0;

	auto prev_time = glfwGetTime();
	while (win.live()) {
		const auto now = glfwGetTime();
		const auto dt = now - prev_time;
		prev_time = now;
		attack = player_control(win, rdr, tick, phys, attack, enemy, player, dt);
		tick.update(dt);
		phys.sim(rdr, dt);
		rdr.draw();
		win.draw();
	}
	rdr.despawn(enemy );
	rdr.despawn(player);
}


