cmake_minimum_required(VERSION 3.14)
project(caaa VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${CMAKE_SOURCE_DIR}/include)
file(GLOB SOURCES "src/*.cpp")
set(EXECUTABLE_NAME caaa)
add_executable(${EXECUTABLE_NAME} ${SOURCES})
target_include_directories(${EXECUTABLE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_compile_options(${EXECUTABLE_NAME} PRIVATE -std=c++23)

# Add nlohmann/json as a dependency
find_package(nlohmann_json REQUIRED)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE nlohmann_json::nlohmann_json)

# Add pybind11 as a dependency
find_package(pybind11 REQUIRED)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE pybind11::module)
pybind11_add_module(engine ${SOURCES})
target_link_libraries(engine PRIVATE nlohmann_json::nlohmann_json)

# Find Python
find_package(Python3 REQUIRED COMPONENTS Development)

# Include Python headers
include_directories(${Python3_INCLUDE_DIRS})

# Link Python library
target_link_libraries(${EXECUTABLE_NAME} PRIVATE ${Python3_LIBRARIES})

# Add Boost.Hana as a dependency
find_package(Boost REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  target_link_libraries(${EXECUTABLE_NAME} PRIVATE ${Boost_LIBRARIES})
  target_link_libraries(engine PRIVATE ${Boost_LIBRARIES})
endif()

# Enable Clang-Tidy
find_program(CLANG_TIDY "clang-tidy")
if(CLANG_TIDY)
  set(CLANG_TIDY_EXCLUDES "-header-filter='^(?!.*out).*'")
  set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY};${CLANG_TIDY_EXCLUDES}")
  set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY};${CLANG_TIDY_EXCLUDES}")
endif()