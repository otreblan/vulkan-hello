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

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../engine.hpp"
#include "../input.hpp"
#include "mawaru.hpp"

namespace ecs::system
{

Mawaru::Mawaru(Engine& engine):
	engine(engine)
{}

void Mawaru::init()
{
}

void Mawaru::update(float delta, void*)
{
	using namespace ecs::component;

	auto& input = engine.inject<Input>();

	auto& transform = engine.get<Transform>(engine.getActiveScene().root);

	float rotation = delta * input.getAxis().x * glm::radians(rotationSpeed);

	transform.matrix = glm::rotate(transform.matrix, rotation, glm::vec3(0, 1, 0));

	if(input.space())
	{
		if(!spacePressed)
		{
			spacePressed = true;
			std::cout << "Click\n";

			engine.getSettings().vsync = !engine.getSettings().vsync;
			engine.getSettings().flush();
		}
	}
	else
	{
		spacePressed = false;
	}
}

}
