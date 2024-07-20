cmake_minimum_required(VERSION 3.10)
project(caaa)

# If not found, fetch from GitHub
if(NOT cJSON_FOUND)
  include(FetchContent)
  FetchContent_Declare(
    cJSON
    GIT_REPOSITORY https://github.com/DaveGamble/cJSON.git
    GIT_TAG        v1.7.14 # Specify the version you need
  )
  FetchContent_MakeAvailable(cJSON)
endif()

add_executable(caaa src/main.c)
#target_include_directories(caaa PRIVATE ${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include)
target_link_libraries(caaa PRIVATE cJSON)