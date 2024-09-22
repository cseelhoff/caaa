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

# Enable Clang-Tidy
find_program(CLANG_TIDY "clang-tidy")
if(CLANG_TIDY)
  set(CLANG_TIDY_EXCLUDES "-header-filter='^(?!.*out).*'")
  set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY};${CLANG_TIDY_EXCLUDES}")
  set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY};${CLANG_TIDY_EXCLUDES}")
endif()
# Define the source and destination paths
set(GAME_DATA_FILE ${CMAKE_SOURCE_DIR}/game_data.json)
set(OUTPUT_DIR ${CMAKE_BINARY_DIR})

# Add custom command for copying the file
add_custom_command(
    TARGET ${EXECUTABLE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${GAME_DATA_FILE} ${OUTPUT_DIR}/game_data.json
)
