add_executable(RenderMesh main.cpp)

target_include_directories(RenderMesh PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(RenderMesh PRIVATE libMelonFrontend)
