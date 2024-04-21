project(scenes)

file(GLOB_RECURSE source CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE headers CONFIGURE_DEPENDS include/*.hpp)

add_executable(scenes ${source} ${headers})
target_include_directories(scenes PRIVATE include)
target_link_libraries(scenes PRIVATE wen)
target_precompile_headers(scenes REUSE_FROM wen)