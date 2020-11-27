add_executable(EntityCommandBuffer main.cpp)

target_include_directories(EntityCommandBuffer PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(EntityCommandBuffer PRIVATE MelonCore)
