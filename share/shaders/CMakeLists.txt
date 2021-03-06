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

function(add_spirv_target)
	set(options)
	set(oneValueArgs TARGET DESTINATION)
	set(multiValueArgs SOURCES)

	cmake_parse_arguments(ADD_SPIRV_TARGET
		"${options}"
		"${oneValueArgs}"
		"${multiValueArgs}"
		${ARGN}
	)

	foreach(SOURCE IN LISTS ADD_SPIRV_TARGET_SOURCES)
		get_filename_component(SRC_EXT ${SOURCE} LAST_EXT)
		string(SUBSTRING ${SRC_EXT} 1 -1 SRC_EXT)
		set(SPIRV ${SRC_EXT}.spv)

		add_custom_command(OUTPUT "${SPIRV}"
			DEPENDS "${SOURCE}"
			WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
			COMMAND glslc "${SOURCE}" -o "${CMAKE_CURRENT_BINARY_DIR}/${SPIRV}"
			COMMENT "${SOURCE} -> ${SPIRV}"
		)

		list(APPEND SHADER_BINARIES "${SPIRV}")

		install(FILES
			"${CMAKE_CURRENT_BINARY_DIR}/${SPIRV}"
			DESTINATION "${ADD_SPIRV_TARGET_DESTINATION}"
		)
	endforeach()

	add_custom_target("${ADD_SPIRV_TARGET_TARGET}" ALL
		DEPENDS
			"${SHADER_BINARIES}"
	)

endfunction()

add_spirv_target(TARGET shaders
	DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/shaders/"
	SOURCES
		shader.frag
		shader.vert
)
