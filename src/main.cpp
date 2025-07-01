#include "window.hpp"
#include "render.hpp"
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
	auto player = rdr.spawn(render::object::player, quad_transform({-0.3f,-0.1f}, {1.0f,1.0f}));
	auto enemy  = rdr.spawn(render::object::enemy , quad_transform({ 0.6f, 0.2f}, {0.5f,0.5f}));
	auto cone = rdr.spawn(render::object::attack_cone, tri_transform({ 0.3f,-0.1f}, {0.4f,0.1f}, {-0.1f,0.4f}));

	size_t frame = 0;
	while (win.live()) {
		++frame;
		if (frame % 144 == 0) {
			auto cone_gfx_state = rdr.access(cone);
			cone_gfx_state->flags().colliding ^= 1;
		}
		rdr.draw(win);
		win.draw();
	}
	rdr.despawn(cone);
	rdr.despawn(enemy );
	rdr.despawn(player);
}


