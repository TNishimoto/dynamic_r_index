cmake_minimum_required(VERSION 3.21)
project(dynamic_r_index CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CPP_STATIC)
set(CMAKE_CXX_FLAGS "-D_INT128 -Wall -Wextra -static")
else(CPP_STATIC)
set(CMAKE_CXX_FLAGS "-D_INT128 -Wall -Wextra")
endif(CPP_STATIC)

set(CMAKE_CXX_FLAGS_DEBUG  "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Og -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE  "-O3 -DNDEBUG -DTIME_DEBUG")

if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  add_compile_definitions(RELEASE_BUILD)
else()
  add_compile_definitions(DEBUG_BUILD)
endif()



# Set PROJECT_IS_TOP_LEVEL to ON if if this is the top level project (not
# if this is included as a subproject of something else). Note that this is
# handled automatically for CMake >= 3.21.
if (CMAKE_VERSION VERSION_LESS 3.21)
    if ("${CMAKE_PROJECT_NAME}" STREQUAL "${PROJECT_NAME}")
        message("SET PROJECT_IS_TOP_LEVEL ON IN ${PROJECT_NAME}")
        set (PROJECT_IS_TOP_LEVEL ON)
    else()
        message("SET PROJECT_IS_TOP_LEVEL OFF IN ${PROJECT_NAME}")
    set (PROJECT_IS_TOP_LEVEL OFF)    
    endif()
endif ()


INCLUDE_DIRECTORIES(modules)
#INCLUDE_DIRECTORIES(modules/libdivsufsort/include)
#INCLUDE_DIRECTORIES(modules/simple_btree/include)


LINK_DIRECTORIES(~/lib)
include_directories(~/include)

#

if(NOT TARGET libdivsufsort)
include_directories(${CMAKE_BINARY_DIR}/modules/libdivsufsort/include)
add_subdirectory(modules/libdivsufsort)
endif()

#add_subdirectory(modules/DYNAMIC)







#FILE(GLOB SOURCES lib/*.cpp)
FILE(GLOB SOURCES modules/libdivsufsort/sa.cpp)

add_library(fn STATIC ${SOURCES})
target_link_libraries(fn divsufsort divsufsort64 sdsl)

add_executable(build_fm_index.out main/build_fm_index_main.cpp)
target_link_libraries(build_fm_index.out fn)

add_executable(build_r_index.out main/build_r_index_main.cpp)
target_link_libraries(build_r_index.out fn)

add_executable(query.out main/query_main.cpp)
target_link_libraries(query.out fn)
