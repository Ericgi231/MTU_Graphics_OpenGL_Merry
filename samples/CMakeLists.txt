if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
	message(FATAL_ERROR "Don't run cmake here. Run it in the root folder of this repository.")
endif()
cmake_minimum_required(VERSION 2.8.12)


####################################
# Edit the following areas to add or remove programs to compile
#
# If you add a new name here, there must be an .c or .cpp file with the same
# name that contains a main() function.
####################################
set(PROGRAMS_TO_MAKE merry triangle triangle-shade triangle-color texture texturefilter glinfo teartest picker prerend panorama pong text ogl2-slideshow ogl2-triangle ogl2-texture tracker-stats videoplay zfight viewer slerp explode flock frustum ik tracker-demo distjudge)


# Make a target that lets us copy all of the vert and frag files from this directory into the bin directory.
# file(GLOB ShaderFiles "*.vert" "*.frag")
# add_custom_target(copyGLSL)
# foreach(ShaderFile ${ShaderFiles})
#     add_custom_command(TARGET copyGLSL PRE_BUILD
#         COMMAND ${CMAKE_COMMAND} -E
#         copy ${ShaderFile} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
# endforeach()


# Compile the list of programs.
foreach(arg ${PROGRAMS_TO_MAKE})
	if(EXISTS ${CMAKE_SOURCE_DIR}/samples/${arg}.c)   # Figure out if the program is a c or cpp file
		set(SOURCE_FILE ${arg}.c)
	else()
		set(SOURCE_FILE ${arg}.cpp)
	endif()
	add_executable(${arg} ${SOURCE_FILE})

	target_link_libraries(${arg} kuhl)
	target_link_libraries(${arg} ${ASSIMP_LIBRARIES})

	# === Optional libraries: ===
	if(VRPN_FOUND)  # Add VRPN to the list if it is available
		target_link_libraries(${arg} ${VRPN_LIBRARIES})
	endif()
	if(OVR_FOUND) # Add Oculus LibOVR to the list if it is available
		target_link_libraries(${arg} ${OVR_LIBRARIES} ${CMAKE_DL_LIBS})
	endif()
	if(ImageMagick_FOUND)
		target_link_libraries(${arg} ${ImageMagick_LIBRARIES})
	endif()
	if(FREETYPE_FOUND)
		target_link_libraries(${arg} ${FREETYPE_LIBRARIES})
	endif()
	if(FFMPEG_FOUND)
		target_link_libraries(${arg} ${FFMPEG_LIBRARIES})
	endif()


	target_link_libraries(${arg} ${GLEW_LIBRARIES} ${GLFW_LIBRARIES} ${M_LIB} ${OPENGL_LIBRARIES} )
	if(APPLE)
		# Some Mac OSX machines need this to ensure that freetype.h is found.
		target_include_directories(${arg} PUBLIC "/opt/X11/include/freetype2/")
	endif()

	set_target_properties(${arg} PROPERTIES LINKER_LANGUAGE "CXX")
	set_target_properties(${arg} PROPERTIES COMPILE_DEFINITIONS "${PREPROC_DEFINE}")

	# Any time we make this program, copy all of the vertex and fragment program files into bin.
	# add_dependencies(${arg} copyGLSL)
	if(OVR_FOUND AND ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
		add_dependencies(${arg} copyOVR)
	endif()

endforeach()
