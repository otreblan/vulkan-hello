// Vulkan
// Copyright © 2020 otreblan
//
// vulkan-hello is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// vulkan-hello is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with vulkan-hello.  If not, see <http://www.gnu.org/licenses/>.

#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "exePath.hpp"
#include "vulkan/renderer.hpp"

int main(int argc, char** argv)
{
	if(argc < 2)
	{
		std::cerr << "Usage: " << argv[0] << " SCENE\n";
		return EXIT_FAILURE;
	}

	std::cerr << exePath() << '\n';

	Renderer app(argv[1]);

	app.run();

	exit(EXIT_SUCCESS);
}
