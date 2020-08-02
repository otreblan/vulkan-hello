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

cmake_minimum_required(VERSION 3.13.0)

project(hello
	VERSION 4.0
	DESCRIPTION "A C++ template"
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
		CXX_STANDARD 17
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
)

## Header only libraries
#pkg_check_modules(header_only
#	REQUIRED IMPORTED_TARGET
#		libfoo3
#		libfoo4
#)

# Linking
target_link_libraries(${PROJECT_NAME}
	PRIVATE
		PkgConfig::shared

#	INTERFACE
#		PkgConfig::header_only
)

# Default flags
if(UNIX AND NOT DEFINED ENV{CXXFLAGS})
	set(CMAKE_CXX_FLAGS "-Wall -Wextra -g")
endif()

# Install target
install(TARGETS ${PROJECT_NAME}
	DESTINATION ${CMAKE_INSTALL_BINDIR}
)
