cmake_minimum_required(VERSION 3.10)
project(caaa VERSION 0.1.0 LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB SOURCES "src/*.c")

# Define a variable for the executable name
set(EXECUTABLE_NAME caaa)

add_executable(${EXECUTABLE_NAME} ${SOURCES})
target_include_directories(${EXECUTABLE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

include(FetchContent)
find_package(cJSON 1.7.18 QUIET) # Check if cJSON is already available

if(NOT cJSON_FOUND) # If not found, fetch and build cJSON
  FetchContent_Declare(
    cJSON
    GIT_REPOSITORY https://github.com/DaveGamble/cJSON.git
    GIT_TAG        v1.7.18
  )
  FetchContent_MakeAvailable(cJSON)
endif()

# Link the cJSON library and include its directories
target_link_libraries(${EXECUTABLE_NAME} PRIVATE cjson)
target_include_directories(${EXECUTABLE_NAME} PRIVATE ${cJSON_SOURCE_DIR})
