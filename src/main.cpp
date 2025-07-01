#include "window.hpp"
#include "render.hpp"
#include "phys.hpp"
#include <glm/glm.hpp>
#include <string_view>


render::per_entity quad_transform(glm::vec2 origin, glm::vec2 dims)
{
	return {{
		{ dims.x  , 0.0f    , 0.0f },
		{ 0.0f    , dims.y  , 0.0f },
		{ origin.x, origin.y, 1.0f },
	}};
}

render::per_entity tri_transform(glm::vec2 origin, glm::vec2 u, glm::vec2 v)
{
	return {{
		{ u.x     , u.y     , 0.0f },
		{ v.x     , v.y     , 0.0f },
		{ origin.x, origin.y, 1.0f },
	}};
}

int main()
{
	using namespace std::literals;
	// TODO: use a NUL-terminated string type
	window win{"Gaming\0"sv};
	if (win.error()) return 1;
	render rdr{};
	const auto player = rdr.spawn(render::object::player, quad_transform({-0.3f,-0.1f}, {1.0f,1.0f}));
	const auto enemy  = rdr.spawn(render::object::enemy , quad_transform({ 0.6f, 0.2f}, {0.5f,0.5f}));
	const auto cone = rdr.spawn(render::object::attack_cone, tri_transform({ 0.3f,-0.1f}, {0.4f,0.1f}, {-0.1f,0.4f}));

	while (win.live()) {
		const auto cone_gfx_state = rdr.access(cone);
		const auto enemy_gfx_state = rdr.access(enemy);
		circle enemy_hitbox{ { enemy_gfx_state->transform[2][0], enemy_gfx_state->transform[2][1] },
			enemy_gfx_state->transform[0][0]/2.0f };
		const auto t0 = glfwGetTime();
		const auto t1 = t0 + 0.3f;
		const glm::vec2 u{ 0.4f * std::cos(t0), 0.4f * std::sin(t0) };
		const glm::vec2 v{ 0.4f * std::cos(t1), 0.4f * std::sin(t1) };
		*cone_gfx_state = tri_transform({ cone_gfx_state->transform[2][0], cone_gfx_state->transform[2][1] }, u, v);
		triangle cone_hitbox{ { cone_gfx_state->transform[2][0], cone_gfx_state->transform[2][1] }, u, v };
		const auto colliding = collision_test(enemy_hitbox, cone_hitbox);
		cone_gfx_state->flags().colliding = colliding;
		rdr.draw(win);
		win.draw();
	}
	rdr.despawn(cone);
	rdr.despawn(enemy );
	rdr.despawn(player);
}


