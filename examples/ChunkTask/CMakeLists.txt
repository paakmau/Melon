add_executable(ChunkTask main.cpp)

target_include_directories(ChunkTask PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(ChunkTask PRIVATE MelonCore)
