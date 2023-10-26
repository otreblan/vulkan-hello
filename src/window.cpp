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

#include "engine.hpp"
#include "window.hpp"

Window::Window(Engine& engine):
	engine(engine)
{
	if(!glfwInit())
		throw;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);

	glfwSetWindowUserPointer(engine.getWindow(), &engine);

	glfwSetFramebufferSizeCallback(engine.getWindow(), Engine::framebufferResizeCallback);
	glfwSetKeyCallback(engine.getWindow(),             Engine::KeyCallback);
}

Window::~Window()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

GLFWwindow* Window::getWindow()
{
	return window;
}
