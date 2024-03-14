cmake_minimum_required(VERSION 3.0) # Проверка версии CMake.
									# Если версия установленой программы
									# старее указаной, произайдёт аварийный выход.
set(MY_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(MY_SRC_DIR ${CMAKE_SOURCE_DIR}/src)
#set(JSON_DIR ${CMAKE_SOURCE_DIR}/nlohmann_json)
#add_executable(searchEngine ${MY_INCLUDE_DIR}/classes.h classes.cpp)
target_include_directories(searchEngine PUBLIC ${MY_INCLUDE_DIR} ${MY_SRC_DIR})

#target_link_libraries(searchEngine PRIVATE ${JSON_DIR}/nlohmann_json)