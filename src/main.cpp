#include "window.hpp"
#include "render.hpp"
#include <glm/glm.hpp>
#include <string_view>


int main()
{
	using namespace std::literals;
	// TODO: use a NUL-terminated string type
	window win{"Gaming\0"sv};
	if (win.error()) return 1;
	render rdr{};
	auto player = rdr.spawn(render::object::player, {-0.3f,-0.1f, 1.0f });
	auto enemy  = rdr.spawn(render::object::enemy , { 0.6f, 0.2f, 0.5f });
	auto cone = rdr.spawn(render::object::attack_cone, { 0.3f, -0.1f, 0.5f });

	size_t frame = 0;
	while (win.live()) {
		++frame;
		if (frame % 144 == 0) {
			auto cone_gfx_state = rdr.access(cone);
			cone_gfx_state->colliding ^= 1;
		}
		rdr.draw(win);
		win.draw();
	}
	rdr.despawn(cone);
	rdr.despawn(enemy );
	rdr.despawn(player);
}


