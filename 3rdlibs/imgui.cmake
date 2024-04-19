project(imgui)

file(GLOB SRC imgui/*.cpp imgui/backends/imgui_impl_glfw.cpp imgui/backends/imgui_impl_vulkan.cpp) 

add_library(imgui STATIC ${SRC})
target_include_directories(imgui PUBLIC ./imgui)
target_link_libraries(imgui PUBLIC glfw Vulkan::Vulkan)