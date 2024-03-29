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

#include <glm/glm.hpp>

class Engine;
struct GLFWwindow;

class Input
{
private:
	Engine& engine;
	glm::vec2 Axis;

	bool wPressed = false;
	bool aPressed = false;
	bool sPressed = false;
	bool dPressed = false;

	bool spacePressed = false;

	void setPressed(bool& pressed, int action);
public:
	Input(Engine& engine);

	glm::vec2 getAxis() const;

	/// Returns true if space is pressed, false otherwise.
	bool space() const;

	void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};
