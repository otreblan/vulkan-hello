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

#include "../engine.hpp"
#include "physics.hpp"

ecs_system::Physics::Physics(Engine& engine):
	engine(engine)
{}

void ecs_system::Physics::init()
{
	std::cout << "Physics started\n";

	collisionConfiguration = std::make_unique<btDefaultCollisionConfiguration>();
	dispatcher             = std::make_unique<btCollisionDispatcher>(collisionConfiguration.get());
	overlappingPairCache   = std::make_unique<btDbvtBroadphase>();
	solver                 = std::make_unique<btSequentialImpulseConstraintSolver>();

	world = std::make_unique<btDiscreteDynamicsWorld>(
		dispatcher.get(),
		overlappingPairCache.get(),
		solver.get(),
		collisionConfiguration.get()
	);

	// TODO: Upload rigidbodies
}

void ecs_system::Physics::update(float delta, void*)
{
	world->stepSimulation(delta, 10);

	for(int i = world->getNumCollisionObjects()-1; i >= 0; i--)
	{
		btCollisionObject* obj  = world->getCollisionObjectArray()[i];
		btRigidBody*       body = btRigidBody::upcast(obj);

		btTransform transform;
		if(body && body->getMotionState())
		{
			body->getMotionState()->getWorldTransform(transform);
		}
		else
		{
			transform = obj->getWorldTransform();
		}

		// TODO: Update engine transforms
	}
}
