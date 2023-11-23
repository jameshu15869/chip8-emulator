cmake_minimum_required(VERSION 3.10)

project(chip8)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-Wall -Werror -O0 -g")

set(CHIP_SRC chip/chip8.h chip/chip8.cpp)

# SDL
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  # Specify the commit you depend on and update it regularly.
  URL https://github.com/google/googletest/archive/b10fad38c4026a29ea6561ab15fc4818170d1c10.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Build GoogleTests
enable_testing()
add_executable(tests_bin ${CHIP_SRC} test/test.cpp)
target_link_libraries(tests_bin GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(tests_bin)

add_executable(main main.cpp ${CHIP_SRC})
target_link_libraries(main ${SDL2_LIBRARIES})
target_link_libraries(main)
