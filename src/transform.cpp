#include "c++lib.hpp"
#include "system.hpp"

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
}

void tfms::transformable(entity e, float scale, glm::vec2 offset, entity parent)
{
	transform tfm{
		{
			{ scale, 0.0f },
			{ 0.0f, scale },
			offset,
		},
		parent,
	};
	transformable(e, tfm);
}

void tfms::transformable(entity e, transform tfm)
{
	tfm.id = e;
	data.emplace_back(tfm);
	add_component(e, system_id::tfms);
	reindex(e, system_id::tfms, data.size()-1);
}

void tfms::remove(entity e)
{
	const std::uint32_t idx = index(e, system_id::tfms);
	const std::uint32_t swapped_idx = data.size()-1;
	data[idx] = data[swapped_idx];
	reindex(data[idx].id, system_id::tfms, idx);
	del_component(e, system_id::tfms);
	data.pop_back();
}

transform *tfms::referential(entity e)
{
	auto at = index(e, system_id::tfms);
	return &data[at];
}

transform tfms::world(entity e)
{
	auto *at = referential(e);
	if (at->parent) {
		return world(at->parent) * *at;
	} else {
		return *at;
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

