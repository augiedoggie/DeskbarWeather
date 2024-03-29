cmake_minimum_required(VERSION 3.19)

project(DeskbarWeather)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)

include(UseHaiku)

option(BUILD_USERGUIDE "Use sphinx to generate the user guide" ON)
if(BUILD_USERGUIDE)
	find_package(Sphinx)
	if (Sphinx_FOUND)
		add_subdirectory(Assets)
	else()
		message(WARNING "Sphinx was not found.  You will not be able to build the user guide.")
	endif()
else()
	message("Generating user guide is disabled.")
endif()

set(PACKAGE_DOCUMENTATION_DIR "" CACHE FILEPATH "Location of documentation when building as a package")
if(PACKAGE_DOCUMENTATION_DIR)
	add_definitions("-DPACKAGE_DOCUMENTATION_DIR=\"${PACKAGE_DOCUMENTATION_DIR}\"")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

option(USE_CLANG "Enable building with clang instead of gcc" OFF)
if(USE_CLANG)
	set(CMAKE_CXX_COMPILER clang++)
	set(CMAKE_C_COMPILER clang)
	add_compile_options(-fPIC)
endif()

option(STRICT_WARNINGS "Compile with extra warnings and errors" ON)
if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 2.96 AND STRICT_WARNINGS)
	add_compile_options(-Wall -Wextra -Wshadow -Werror)
	if(USE_CLANG)
		# clang finds a lot more problems than gcc, disable some of the warnings
		# set these here instead of earlier, they must come after -Wall, -Werror, etc...
		add_compile_options(-Wno-overloaded-virtual -Wno-unused-private-field
							-Wno-return-type-c-linkage -Wno-unused-const-variable
							-Wno-deprecated-register)
	endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_definitions(-DDEBUG)
endif(CMAKE_BUILD_TYPE STREQUAL "Debug")

add_subdirectory(Source)
