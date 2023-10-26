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
#include "mawaru.hpp"

ecs_system::Mawaru::Mawaru(Engine& engine):
	engine(engine)
{}

void ecs_system::Mawaru::init()
{
}

void ecs_system::Mawaru::update(float delta, void*)
{
	using namespace glm;

	Scene& scene    = engine.getActiveScene();
	auto& transform = scene.registry.get<component::Transform>(scene.root);

	float rotation = delta * engine.getInput().getAxis().x * radians(rotationSpeed);

	transform.matrix = rotate(transform.matrix, rotation, vec3(0.0f, 1.0f, 0.0f));
}
