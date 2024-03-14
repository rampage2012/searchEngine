cmake_minimum_required(VERSION 3.11) # Проверка версии CMake.
									# Если версия установленой программы
									# старее указаной, произайдёт аварийный выход.

project("Search Engine" VERSION 1.0)

set(CMAKE_CXX_STANDART 14)

set(JSON_BuildTests OFF CACHE INTERNAL "")
set(MY_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(MY_SRC_DIR ${CMAKE_SOURCE_DIR}/src)

include(FetchContent)
#FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
FetchContent_Declare(
  googletest
  # Specify the commit you depend on and update it regularly.
  URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
set(gtest_disable_pthreads on)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
enable_testing()

add_subdirectory(nlohmann_json)

add_executable(searchEngine main.cpp ${MY_INCLUDE_DIR}/classes.h ${MY_SRC_DIR}/classes.cpp test.cpp)
target_include_directories(searchEngine PUBLIC ${MY_INCLUDE_DIR})
target_link_libraries(searchEngine PRIVATE nlohmann_json::nlohmann_json GTest::gtest_main)

# Now simply link against gtest or gtest_main as needed. Eg
add_executable(example test.cpp)
target_link_libraries(example GTest::gtest_main)
add_test(NAME exampleTest COMMAND example)
#add_test(NAME exampleTest COMMAND searchEngine)
include(GoogleTest)
gtest_discover_tests(searchEngine)