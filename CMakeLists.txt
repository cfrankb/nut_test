cmake_minimum_required(VERSION 3.10)
project(nut-test)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")
set(CMAKE_CXX_STANDARD 20)

# Add your source files
file(GLOB_RECURSE SOURCES src/*.cpp src/shared/*.cpp)

add_compile_options(-fexceptions)


# Include Squirrel headers
#include_directories(${CMAKE_SOURCE_DIR}/external/squirrel/include)
include_directories(
    ${CMAKE_SOURCE_DIR}/external/quirrel/squirrel
    ${CMAKE_SOURCE_DIR}/external/quirrel/include
    ${CMAKE_SOURCE_DIR}/external/quirrel/sqrat/include
    #${CMAKE_SOURCE_DIR}/external/quirrel-4.6.0/include
    #${CMAKE_SOURCE_DIR}/external/quirrel-4.6.0
)


# Link Squirrel libraries

#add_subdirectory(external/squirrel squirrel)
#add_subdirectory(external/quirrel-4.6.0)
add_subdirectory(external/quirrel)
target_compile_options(squirrel PRIVATE -fexceptions)
target_compile_options(sqstdlib PRIVATE -fexceptions)
target_compile_options(sqmodules PRIVATE -fexceptions)
target_compile_options(quirrel-compiler PRIVATE -fexceptions)
#link_directories(${CMAKE_SOURCE_DIR}/external/squirrel/lib)


target_compile_options(squirrel PRIVATE -fexceptions)
target_compile_options(quirrel-compiler PRIVATE -fexceptions)

add_executable(nut-test ${SOURCES})

# Link against Squirrel and SqStdLib
#target_link_libraries(nut-test squirrel::squirrel squirrel::sqstdlib)
target_link_libraries(nut-test squirrel  sqmodules sqstdlib  quirrel-compiler )
