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

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "../engine.hpp"
#include "physics.hpp"
#include "../component/transform.hpp"
#include "../component/collider.hpp"

namespace ecs::system
{

Physics::Physics(Engine& engine):
	engine(engine)
{}

Physics::~Physics()
{
	//remove the rigidbodies from the dynamics world and delete them
	for (int i = world->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj  = world->getCollisionObjectArray()[i];
		btRigidBody*       body = btRigidBody::upcast(obj);

		if(body && body->getMotionState())
			delete body->getMotionState();

		world->removeCollisionObject(obj);
		delete obj;
	}

	for(int i = 0; i < collisionShapes.size(); i++)
	{
		delete collisionShapes[i];
	}
}

void Physics::init()
{
	using namespace ecs::component;

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

	world->setGravity(btVector3(0, -9.8, 0));

	const auto cGroup = engine.getActiveScene().registry.group<Collider>(entt::get<Transform>);

	collisionShapes.reserve(cGroup.size());

	for(entt::entity entity: cGroup)
	{
		auto [collider, transform] = cGroup.get<Collider, Transform>(entity);

		glm::vec3 box   = (collider.max - collider.min) / 2.f;
		auto*     shape = new btBoxShape(btVector3(box.x, box.y, box.z));

		shape->setUserPointer(&transform);

		collisionShapes.push_back(shape);

		btTransform _transform;

		_transform.setFromOpenGLMatrix(glm::value_ptr(transform.matrix));

		btScalar mass = collider.mass;

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0);

		btVector3 localInertia(0, 0, 0);

		if (isDynamic)
			shape->calculateLocalInertia(mass, localInertia);

		//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
		auto* myMotionState = new btDefaultMotionState(_transform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, shape, localInertia);

		auto* body = new btRigidBody(rbInfo);

		body->setUserPointer(&transform);

		//add the body to the dynamics world
		world->addRigidBody(body);
	}
}

void Physics::update(float delta, void*)
{
	using namespace ecs::component;

	world->stepSimulation(delta, 10);

	// TODO: Update physics transform from world transform

	for(int i = world->getNumCollisionObjects()-1; i >= 0; i--)
	{
		btCollisionObject* obj  = world->getCollisionObjectArray()[i];
		btRigidBody*       body = btRigidBody::upcast(obj);

		btTransform _transform;
		if(body && body->getMotionState())
		{
			body->getMotionState()->getWorldTransform(_transform);
		}
		else
		{
			_transform = obj->getWorldTransform();
		}

		Transform* transform = (Transform*)obj->getUserPointer();

		_transform.getOpenGLMatrix(glm::value_ptr(transform->matrix));
	}
}

}
