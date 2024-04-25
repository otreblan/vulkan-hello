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

#include <assimp/matrix4x4.h>
#include <boost/container/flat_set.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/align/aligned_allocator.hpp>
#include <entt/entt.hpp>
#include <glm/mat4x4.hpp>

static const size_t cache_line = 64;

template <typename T, std::size_t N>
using small_aligned_vector = boost::container::small_vector<T, N, boost::alignment::aligned_allocator<T, cache_line>>;

template <typename T, std::size_t N>
using small_flat_set = boost::container::flat_set<T, std::less<T>, small_aligned_vector<T, N>>;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnonnull"
template <typename... T>
using group_t = decltype(((entt::registry*)nullptr)->group<T...>());
#pragma GCC diagnostic pop

glm::mat4x4 toGlm(const aiMatrix4x4& m);
glm::vec3 toGlm(const aiVector3D& v);
