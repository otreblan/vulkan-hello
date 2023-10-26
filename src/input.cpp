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

#include <iostream>

#include <GLFW/glfw3.h>

#include "input.hpp"
#include "engine.hpp"

Input::Input(Engine& engine):
	engine(engine),
	Axis(0, 0)
{
}

glm::vec2 Input::getAxis() const
{
	return Axis;
}

void Input::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	switch(key)
	{
		case GLFW_KEY_A:
			if(action == GLFW_PRESS)
				aPressed = true;
			else if(action == GLFW_RELEASE)
				aPressed = false;
			break;

		case GLFW_KEY_D:
			if(action == GLFW_PRESS)
				dPressed = true;
			else if(action == GLFW_RELEASE)
				dPressed = false;
			break;
	}

	if(aPressed && !dPressed)
		Axis.x = -1;
	else if(!aPressed && dPressed)
		Axis.x = 1;
	else
		Axis.x = 0;

	//TODO
}
