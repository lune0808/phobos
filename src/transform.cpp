#include "system.hpp"
#include <cassert>

namespace phobos {

glm::vec2 &transform::pos() { return (*this)[2]; }
glm::vec2 &transform::  x() { return (*this)[0]; }
glm::vec2 &transform::  y() { return (*this)[1]; }

int tfms::init()
{
	return 0;
}

void tfms::fini()
{
}

void tfms::update(float, float)
{
	for (const auto e : on_hold()) {
		data.erase(e);
	}
}

void tfms::clear()
{
	data.clear();
}

void tfms::transformable(entity e, transform tfm)
{
	auto [_, inserted] = data.emplace(e, tfm);
	assert(inserted);
}

transform *tfms::referential(entity e)
{
	auto at = data.find(e);
	return at != data.end()? &at->second: nullptr;
}

transform tfms::world(entity e)
{
	auto at = data.find(e);
	if (at->second.parent) {
		return world(at->second.parent) * at->second;
	} else {
		assert(at != data.end());
		return at->second;
	}
}

transform operator*(transform l, transform r)
{
	const glm::mat3x3 lm{
		glm::vec3{ l.  x(), 0.0f },
		glm::vec3{ l.  y(), 0.0f },
		glm::vec3{ l.pos(), 1.0f },
	};
	const glm::mat3x3 rm{
		glm::vec3{ r.  x(), 0.0f },
		glm::vec3{ r.  y(), 0.0f },
		glm::vec3{ r.pos(), 1.0f },
	};
	const auto m = lm * rm;
	return {{
		glm::vec2{m[0][0], m[0][1]},
		glm::vec2{m[1][0], m[1][1]},
		glm::vec2{m[2][0], m[2][1]},
	}};
}

} // phobos

