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

transform *tfms::get(entity e)
{
	auto at = data.find(e);
	return at != data.end()? &at->second: nullptr;
}

} // phobos

