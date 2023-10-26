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

#include "utils.hpp"

glm::mat4x4 toGlm(const aiMatrix4x4& m)
{
	glm::mat4x4 _m;

	_m[0][0] = m.a1; _m[1][0] = m.a2; _m[2][0] = m.a3; _m[3][0] = m.a4;
	_m[0][1] = m.b1; _m[1][1] = m.b2; _m[2][1] = m.b3; _m[3][1] = m.b4;
	_m[0][2] = m.c1; _m[1][2] = m.c2; _m[2][2] = m.c3; _m[3][2] = m.c4;
	_m[0][3] = m.d1; _m[1][3] = m.d2; _m[2][3] = m.d3; _m[3][3] = m.d4;

	return _m;
}

glm::vec3 toGlm(const aiVector3D& v)
{
	return {v.x, v.y, v.z};
}
