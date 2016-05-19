# Compile fmt
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/fmt)
add_library(fmt_3_0_0
	STATIC
	src/format.cc
)

# Variables for linking
set(FMT_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include PARENT_SCOPE)
set(FMT_LIBRARIES fmt_3_0_0 PARENT_SCOPE)
