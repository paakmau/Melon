add_executable(SharedComponent main.cpp)

target_include_directories(SharedComponent PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(SharedComponent PRIVATE MelonCore)
