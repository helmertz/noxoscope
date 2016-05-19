add_library(stb
	STATIC
	src/stb_image.c
)

target_include_directories(stb PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Variables for linking
set(STB_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include PARENT_SCOPE)
set(STB_LIBRARIES stb PARENT_SCOPE)

