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

#include <entt/entt.hpp>

class Injector
{
private:
	entt::registry registry;

public:

	template<typename Type, typename... Args>
	Type& emplace_injectable(Args &&...args) {
		return registry.ctx().emplace<Type>(std::forward<Args>(args)...);
	}

	template<typename Type>
	[[nodiscard]] Type& inject() {
		return registry.ctx().get<Type>();
	}

	template<typename Type>
	[[nodiscard]] const Type& inject() const {
		return registry.ctx().get<Type>();
	}

};
