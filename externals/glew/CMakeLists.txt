find_package(OpenGL REQUIRED)

# Compile glew
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(glew_2_0_0
		STATIC
		src/glew.c
)

target_link_libraries(glew_2_0_0
	${OPENGL_LIBRARIES}
)

# Variables for linking
set(GLEW_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include PARENT_SCOPE)
set(GLEW_LIBRARIES glew_2_0_0 PARENT_SCOPE)
