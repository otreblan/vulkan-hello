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
	LANGUAGES "CXX"
)

# Without this, paths are not relative in the sources list
cmake_policy(SET CMP0076 NEW)

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

# Headers
target_include_directories(${PROJECT_NAME}
	PRIVATE
		${PROJECT_SOURCE_DIR}/include
)

add_subdirectory(src) # Sources list
add_subdirectory(pkg) # Packaging

# Shared libraries
pkg_check_modules(shared
	REQUIRED IMPORTED_TARGET
		vulkan
		glfw3
)

find_package(XdgUtils REQUIRED
	COMPONENTS
		DesktopEntry
		BaseDir
)

# Header only libraries
pkg_check_modules(header_only
	REQUIRED IMPORTED_TARGET
		glm
)

# Linking
target_link_libraries(${PROJECT_NAME}
	PRIVATE
		PkgConfig::shared
		XdgUtils::DesktopEntry
		XdgUtils::BaseDir

	INTERFACE
		PkgConfig::header_only
)

# Defining some macros
target_compile_definitions(${PROJECT_NAME}
	PRIVATE
		GLFW_INCLUDE_VULKAN
		GLM_FORCE_RADIANS
		GLM_FORCE_DEPTH_ZERO_TO_ONE
)

# Default flags and macros
if(UNIX AND NOT DEFINED ENV{CXXFLAGS})
	set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-missing-field-initializers -g")
	target_compile_definitions(${PROJECT_NAME}
		PRIVATE
			DEBUG
	)
endif()

# Install target
install(TARGETS ${PROJECT_NAME}
	DESTINATION ${CMAKE_INSTALL_BINDIR}
)
