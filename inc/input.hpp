#pragma once
#include "window.hpp"
#include <bitset>

namespace phobos {

enum class key {
	K_NONE = 0,
	K_ESCAPE,
	K_A = 'A',
	K_B = 'B',
	K_C = 'C',
	K_D = 'D',
	K_E = 'E',
	K_F = 'F',
	K_G = 'G',
	K_H = 'H',
	K_I = 'I',
	K_J = 'J',
	K_K = 'K',
	K_L = 'L',
	K_M = 'M',
	K_N = 'N',
	K_O = 'O',
	K_P = 'P',
	K_Q = 'Q',
	K_R = 'R',
	K_S = 'S',
	K_T = 'T',
	K_U = 'U',
	K_V = 'V',
	K_W = 'W',
	K_X = 'X',
	K_Y = 'Y',
	K_Z = 'Z',
	K_LEFT_SQUARE_BRACKET = '[',
	K_RIGHT_SQUARE_BRACKET = ']',
	NUM
};

struct input {
	bool held(key k) const;
	bool pressed(key k) const;

	int init();
	void fini();
	void update(float now, float dt);
	void clear();

	window win;
private:
	std::bitset<static_cast<size_t>(key::NUM)> held_state, pressed_state;
};

struct gl {
	int init();
	void fini();
	void update(float, float);
	void clear();
};

} // phobos

