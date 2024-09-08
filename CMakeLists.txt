cmake_minimum_required(VERSION 3.14)
project(caaa VERSION 0.1.0 LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

include_directories(${CMAKE_SOURCE_DIR}/include)
file(GLOB SOURCES "src/*.c")
set(EXECUTABLE_NAME caaa)
add_executable(${EXECUTABLE_NAME} ${SOURCES})
target_include_directories(${EXECUTABLE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Find and link the system-installed cJSON library
find_package(cJSON REQUIRED)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE cjson)

# Enable Clang-Tidy
find_program(CLANG_TIDY "clang-tidy")
if(CLANG_TIDY)
  set(CLANG_TIDY_EXCLUDES "-header-filter='^(?!.*out).*'")
  set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY};${CLANG_TIDY_EXCLUDES}")
  set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY};${CLANG_TIDY_EXCLUDES}")
endif()
