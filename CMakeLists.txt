cmake_minimum_required(VERSION 3.14)
project(caaa VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${CMAKE_SOURCE_DIR}/include)
file(GLOB SOURCES "src/*.cpp")
set(EXECUTABLE_NAME caaa)
add_executable(${EXECUTABLE_NAME} ${SOURCES})
target_include_directories(${EXECUTABLE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_compile_options(${EXECUTABLE_NAME} PRIVATE -std=c++23)

# Find and link the system-installed cJSON library
find_package(cJSON REQUIRED)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE cjson)

find_package(pybind11 REQUIRED)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE pybind11::module)
pybind11_add_module(engine src/engine.cpp)

# Find Python
find_package(Python3 REQUIRED COMPONENTS Development)

# Include Python headers
include_directories(${Python3_INCLUDE_DIRS})

# Link Python library
target_link_libraries(${EXECUTABLE_NAME} PRIVATE ${Python3_LIBRARIES})

# Enable Clang-Tidy
find_program(CLANG_TIDY "clang-tidy")
if(CLANG_TIDY)
  set(CLANG_TIDY_EXCLUDES "-header-filter='^(?!.*out).*'")
  set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY};${CLANG_TIDY_EXCLUDES}")
  set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY};${CLANG_TIDY_EXCLUDES}")
endif()