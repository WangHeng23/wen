project(example)

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS model.cpp)

add_executable(${PROJECT_NAME} ${SRC})

target_link_libraries(${PROJECT_NAME} PRIVATE wen)
target_precompile_headers(${PROJECT_NAME} REUSE_FROM wen)