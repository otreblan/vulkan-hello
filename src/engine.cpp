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

#include <chrono>

#include "engine.hpp"

Engine::Engine(const std::filesystem::path& mainScene):
	mainScene(mainScene)
{};

int Engine::run()
{
	using namespace std::chrono;

	auto currentTime = high_resolution_clock::now();
	float delta      = 1.f/60;

	entt::basic_scheduler<float> scheduler;

	scheduler.attach<Renderer>(&mainScene);

	while(!scheduler.empty())
	{
		auto lastTime = currentTime;

		scheduler.update(delta);

		currentTime = high_resolution_clock::now();
		delta       = duration<float, seconds::period>(currentTime - lastTime).count();
	}

	return EXIT_SUCCESS;
}
