cmake_minimum_required(VERSION 4.2.0)
project(GameEngine)

set(CMAKE_CXX_STANDARD 17)

add_executable(GameEngine)


target_sources(GameEngine PRIVATE
	GameEngine.cpp
)
