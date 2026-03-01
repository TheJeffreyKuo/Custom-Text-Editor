cmake_minimum_required(VERSION 3.16)
project(kilo-cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(kilo_lib STATIC
    src/lib_placeholder.cpp
)

target_include_directories(kilo_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_compile_options(kilo_lib PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror=return-type
)

add_executable(kilo src/main.cpp)
target_link_libraries(kilo PRIVATE kilo_lib)

target_compile_options(kilo PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror=return-type
)

add_executable(kilo_tests tests/test_main.cpp)
target_link_libraries(kilo_tests PRIVATE kilo_lib)

target_compile_options(kilo_tests PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

enable_testing()
add_test(NAME unit_tests COMMAND kilo_tests)
