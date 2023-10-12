# Vulkan
# Copyright © 2020 otreblan
#
# vulkan-hello is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# vulkan-hello is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with vulkan-hello.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.13.0)

project(vulkan-hello
	VERSION 0.0.0
	DESCRIPTION "Vulkan"
	HOMEPAGE_URL "https://github.com/otreblan/hello"
)

# Default build type
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug")
endif()

include(GNUInstallDirs)
include(FindPkgConfig)
include(CPack)

# The program itself
add_executable(${PROJECT_NAME})

# C++ version
set_target_properties(${PROJECT_NAME}
	PROPERTIES
		CXX_STANDARD 20
)

add_subdirectory(pkg)
add_subdirectory(share)
add_subdirectory(src) # Sources list

# Shared libraries
pkg_check_modules(libraries
	REQUIRED IMPORTED_TARGET
		assimp
		glfw3
		glm
		vulkan
)

find_package(XdgUtils REQUIRED
	COMPONENTS
		DesktopEntry
		BaseDir
)

# Linking
target_link_libraries(${PROJECT_NAME}
	PRIVATE
		PkgConfig::libraries
		XdgUtils::DesktopEntry
		XdgUtils::BaseDir
)

# Defining some macros
target_compile_definitions(${PROJECT_NAME}
	PRIVATE
		$<$<CONFIG:DEBUG>:VK_DEBUG>
		GLFW_INCLUDE_VULKAN
		GLM_FORCE_DEPTH_ZERO_TO_ONE
		GLM_FORCE_RADIANS
)

target_compile_options(${PROJECT_NAME}
	PRIVATE
		$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wno-missing-field-initializers>
)

# Install target
install(TARGETS ${PROJECT_NAME}
	DESTINATION ${CMAKE_INSTALL_BINDIR}
)
