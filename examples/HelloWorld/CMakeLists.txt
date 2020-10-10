add_executable(HelloWorld main.cpp)

target_include_directories(HelloWorld PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(HelloWorld PRIVATE MelonCore)
