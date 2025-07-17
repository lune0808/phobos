#pragma once
#include "c++lib.hpp"
#include "entity.hpp"

namespace phobos {

class tick
{
public:
	struct expire_in_t { float remaining; entity id; };
private:
	std::vector<expire_in_t> expiring_;
public:
	int init();
	void fini();
	void update(float now, float dt);
	void remove(entity e);

	void wait(entity listen, float seconds);

	std::vector<entity> timeout;
};

} // phobos

