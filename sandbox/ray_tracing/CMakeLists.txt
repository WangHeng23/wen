project(ray_tracing)

file(GLOB_RECURSE source CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE headers CONFIGURE_DEPENDS include/*.hpp)

add_executable(ray_tracing ${source} ${headers})
target_include_directories(ray_tracing PRIVATE include)
target_link_libraries(ray_tracing PRIVATE wen)
target_precompile_headers(ray_tracing REUSE_FROM wen)