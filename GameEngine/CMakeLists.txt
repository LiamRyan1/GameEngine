cmake_minimum_required(VERSION 4.2.0)
project(GameEngine)

set(CMAKE_CXX_STANDARD 17)
# Add this line for Windows


find_package(Bullet REQUIRED)
message(STATUS "BULLET_LIBRARIES: ${BULLET_LIBRARIES}")
message(STATUS "BULLET_INCLUDE_DIRS: ${BULLET_INCLUDE_DIRS}")

find_package(glfw3 REQUIRED)
message(STATUS "GLFW3_FOUND: ${glfw3_FOUND}")

find_package(OpenGL REQUIRED)
message(STATUS "OPENGL_FOUND: ${OPENGL_FOUND}")
message(STATUS "OPENGL_LIBRARIES: ${OPENGL_LIBRARIES}")

find_package(glm REQUIRED)
message(STATUS "GLM_FOUND: ${glm_FOUND}") 

find_package(GLEW REQUIRED)
message(STATUS "GLEW_FOUND: ${GLEW_FOUND}")

add_executable(GameEngine)


target_sources(GameEngine PRIVATE
	src/Main.cpp
	src/Engine.cpp
	src/Camera.cpp
	src/Transform.cpp
	src/Renderer.cpp
	src/Input.cpp
	src/Mesh.cpp
	src/GameTime.cpp
	src/CameraController.cpp

	
    # --- Added ImGui Core ---
    external/imgui/core/imgui.cpp
    external/imgui/core/imgui_draw.cpp
    external/imgui/core/imgui_widgets.cpp
    external/imgui/core/imgui_tables.cpp

    # --- Added ImGui Backends ---
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

# Copy shaders to build directory
add_custom_command(TARGET GameEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:GameEngine>/shaders
    COMMENT "Copying shaders to build directory"
)

# --- Added ImGui include dirs ---
target_include_directories(GameEngine PRIVATE external/imgui/core)
target_include_directories(GameEngine PRIVATE external/imgui/backends)

target_include_directories(GameEngine PRIVATE include)
target_include_directories(GameEngine PRIVATE ${BULLET_INCLUDE_DIRS})
target_link_libraries(GameEngine ${BULLET_LIBRARIES}
	glfw
	OpenGL::GL
	glm::glm
	GLEW::GLEW)
