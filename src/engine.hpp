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

#pragma once

#include <filesystem>

#include "input.hpp"
#include "scene.hpp"
#include "settings.hpp"
#include "vulkan/renderer.hpp"
#include "window.hpp"

class Engine
{
public:
	Engine(const std::filesystem::path& mainScene);
	~Engine();

	/// Starts the engine and returns an exit code.
	int run();

	Scene&      getActiveScene();
	GLFWwindow* getWindow();
	Input&      getInput();
	Settings&   getSettings();
	void        setRenderer(Renderer* renderer);

	/// Stops the engine and exits
	void stop();

	template<typename... Type>
	[[nodiscard]] decltype(auto) get(const entt::entity entt)
	{
		return getActiveScene().registry.get<Type...>(entt);
	}

private:
	Scene    mainScene;
	Window   window;
	Input    input;
	Settings settings;

	/// Non owning reference
	Renderer* activeRenderer;

	entt::basic_scheduler<float> scheduler;

	bool shouldStop = false;

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

	friend class Window;
};
