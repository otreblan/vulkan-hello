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

#include <taskflow/taskflow.hpp>

#include "engine.hpp"
#include "system/game.hpp"
#include "system/physics.hpp"

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
	using namespace ecs::system;

	auto currentTime = high_resolution_clock::now();
	auto lastTime = currentTime;

	float delta      = 1.f/60;

	tf::Executor executor;
	tf::Taskflow gameloop_taskflow;

	Game     game(*this);
	Physics  physics(*this);
	Renderer renderer(*this);

	// Tasks
	tf::Task start = gameloop_taskflow.placeholder();

	// TODO: Pass subflow in a better way
	tf::Task game_task     = gameloop_taskflow.emplace([&](){game.update(delta, nullptr);});
	tf::Task physics_task  = gameloop_taskflow.emplace([&](tf::Subflow& sbf){physics.update(delta, &sbf);});
	tf::Task renderer_task = gameloop_taskflow.emplace([&](){renderer.update(delta, nullptr);});

	tf::Task end = gameloop_taskflow.placeholder();

	// Names
	gameloop_taskflow.name("Game loop");
	start.name("Start");

	game_task.name("Game");
	physics_task.name("Physics");
	renderer_task.name("Renderer");

	end.name("End");

	// Dependencies
	start.precede(end);

	start.precede(game_task);
	game_task.precede(physics_task);
	physics_task.precede(renderer_task);

	renderer_task.precede(end);

	//gameloop_taskflow.dump(std::cout);
	//exit(EXIT_FAILURE);

	// TODO: Wrap this in a new custom executor
	game.init();
	physics.init();
	renderer.init();

	while(!glfwWindowShouldClose(getWindow()))
	{
		lastTime = currentTime;

		glfwPollEvents();
		executor.run(gameloop_taskflow).wait();

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

Settings& Engine::getSettings()
{
	return settings;
}

void Engine::setRenderer(Renderer* renderer)
{
	activeRenderer = renderer;
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
