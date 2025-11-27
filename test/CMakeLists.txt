cmake_minimum_required(VERSION 3.21)
project(test CXX)

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


INCLUDE_DIRECTORIES(../modules)
#INCLUDE_DIRECTORIES(modules/libdivsufsort/include)
#INCLUDE_DIRECTORIES(modules/simple_btree/include)


LINK_DIRECTORIES(~/lib)
include_directories(~/include)

#

if(NOT TARGET libdivsufsort)
include_directories(${CMAKE_BINARY_DIR}/../modules/libdivsufsort/include)
#add_subdirectory(../modules/libdivsufsort)
add_subdirectory(../modules/libdivsufsort ${CMAKE_BINARY_DIR}/libdivsufsort_build)

endif()

#add_subdirectory(modules/DYNAMIC)


add_executable(r_index_test r_index_test_main.cpp)
target_link_libraries(r_index_test divsufsort divsufsort64 sdsl)

add_executable(lighweight_query lighweight_query_main.cpp)
target_link_libraries(lighweight_query divsufsort divsufsort64 sdsl)

add_executable(counter_example counter_example_main.cpp)
target_link_libraries(counter_example divsufsort divsufsort64 sdsl)

add_executable(generate_query generate_query_main.cpp)
target_link_libraries(generate_query divsufsort divsufsort64 sdsl)

add_executable(fm_index_test fm_index_test_main.cpp)
target_link_libraries(fm_index_test divsufsort divsufsort64 sdsl)

add_executable(deletion_demo deletion_demo_main.cpp)
target_link_libraries(deletion_demo divsufsort divsufsort64 sdsl)

add_executable(insertion_demo insertion_demo_main.cpp)
target_link_libraries(insertion_demo divsufsort divsufsort64 sdsl)



#add_executable(lcp.out main/lcp_main.cpp)
#target_link_libraries(lcp.out fn)

#add_executable(tmp.out main/tmp_test_main.cpp)
#target_link_libraries(tmp.out fn)


#if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#    target_compile_definitions(dynamic_r_index_test.out PRIVATE DEBUG_TMP)
#endif()


#add_executable(performance.out main/performance_main.cpp)
#target_link_libraries(performance.out fn)


#add_executable(performance.out main/performance_main.cpp)
#target_link_libraries(performance.out fn)

#add_executable(online_reverse_bwt.out main/online_reverse_bwt_main.cpp)
#target_link_libraries(online_reverse_bwt.out fn)

#add_executable(dynamic_rindex_performance.out main/dynamic_rindex_performance_main.cpp)
#target_link_libraries(dynamic_rindex_performance.out fn)

#add_executable(build_test.out main/build_test_main.cpp)
#target_link_libraries(build_test.out fn)


#add_executable(count.out count_main.cpp)
#target_link_libraries(count.out fn)

#add_executable(lcp.out lcp_main.cpp)
#target_link_libraries(lcp.out fn)

#add_executable(insert.out insert_main.cpp)
#target_link_libraries(insert.out fn)

#add_executable(reorder_test.out reorder_test_main.cpp)
#target_link_libraries(reorder_test.out fn)
