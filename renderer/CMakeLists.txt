project(wen)

file(GLOB_RECURSE SRC "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.hpp")

add_library(${PROJECT_NAME} STATIC ${SRC} ${HEADERS})

target_link_libraries(${PROJECT_NAME} PUBLIC spdlog glfw glm Vulkan::Vulkan imgui)
target_link_libraries(${PROJECT_NAME} PRIVATE shaderc_shared tinyobjloader)
target_include_directories(${PROJECT_NAME} PUBLIC ./include)
target_include_directories(${PROJECT_NAME} PRIVATE ../3rdlibs/stb)
target_precompile_headers(${PROJECT_NAME} PUBLIC "include/pch.hpp")