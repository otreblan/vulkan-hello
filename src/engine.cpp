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
#include "system/game.hpp"

Engine::Engine(const std::filesystem::path& mainScene):
	mainScene(mainScene),
	window(*this),
	input(*this)
{}

Engine::~Engine()
{}

int Engine::run()
{
	using namespace std::chrono;
	using namespace ecs_system;

	auto currentTime = high_resolution_clock::now();
	float delta      = 1.f/60;

	scheduler.attach([](auto...){glfwPollEvents();});
	scheduler.attach<Game>(*this);
	scheduler.attach<Renderer>(*this);

	while(!scheduler.empty() && !shouldStop)
	{
		auto lastTime = currentTime;

		scheduler.update(delta);

		currentTime = high_resolution_clock::now();
		delta       = duration<float, seconds::period>(currentTime - lastTime).count();
	}

	return EXIT_SUCCESS;
}

Scene& Engine::getActiveScene()
{
	return mainScene;
}

GLFWwindow* Engine::getWindow()
{
	return window.getWindow();
}

Input& Engine::getInput()
{
	return input;
}

void Engine::setRenderer(Renderer* renderer)
{
	activeRenderer = renderer;
}

void Engine::stop()
{
	shouldStop = true;
}

void Engine::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto engine = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));

	if(engine->activeRenderer)
		engine->activeRenderer->framebufferResizeCallback(window, width, height);
}

void Engine::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	auto engine = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));

	engine->input.keyCallback(window, key, scancode, action, mods);
}
