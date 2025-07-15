#pragma once
#include "entity.hpp"
#include <unordered_map>

namespace phobos {

class tick
{
public:
	struct expire_in_t { float remaining; };
	struct follow_t { entity target; };
private:
	std::unordered_map<entity, expire_in_t> expiring_;
	std::unordered_map<entity, follow_t> following_;
public:
	int init();
	void fini();
	void update(float now, float dt);
	void clear();

	void expire_in(entity e, expire_in_t data);
	bool live(entity e);
	void follow(entity e, follow_t data);
};

} // phobos

