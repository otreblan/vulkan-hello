# A C++ template
# Copyright © 2019-2020 otreblan
#
# hello is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# hello is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with hello.  If not, see <http://www.gnu.org/licenses/>.

configure_file(config.cpp.in
	${CMAKE_CURRENT_SOURCE_DIR}/config.cpp
	@ONLY
)

target_sources(${PROJECT_NAME}
	PRIVATE
		config.cpp
		exePath.cpp
		helloTriangle.cpp
		main.cpp
		stb_image.cpp
		swapChainSupportDetails.cpp
		vertex.cpp
)
