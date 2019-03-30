# cmake -G "Unix Makefiles" --build <build Dir> <Cmakelist dir>

cmake_minimum_required(VERSION 3.10.2)
project(Gameboy)

set(CMAKE_BUILD_TYPE Debug)

SET(GCC_COVERAGE_COMPILE_FLAGS "-fsanitize=address,undefined -fno-omit-frame-pointer")

# Add compiler flags to cmakes flag variable
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS} -Wall -Wno-long-long -pedantic")


# Gets sources and puts them in SOURCES variable
file(GLOB SOURCES "src/*.c")

INCLUDE_DIRECTORIES(Dependencies/Include include)
LINK_DIRECTORIES(Dependencies/Libs)

add_executable(Gameboy ${SOURCES})

if(APPLE) 
    TARGET_LINK_LIBRARIES(Gameboy glfw pthread)
elseif(UNIX)
    TARGET_LINK_LIBRARIES(Gameboy glfw3 GLU GL X11 m dl Xinerama Xrandr Xi Xcursor Xxf86vm pthread)
endif()
