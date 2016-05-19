cmake_minimum_required(VERSION 2.8.3)

project(Noxoscope)

set(Noxoscope_VERSION_MAJOR 0)
set(Noxoscope_VERSION_MINOR 1)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Set up help variables

if(CMAKE_SIZEOF_VOID_P MATCHES 8)
	set(PROJECT_ARCH "x86_64")
else()
	set(PROJECT_ARCH "x86")
endif()

if(WIN32)
	if(PROJECT_ARCH MATCHES "x86_64")
		set(WIN_LIB_ARCH "win64")
	else()
		set(WIN_LIB_ARCH "win32")
	endif()
endif()

# Add compiler flags

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -std=c++1y -Wno-unused-parameter")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -g")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} -O2")
elseif(MSVC)
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()
	# Ignore specific warnings
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4100 /wd4458")
endif()

# Add external dependencies

# SDL 2
find_package(SDL2 REQUIRED)

# GLEW
add_definitions(-DGLEW_STATIC)
add_subdirectory(externals/glew)

# GLM
set(GLM_INCLUDE_DIRS "externals/glm")

# JSON for Modern C++
set(JSON_INCLUDE_DIRS "externals/json")

# rlutil
set(RLUTIL_INCLUDE_DIRS "externals/rlutil")

# fmt
add_subdirectory(externals/fmt)

# imgui
add_subdirectory(externals/imgui)

# Assimp
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "." FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "." FORCE)
set(ASSIMP_BUILD_STATIC_LIB ON CACHE BOOL "." FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "." FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "." FORCE)

add_subdirectory(externals/assimp)
set(ASSIMP_INCLUDE_DIRS externals/assimp/include)

# STB image
add_subdirectory(externals/stb)

if(NS_BUILD_TESTS)
	# Catch
	set(CATCH_INCLUDE_DIRS externals/catch)

	# Rapidcheck
	add_subdirectory(externals/rapidcheck)
	set(RAPIDCHECK_INCLUDE_DIRS
		externals/rapidcheck/include
		externals/rapidcheck/extras/catch/include
	)
endif()

set(HEADERS
	src/Model.h
	src/Mesh.h
	src/Logging.h
	src/ShaderProgram.h
	src/Entity.h
	src/Constants.h
	src/FileTools.h
	src/Noxoscope.h
	src/Light.h
	src/GLObject.h
	src/GeometryMath.h
	src/GLUtil.h
)

set(SOURCES
	src/Model.cpp
	src/Mesh.cpp
	src/ShaderProgram.cpp
	src/Entity.cpp
	src/Constants.cpp
	src/Noxoscope.cpp
	src/FileTools.cpp
	src/GeometryMath.cpp
	src/GLUtil.cpp
)

set(INCLUDES
	${SDL2_INCLUDE_DIR}
	${GLEW_INCLUDE_DIRS}
	${GLM_INCLUDE_DIRS}
	${JSON_INCLUDE_DIRS}
	${ASSIMP_INCLUDE_DIRS}
	${STB_INCLUDE_DIRS}
	${RLUTIL_INCLUDE_DIRS}
	${RLUTIL_INCLUDE_DIRS}
	${FMT_INCLUDE_DIRS}
	${IMGUI_INCLUDE_DIRS}
)

set(LINK_LIBS
	assimp
	${SDL2_LIBRARY}
	${GLEW_LIBRARIES}
	${FMT_LIBRARIES}
	${IMGUI_LIBRARIES}
	${STB_LIBRARIES}
)

add_library(NoxoscopeLib STATIC
	${HEADERS} # Added to improve visibilitiy in some IDEs
	${SOURCES}
)

target_include_directories(NoxoscopeLib PRIVATE
	${INCLUDES}
)

target_link_libraries(NoxoscopeLib
	${LINK_LIBS}
)

add_executable(Noxoscope
	src/Main.cpp
)

target_include_directories(Noxoscope PRIVATE
	${INCLUDES}
)

target_link_libraries(Noxoscope
	${LINK_LIBS}
	NoxoscopeLib
)

# Add symlink to asset directory. Windows requires running
# create_assets_link.bat manually
if(WIN32)
	add_custom_command(TARGET Noxoscope PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E echo
		mklink /j
		\\\"$<TARGET_FILE_DIR:Noxoscope>/assets\\\"
		\\\"${PROJECT_SOURCE_DIR}/assets\\\"
		">"
		\"${PROJECT_SOURCE_DIR}/create_assets_link.bat\")
else()
	add_custom_command(TARGET Noxoscope PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E create_symlink
		\"${PROJECT_SOURCE_DIR}/assets\"
		\"$<TARGET_FILE_DIR:Noxoscope>/assets\")
endif()

# Copy over bundled SDL2 dll on Windows
if(WIN32)
	function(copySDLDLL destProj)
		add_custom_command(TARGET ${destProj} PRE_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${PROJECT_SOURCE_DIR}/lib/${WIN_LIB_ARCH}/SDL2.dll"
			$<TARGET_FILE_DIR:${destProj}>)
	endfunction()

	copySDLDLL(Noxoscope)
endif()

if(NS_BUILD_TESTS)
	add_executable(NoxoscopeTest
		test/TestShared.h
		test/TestMain.cpp
		test/TestShared.cpp
		test/MathTest.cpp
	)
	target_include_directories(NoxoscopeTest PRIVATE
		src
		${INCLUDES}
		${RAPIDCHECK_INCLUDE_DIRS}
		${CATCH_INCLUDE_DIRS}
	)
	target_link_libraries(NoxoscopeTest
		${LINK_LIBS}
		NoxoscopeLib
		rapidcheck
	)

	if(WIN32)
		copySDLDLL(NoxoscopeTest)
	endif()
endif()
