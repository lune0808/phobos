#include "window.hpp"


int main()
{
	using namespace std::literals;
	window win{"Gaming\0"sv};
	if (!win.error()) while (win.live()) {
		win.draw();
	}
}


