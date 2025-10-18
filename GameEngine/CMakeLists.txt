cmake_minimum_required(VERSION 4.2.0)
project(GameEngine)

set(CMAKE_CXX_STANDARD 17)

find_package(Bullet REQUIRED)
message(STATUS "BULLET_LIBRARIES: ${BULLET_LIBRARIES}")
message(STATUS "BULLET_INCLUDE_DIRS: ${BULLET_INCLUDE_DIRS}")

add_executable(GameEngine)


target_sources(GameEngine PRIVATE
	src/Main.cpp
	src/Engine.cpp
)
target_include_directories(GameEngine PRIVATE include)
target_include_directories(GameEngine PRIVATE ${BULLET_INCLUDE_DIRS})
target_link_libraries(GameEngine ${BULLET_LIBRARIES})
