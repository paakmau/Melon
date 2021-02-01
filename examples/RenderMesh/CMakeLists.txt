add_executable(RenderMesh main.cpp)

target_link_libraries(RenderMesh PRIVATE MelonFrontend)

file(COPY mesh.obj DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
