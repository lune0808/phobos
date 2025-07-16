#include "c++lib.hpp"
#include "system.hpp"

static phobos::key glfw2phoboskey(int key)
{
	switch (key) {
	case GLFW_KEY_ESCAPE: return phobos::key::K_ESCAPE;
	case GLFW_KEY_A: return phobos::key::K_A;
	case GLFW_KEY_B: return phobos::key::K_B;
	case GLFW_KEY_C: return phobos::key::K_C;
	case GLFW_KEY_D: return phobos::key::K_D;
	case GLFW_KEY_E: return phobos::key::K_E;
	case GLFW_KEY_F: return phobos::key::K_F;
	case GLFW_KEY_G: return phobos::key::K_G;
	case GLFW_KEY_H: return phobos::key::K_H;
	case GLFW_KEY_I: return phobos::key::K_I;
	case GLFW_KEY_J: return phobos::key::K_J;
	case GLFW_KEY_K: return phobos::key::K_K;
	case GLFW_KEY_L: return phobos::key::K_L;
	case GLFW_KEY_M: return phobos::key::K_M;
	case GLFW_KEY_N: return phobos::key::K_N;
	case GLFW_KEY_O: return phobos::key::K_O;
	case GLFW_KEY_P: return phobos::key::K_P;
	case GLFW_KEY_Q: return phobos::key::K_Q;
	case GLFW_KEY_R: return phobos::key::K_R;
	case GLFW_KEY_S: return phobos::key::K_S;
	case GLFW_KEY_T: return phobos::key::K_T;
	case GLFW_KEY_U: return phobos::key::K_U;
	case GLFW_KEY_V: return phobos::key::K_V;
	case GLFW_KEY_W: return phobos::key::K_W;
	case GLFW_KEY_X: return phobos::key::K_X;
	case GLFW_KEY_Y: return phobos::key::K_Y;
	case GLFW_KEY_Z: return phobos::key::K_Z;
	case GLFW_KEY_LEFT_BRACKET: return phobos::key::K_LEFT_SQUARE_BRACKET;
	case GLFW_KEY_RIGHT_BRACKET: return phobos::key::K_RIGHT_SQUARE_BRACKET;
	default: return phobos::key::K_NONE;
	}
}

namespace phobos {

int input::init()
{
	int status = 1;
	using namespace std::literals;
	win.init("Phobos\0"sv);
	const auto key_callback = [] (GLFWwindow *handle, int key, int scan, int act, int mod)
	{
		input &ref = *static_cast<input*>(glfwGetWindowUserPointer(handle));
		(void) scan;
		(void) mod;
		const auto idx = static_cast<size_t>(glfw2phoboskey(key));
		if (idx == 0)
			// unhandled
			return;
		if (act == GLFW_PRESS) {
			ref.held_state.set(idx, true);
			ref.pressed_state.set(idx, true);
		} else if (act == GLFW_RELEASE) {
			ref.held_state.set(idx, false);
		}
	};
	if (win.error()) goto end;
	glfwSetWindowUserPointer(win.get_handle(), this);
	glfwSetKeyCallback(win.get_handle(), key_callback);
	status = 0;
end:
	return status;
}

void input::fini()
{
}

int gl::init()
{
	return 0;
}

// HACK: can't reverse macros so `input` init's the GL
// and `gl` fini's the GL
void gl::fini()
{
	system.input.win.fini();
}

void gl::update(float, float)
{
}

void input::update(float, float)
{
	pressed_state.reset();
}

bool input::held(key k) const
{
	return held_state.test(static_cast<size_t>(k));
}

bool input::pressed(key k) const
{
	return pressed_state.test(static_cast<size_t>(k));
}

void input::remove(std::uint32_t)
{
}

void gl::remove(entity)
{
}

} // phobos

