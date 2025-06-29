#include "window.hpp"
#include "shader.hpp"


int main()
{
	using namespace std::literals;
	window win{"Gaming\0"sv};
	if (win.error()) return 1;

	shader_pipeline shader{
		"#version 410 core\n"
		"layout(location=0) in vec2 attr_pos;\n"
		"void main() {\n"
			"gl_Position = vec4(attr_pos, 0.0, 1.0);\n"
		"}\n\0"sv,
		"#version 410 core\n"
		"out vec4 frag_color;\n"
		"void main() {\n"
			"frag_color = vec4(1.0, 1.0, 1.0, 1.0);\n"
		"}\n\0"sv,
	};
	if (!shader.ok()) return 1;

	while (win.live()) {
		win.draw();
	}

	shader.fini();
}


