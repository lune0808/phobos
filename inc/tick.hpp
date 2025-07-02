#pragma once
#include "render.hpp" // entity
#include <unordered_map>

class tick
{
public:
	struct expire_in_t { float remaining; };
	struct follow_t { render::entity target; };
	struct collide_test_t { render::entity target; };
private:
	std::unordered_map<render::entity, expire_in_t> expiring_;
	std::unordered_map<render::entity, follow_t> following_;
	std::unordered_map<render::entity, collide_test_t> colliding_;
	render &rdr;

public:
	tick(render &rdr): rdr(rdr) {}

	void expire_in(render::entity e, expire_in_t data);
	void follow(render::entity e, follow_t data);
	void collide_test(render::entity e, collide_test_t data);
	void update(float dt);
};

